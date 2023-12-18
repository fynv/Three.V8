#include <thread>
#include <vector>
#include "AudioBuffer.h"
#include "AudioIO.h"
#include "OpusRecorder.h"
#include "utils/AsyncCallbacks.h"

class AudioRecorder
{
public:
	AudioRecorder(int devId)
	{
		m_audio_in = (std::unique_ptr<AudioIn>)(new AudioIn(devId, 48000, callback, eof_callback, this));
	}

	~AudioRecorder()
	{
		m_audio_in = nullptr;

		while (m_buffer_queue.Size() > 0)
		{
			AudioBuffer* buf = m_buffer_queue.Pop();
			delete buf;
		}
	}

	AudioBuffer* get_buffer()
	{
		return m_buffer_queue.Pop();
	}

private:
	ConcurrentQueue<AudioBuffer*> m_buffer_queue;
	std::unique_ptr<AudioIn> m_audio_in;

	static void eof_callback(void* usr_ptr) {}

	static bool callback(const short* buf, size_t buf_size, void* usr_ptr)
	{
		AudioRecorder* self = (AudioRecorder*)usr_ptr;
		self->recordBuf(buf, buf_size);
		return true;
	}

	void recordBuf(const short* buf, size_t buf_size)
	{
		AudioBuffer* newBuf = new AudioBuffer(1, buf_size);
		memcpy(newBuf->data(), buf, buf_size * sizeof(short));
		m_buffer_queue.Push(newBuf);
	}
};

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

static const AVCodecID audio_codec_id = AV_CODEC_ID_OPUS;
static const int sample_rate = 48000;

class OpusRecorder::Internal
{
public:
	Internal(int id_audio_device)
	{
		AVCodec* audio_codec = avcodec_find_encoder(audio_codec_id);
		m_audio_enc = avcodec_alloc_context3(audio_codec);
		m_audio_enc->codec_id = audio_codec_id;
		m_audio_enc->sample_fmt = AV_SAMPLE_FMT_FLTP;
		m_audio_enc->bit_rate = 32768;
		m_audio_enc->sample_rate = sample_rate;
		m_audio_enc->channel_layout = AV_CH_LAYOUT_MONO;
		m_audio_enc->channels = av_get_channel_layout_nb_channels(m_audio_enc->channel_layout);

		avcodec_open2(m_audio_enc, audio_codec, nullptr);

		int nb_samples = m_audio_enc->frame_size;

		m_frame = av_frame_alloc();
		m_frame->format = m_audio_enc->sample_fmt;
		m_frame->channel_layout = m_audio_enc->channel_layout;
		m_frame->sample_rate = m_audio_enc->sample_rate;
		m_frame->nb_samples = nb_samples;
		av_frame_get_buffer(m_frame, 0);

		m_tmp_frame = av_frame_alloc();
		m_tmp_frame->format = AV_SAMPLE_FMT_S16;
		m_tmp_frame->channel_layout = m_audio_enc->channel_layout;
		m_tmp_frame->sample_rate = m_audio_enc->sample_rate;
		m_tmp_frame->nb_samples = nb_samples;
		av_frame_get_buffer(m_tmp_frame, 0);

		m_swr_ctx = swr_alloc_set_opts(nullptr, 
			m_frame->channel_layout, m_audio_enc->sample_fmt, m_frame->sample_rate,
			m_tmp_frame->channel_layout, AV_SAMPLE_FMT_S16, m_tmp_frame->sample_rate, 0, nullptr);
		swr_init(m_swr_ctx);


		m_audio_recorder = (std::unique_ptr<AudioRecorder>)(new AudioRecorder(id_audio_device));
		m_thread_record = (std::unique_ptr<std::thread>)(new std::thread(thread_record, this));

		AsyncCallbacks::AddSubQueue(&m_async_queue);
	}

	~Internal()
	{
		AsyncCallbacks::RemoveSubQueue(&m_async_queue);

		m_recording = false;
		m_thread_record->join();
		m_thread_record = nullptr;

		delete m_buf_in;

		avcodec_free_context(&m_audio_enc);
		av_frame_free(&m_frame);
		av_frame_free(&m_tmp_frame);
		swr_free(&m_swr_ctx);

	}

