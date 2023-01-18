#include <vector>
#include <queue>
#include "utils/Semaphore.h"
#include "AudioBuffer.h"
#include "AudioIO.h"
#include "OpusPlayer.h"

template<typename T>
class ConcurrentQueue
{
public:
	ConcurrentQueue()
	{
	}

	~ConcurrentQueue()
	{
	}

	size_t Size()
	{
		return m_queue.size();
	}

	T Front()
	{
		return m_queue.front();
	}

	void Push(T packet)
	{
		m_mutex.lock();
		m_queue.push(packet);
		m_mutex.unlock();
		m_semaphore_out.notify();
	}

	T Pop()
	{
		m_semaphore_out.wait();
		m_mutex.lock();
		T ret = m_queue.front();
		m_queue.pop();
		m_mutex.unlock();
		return ret;
	}

private:
	std::queue<T> m_queue;
	std::mutex m_mutex;
	Semaphore m_semaphore_out;
};


extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

static const AVCodecID audio_codec_id = AV_CODEC_ID_OPUS;
static const int sample_rate = 48000;

class OpusPlayer::Internal
{
public:
	Internal(int id_audio_device)
	{
		static bool s_first_time = true;
		if (s_first_time)
		{
			av_log_set_level(AV_LOG_QUIET);
			s_first_time = false;
		}

		AVCodec* audio_codec = avcodec_find_decoder(audio_codec_id);
		m_p_codec_ctx_audio = avcodec_alloc_context3(audio_codec);
		m_p_codec_ctx_audio->codec_id = audio_codec_id;
		m_p_codec_ctx_audio->sample_fmt = AV_SAMPLE_FMT_FLTP;
		m_p_codec_ctx_audio->bit_rate = 32768;
		m_p_codec_ctx_audio->sample_rate = sample_rate;
		m_p_codec_ctx_audio->channel_layout = AV_CH_LAYOUT_MONO;
		m_p_codec_ctx_audio->channels = av_get_channel_layout_nb_channels(m_p_codec_ctx_audio->channel_layout);

		avcodec_open2(m_p_codec_ctx_audio, audio_codec, nullptr);
		m_p_frm_raw_audio = av_frame_alloc();
		m_p_frm_s16_audio = av_frame_alloc();

		m_swr_ctx = swr_alloc_set_opts(nullptr, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, m_p_codec_ctx_audio->sample_rate,
			m_p_codec_ctx_audio->channel_layout, m_p_codec_ctx_audio->sample_fmt, m_p_codec_ctx_audio->sample_rate, 0, nullptr);
		swr_init(m_swr_ctx);

		m_audio_out = (std::unique_ptr<AudioOut>)(new AudioOut(id_audio_device, m_p_codec_ctx_audio->sample_rate, callback, eof_callback, this));
	}

	~Internal()
	{
		m_audio_playing = false;
		m_audio_out = nullptr;

		if (m_packet_ref)
		{
			av_packet_unref(&m_packet);
			m_packet_ref = false;
		}

		swr_free(&m_swr_ctx);
		av_frame_free(&m_p_frm_s16_audio);
		av_frame_free(&m_p_frm_raw_audio);
		avcodec_free_context(&m_p_codec_ctx_audio);
	}

	void AddPacket(size_t size, const uint8_t* data)
	{
		AVPacket pkt;
		av_new_packet(&pkt, (int)size);
		memcpy(pkt.data, data, size);
		m_packet_queue.Push(pkt);
	}

private:
	AVCodecContext* m_p_codec_ctx_audio;
	AVFrame* m_p_frm_raw_audio;
	AVFrame* m_p_frm_s16_audio;
	std::unique_ptr<AudioBuffer> m_audio_buffer;
	SwrContext* m_swr_ctx;

	ConcurrentQueue<AVPacket> m_packet_queue;

	bool m_audio_playing = true;
	std::unique_ptr<AudioOut> m_audio_out;

	bool m_packet_ref = false;
	AVPacket m_packet;
	int m_in_pos = 0;
	int m_in_length = 0;
	int m_in_buf_size = 0;

	static void eof_callback(void* usr_ptr)
	{

	}

	static bool callback(short* buf, size_t buf_size, void* usr_ptr)
	{
		Internal* self = (Internal*)usr_ptr;
		ConcurrentQueue<AVPacket>& queue = self->m_packet_queue;

		if (!self->m_audio_playing) 
		{
			memset(buf, 0, sizeof(short) * buf_size * 2);
			return false;
		}
		
		int out_pos = 0;
		while (self->m_audio_playing && out_pos < buf_size)
		{
			if (self->m_audio_buffer != nullptr && self->m_in_length - self->m_in_pos >= 1)
			{
				int copy_size = self->m_in_length - self->m_in_pos;
				int copy_size_out = buf_size - out_pos;
				if (copy_size > copy_size_out) copy_size = copy_size_out;
				short* p_out = buf + out_pos * 2;
				const short* p_in = (const short*)self->m_audio_buffer->data() + self->m_in_pos * 2;
				memcpy(p_out, p_in, sizeof(short) * copy_size * 2);
				out_pos += copy_size;
				self->m_in_pos += copy_size;
			}
			else
			{
				while (true)
				{
					int ret = avcodec_receive_frame(self->m_p_codec_ctx_audio, self->m_p_frm_raw_audio);
					if (ret == 0)
					{
						self->m_in_length = self->m_p_frm_raw_audio->nb_samples;
						if (self->m_audio_buffer == nullptr || self->m_in_length > self->m_in_buf_size)
						{
							self->m_in_buf_size = self->m_in_length;
							self->m_audio_buffer = (std::unique_ptr<AudioBuffer>)(new AudioBuffer(2, self->m_in_buf_size));
							av_samples_fill_arrays(self->m_p_frm_s16_audio->data, self->m_p_frm_s16_audio->linesize,
								self->m_audio_buffer->data(), 2, self->m_in_buf_size, AV_SAMPLE_FMT_S16, 0);
						}
						swr_convert(self->m_swr_ctx, self->m_p_frm_s16_audio->data, self->m_in_length,
							(const uint8_t**)self->m_p_frm_raw_audio->data, self->m_in_length);

						self->m_in_pos = 0;
						break;
					}

					if (self->m_packet_ref)
					{
						av_packet_unref(&self->m_packet);
						self->m_packet_ref = false;
					}

					while (self->m_audio_playing && queue.Size() == 0)
					{
						std::this_thread::sleep_for(std::chrono::microseconds(10));
					}
					if (!self->m_audio_playing) break;

					self->m_packet = queue.Pop();
					self->m_packet_ref = true;

					avcodec_send_packet(self->m_p_codec_ctx_audio, &self->m_packet);
				}
			}
		}

		if (out_pos < buf_size)
		{
			short* p_out = buf + out_pos * 2;
			size_t bytes = sizeof(short) * (buf_size - out_pos) * 2;
			memset(p_out, 0, bytes);
		}
		return true;
	}

};


OpusPlayer::OpusPlayer(int id_audio_device)
{
	if (id_audio_device < 0)
	{
		g_get_list_audio_devices(false, nullptr, &id_audio_device);
	}
	m_internal = std::unique_ptr<Internal>(new Internal(id_audio_device));
}

OpusPlayer::~OpusPlayer()
{

}

void OpusPlayer::AddPacket(size_t size, const uint8_t* data)
{
	m_internal->AddPacket(size, data);

}