	PacketCallback callback = nullptr;
	void* callback_data = nullptr;

private:
	AVCodecContext* m_audio_enc;
	AVFrame* m_frame = nullptr;
	AVFrame* m_tmp_frame = nullptr;
	struct SwrContext* m_swr_ctx = nullptr;

	std::unique_ptr<AudioRecorder> m_audio_recorder;
	AudioBuffer* m_buf_in = nullptr;
	int m_in_pos = 0;
	int64_t m_next_pts = 0;
	int64_t m_sample_count = 0;

	bool m_recording = true;
	std::unique_ptr<std::thread> m_thread_record;

	class PacketCallable : public Callable
	{
	public:
		Internal* self;
		std::vector<uint8_t> packet;

		PacketCallable(Internal* self, const std::vector<uint8_t>& packet)
			: self(self), packet(packet)
		{

		}

		void call() override
		{
			if (self->callback != nullptr)
			{
				self->callback(packet.data(), packet.size(), self->callback_data);
			}
		}
	};

	AsyncQueue m_async_queue;

	void update_audio()
	{
		AVCodecContext* c = m_audio_enc;
		int16_t* buf = (int16_t*)m_tmp_frame->data[0];
		int out_len = m_tmp_frame->nb_samples;
		int out_pos = 0;

		while (out_pos < out_len)
		{
			if (m_buf_in != nullptr && m_in_pos < m_buf_in->len())
			{
				int copy_size = m_buf_in->len() - m_in_pos;
				int copy_size_out = out_len - out_pos;
				if (copy_size > copy_size_out) copy_size = copy_size_out;
				short* p_out = buf + out_pos;
				const short* p_in = (const short*)m_buf_in->data() + m_in_pos;
				memcpy(p_out, p_in, sizeof(short) * copy_size);
				out_pos += copy_size;
				m_in_pos += copy_size;
			}
			else
			{
				delete m_buf_in;
				m_buf_in = m_audio_recorder->get_buffer();
				m_in_pos = 0;
			}
		}
		m_tmp_frame->pts = m_next_pts;
		m_next_pts += m_tmp_frame->nb_samples;

		int dst_nb_samples = (int)av_rescale_rnd(swr_get_delay(m_swr_ctx, c->sample_rate) + m_tmp_frame->nb_samples, c->sample_rate, c->sample_rate, AV_ROUND_UP);
		
		av_frame_make_writable(m_frame);
		swr_convert(m_swr_ctx, m_frame->data, dst_nb_samples, (const uint8_t**)m_tmp_frame->data, m_tmp_frame->nb_samples);

		m_frame->pts = av_rescale_q(m_sample_count, { 1, c->sample_rate }, c->time_base);
		m_sample_count += dst_nb_samples;

		int ret = avcodec_send_frame(c, m_frame);
		while (ret >= 0)
		{
			AVPacket pkt = { 0 };
			ret = avcodec_receive_packet(c, &pkt);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				break;

			std::vector<uint8_t> packet(pkt.size);
			memcpy(packet.data(), pkt.data, pkt.size);

			m_async_queue.Add(new PacketCallable(this, packet));

			av_packet_unref(&pkt);
		}
	}

	static void thread_record(Internal* self)
	{
		while (self->m_recording)
		{
			self->update_audio();
		}
	}

};

OpusRecorder::OpusRecorder(int id_audio_device)
{
	static bool s_first_time = true;
	if (s_first_time)
	{
		av_log_set_level(AV_LOG_QUIET);
		s_first_time = false;
	}

	if (id_audio_device < 0)
	{
		g_get_list_audio_devices(false, &id_audio_device, nullptr);
	}
	m_internal = std::unique_ptr<Internal>(new Internal(id_audio_device));
}

OpusRecorder::~OpusRecorder()
{

}

void OpusRecorder::SetCallback(PacketCallback callback, void* callback_data)
{
	m_internal->callback = callback;
	m_internal->callback_data = callback_data;
}

void* OpusRecorder::GetCallbackData()
{
	return m_internal->callback_data;
}
