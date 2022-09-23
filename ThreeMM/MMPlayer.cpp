#include "MMIOContext.h"
#include "MMPlayer.h"
#include "AudioBuffer.h"
#include "AudioIO.h"
#include "utils/Image.h"
#include "utils/Utils.h"
#include "renderers/GLUtils.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/log.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "swresample.lib")

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

class Semaphore {
public:
	Semaphore(int count_ = 0)
		: count(count_) {}

	inline void notify()
	{
		std::unique_lock<std::mutex> lock(mtx);
		count++;
		cv.notify_one();
	}

	inline void wait()
	{
		std::unique_lock<std::mutex> lock(mtx);

		while (count == 0) {
			cv.wait(lock);
		}
		count--;
	}

private:
	std::mutex mtx;
	std::condition_variable cv;
	int count;
};


class MMPlayer
{
public:
	class PacketQueue;
	class AudioPlayback;
	class VideoPlayback;

	MMPlayer(const char* fn, bool play_audio = true, bool play_video = true, int id_audio_device = 0, int speed = 1);
	~MMPlayer();

	uint64_t get_duration() const { return m_duration; }
	bool is_playing() const;
	bool is_eof_reached() const;
	uint64_t get_position() const;

	int video_width() const { return m_video_width; }
	int video_height() const { return m_video_height; }

	void stop();
	void start();
	void set_position(uint64_t pos);
	void set_audio_device(int id_audio_device);

	const Image* read_video_frame();

private:
	void _start(uint64_t pos);

	std::unique_ptr<MMIOContext> m_io_ctx;

	std::unique_ptr<PacketQueue> m_queue_audio;
	std::unique_ptr<PacketQueue> m_queue_video;

	AVFormatContext* m_p_fmt_ctx = nullptr;

	int m_a_idx = -1;
	int m_v_idx = -1;

	int m_audio_time_base_num, m_audio_time_base_den;
	int m_video_time_base_num, m_video_time_base_den;
	int m_video_width, m_video_height;
	uint64_t m_duration;

	AVCodecContext* m_p_codec_ctx_audio;
	AVFrame* m_p_frm_raw_audio;
	AVFrame* m_p_frm_s16_audio;
	std::unique_ptr<AudioBuffer> m_audio_buffer;
	SwrContext* m_swr_ctx;

	AVCodecContext* m_p_codec_ctx_video;
	AVFrame* m_p_frm_raw_video;
	AVFrame* m_p_frm_rgb_video;
	std::unique_ptr<Image> m_video_buffer;
	SwsContext* m_sws_ctx;

	std::unique_ptr<Image> m_video_bufs[3];
	int m_last_video_buf = 2;
	bool m_updated = true;

	std::unique_ptr<AVPacket> m_p_packet;
	bool m_demuxing = false;
	bool m_audio_playing = false;
	bool m_video_playing = false;
	bool m_audio_eof = true;
	bool m_video_eof = true;

	uint64_t m_sync_local_time;
	uint64_t m_sync_progress;
	mutable std::mutex m_mutex_sync;
	void _set_sync_point(uint64_t local_time, uint64_t progress);
	void _get_sync_point(uint64_t& local_time, uint64_t& progress) const;

	int m_id_audio_device;

	static void thread_demux(MMPlayer* self);
	std::unique_ptr<std::thread> m_thread_demux;

	std::unique_ptr<AudioPlayback> m_audio_playback;
	std::unique_ptr<VideoPlayback> m_video_playback;

	int m_speed;
};


class MMPlayer::PacketQueue
{
public:
	PacketQueue(int cache_size) : m_semaphore_in(cache_size)
	{
	}

	~PacketQueue()
	{
	}

	size_t Size()
	{
		return m_queue.size();
	}

	AVPacket Front()
	{
		return m_queue.front();
	}

	void Push(AVPacket packet)
	{
		m_semaphore_in.wait();
		m_mutex.lock();
		m_queue.push(packet);
		m_mutex.unlock();
		m_semaphore_out.notify();
	}

	AVPacket Pop()
	{
		m_semaphore_out.wait();
		m_mutex.lock();
		AVPacket ret = m_queue.front();
		m_queue.pop();
		m_mutex.unlock();
		m_semaphore_in.notify();
		return ret;
	}

private:
	std::queue<AVPacket> m_queue;
	std::mutex m_mutex;
	Semaphore m_semaphore_in;
	Semaphore m_semaphore_out;
};


class MMPlayer::AudioPlayback
{
public:
	AudioPlayback(int audioDevId, MMPlayer* player) : m_player(player)
	{
		m_p_packet = std::unique_ptr<AVPacket>(new AVPacket);
		m_packet_ref = false;
		m_audio_out = (std::unique_ptr<AudioOut>)(new AudioOut(audioDevId, player->m_p_codec_ctx_audio->sample_rate, callback, eof_callback, this));
	}


	~AudioPlayback()
	{
		m_audio_out = nullptr;

		if (m_packet_ref)
		{
			av_packet_unref(m_p_packet.get());
			m_packet_ref = false;
		}
	}

private:
	MMPlayer* m_player;
	std::unique_ptr<AudioOut> m_audio_out;

	bool m_eof = false;
	bool m_packet_ref = false;
	std::unique_ptr<AVPacket> m_p_packet;
	int m_in_pos = 0;
	int m_in_length = 0;
	int m_in_buf_size = 0;

	int m_sync_count = 0;

	static void eof_callback(void* usr_ptr)
	{
		AudioPlayback* self = (AudioPlayback*)usr_ptr;
		MMPlayer* player = self->m_player;
		player->m_audio_eof = true;
	}

	static bool callback(short* buf, size_t buf_size, void* usr_ptr)
	{
		AudioPlayback* self = (AudioPlayback*)usr_ptr;
		MMPlayer* player = self->m_player;
		PacketQueue& queue = *player->m_queue_audio;
		int time_base_num = player->m_audio_time_base_num;
		int time_base_den = player->m_audio_time_base_den;
		int speed = player->m_speed;

		bool eof = self->m_eof;
		if (!player->m_audio_playing || eof)
		{
			memset(buf, 0, sizeof(short) * buf_size * 2);
			return false;
		}

		int out_pos = 0;
		int count_packet = 0;
		int64_t progress = -1;
		while (player->m_audio_playing && !eof && out_pos < buf_size)
		{
			if (player->m_audio_buffer != nullptr && self->m_in_length - self->m_in_pos >= speed)			
			{
				int copy_size = (self->m_in_length - self->m_in_pos) / speed;
				int copy_size_out = buf_size - out_pos;
				if (copy_size > copy_size_out) copy_size = copy_size_out;
				short* p_out = buf + out_pos * 2;
				const short* p_in = (const short*)player->m_audio_buffer->data() + self->m_in_pos * 2;
				if (speed > 1)
				{
					for (int i = 0; i < copy_size; i++)
					{
						float f0 = 0.0f;
						float f1 = 0.0f;
						for (int j = 0; j < speed; j++)
						{
							f0 += (float)(*p_in);
							p_in++;
							f1 += (float)(*p_in);
							p_in++;
						}
						short v0 = (short)(f0 / (float)speed + 0.5f);
						short v1 = (short)(f1 / (float)speed + 0.5f);
						*p_out = v0;
						p_out++;
						*p_out = v1;
						p_out++;
					}
				}
				else
				{
					memcpy(p_out, p_in, sizeof(short) * copy_size * 2);
				}
				out_pos += copy_size;
				self->m_in_pos += copy_size * speed;
			}
			else
			{
				while (true)
				{
					int ret = avcodec_receive_frame(player->m_p_codec_ctx_audio, player->m_p_frm_raw_audio);
					if (ret == 0)
					{
						self->m_in_length = player->m_p_frm_raw_audio->nb_samples;
						if (player->m_audio_buffer == nullptr || self->m_in_length > self->m_in_buf_size)
						{
							self->m_in_buf_size = self->m_in_length;
							player->m_audio_buffer = (std::unique_ptr<AudioBuffer>)(new AudioBuffer(2, self->m_in_buf_size));
							av_samples_fill_arrays(player->m_p_frm_s16_audio->data, player->m_p_frm_s16_audio->linesize,
								player->m_audio_buffer->data(), 2, self->m_in_buf_size, AV_SAMPLE_FMT_S16, 0);
						}
						swr_convert(player->m_swr_ctx, player->m_p_frm_s16_audio->data, self->m_in_length,
							(const uint8_t**)player->m_p_frm_raw_audio->data, self->m_in_length);

						self->m_in_pos = 0;
						break;
					}

					if (self->m_packet_ref)
					{
						av_packet_unref(self->m_p_packet.get());
						self->m_packet_ref = false;
					}

					while (player->m_audio_playing && !eof && queue.Size() == 0)
					{
						if (!player->m_demuxing) eof = true;
					}
					if (!player->m_audio_playing || eof) break;

					*self->m_p_packet = queue.Pop();
					self->m_packet_ref = true;

					if (count_packet == 0)
					{
						while (true)
						{
							progress = self->m_p_packet->dts * time_base_num * AV_TIME_BASE / time_base_den;
							if (progress >= player->m_sync_progress) break;

							avcodec_send_packet(player->m_p_codec_ctx_audio, self->m_p_packet.get());
							while (avcodec_receive_frame(player->m_p_codec_ctx_audio, player->m_p_frm_raw_audio) == 0);
							av_packet_unref(self->m_p_packet.get());
							self->m_packet_ref = false;

							while (player->m_audio_playing && !eof && queue.Size() == 0)
							{
								if (!player->m_demuxing) eof = true;
							}
							if (!player->m_audio_playing || eof) break;
							*self->m_p_packet = queue.Pop();
							self->m_packet_ref = true;
						}
						if (!player->m_audio_playing || eof) break;
					}

					avcodec_send_packet(player->m_p_codec_ctx_audio, self->m_p_packet.get());
					count_packet++;
				}

				if (!player->m_audio_playing || eof) break;
			}
		}

		if (out_pos < buf_size)
		{
			short* p_out = buf + out_pos * 2;
			size_t bytes = sizeof(short) * (buf_size - out_pos) * 2;
			memset(p_out, 0, bytes);
		}

		static int s_sync_interval = 10;
		if (progress > 0)
		{
			if (self->m_sync_count == 0)
			{
				uint64_t localtime = time_micro_sec();
				player->_set_sync_point(localtime, progress);
			}
			self->m_sync_count = (self->m_sync_count + 1) % s_sync_interval;
		}

		self->m_eof = eof;
		return true;

	}

};


class MMPlayer::VideoPlayback
{
public:
	VideoPlayback(MMPlayer* player) : m_player(player)
	{
		m_thread_read = (std::unique_ptr<std::thread>)(new std::thread(thread_read, this));
	}


	~VideoPlayback()
	{
		m_thread_read->join();
	}

private:
	MMPlayer* m_player;
	std::unique_ptr<std::thread> m_thread_read;

	static void thread_read(VideoPlayback* self)
	{
		MMPlayer* player = (MMPlayer*)self->m_player;
		PacketQueue& queue = *player->m_queue_video;

		int time_base_num = player->m_video_time_base_num;
		int time_base_den = player->m_video_time_base_den;

		while (player->m_video_playing && !player->m_video_eof)
		{
			uint64_t localtime, progress;
			player->_get_sync_point(localtime, progress);
			uint64_t t = time_micro_sec();
			int64_t cur_progress = progress + (t - localtime) * player->m_speed;

			while (player->m_video_playing && !player->m_video_eof && queue.Size() == 0)
			{
				if (!player->m_demuxing)
					player->m_video_eof = true;
			}
			if (!player->m_video_playing || player->m_video_eof) break;

			AVPacket packet = queue.Front();
			int64_t t_next_frame = packet.dts * time_base_num * AV_TIME_BASE / time_base_den;
			if (t_next_frame > cur_progress)
			{
				std::this_thread::sleep_for(std::chrono::microseconds(t_next_frame - cur_progress));
			}
			else
			{
				while (true)
				{
					packet = queue.Pop();
					avcodec_send_packet(player->m_p_codec_ctx_video, &packet);
					avcodec_receive_frame(player->m_p_codec_ctx_video, player->m_p_frm_raw_video);
					av_packet_unref(&packet);

					if (queue.Size() == 0) break;
					AVPacket next_packet = queue.Front();
					t_next_frame = next_packet.dts * time_base_num * AV_TIME_BASE / time_base_den;
					if (t_next_frame > cur_progress) break;

				}

				sws_scale(player->m_sws_ctx, (const uint8_t* const*)player->m_p_frm_raw_video->data, player->m_p_frm_raw_video->linesize,
					0, player->m_p_codec_ctx_video->height, player->m_p_frm_rgb_video->data, player->m_p_frm_rgb_video->linesize);

				int this_buf = (player->m_last_video_buf + 1) % 3;
				*player->m_video_bufs[this_buf] = *player->m_video_buffer;
				player->m_last_video_buf = this_buf;
				player->m_updated = true;
			}
		}
		player->m_video_eof = true;
	}
};


MMPlayer::MMPlayer(const char* fn, bool play_audio, bool play_video, int id_audio_device, int speed) 
	: m_id_audio_device(id_audio_device), m_speed(speed)
{
	static bool s_first_time = true;
	if (s_first_time)
	{
		av_log_set_level(AV_LOG_QUIET);
		s_first_time = false;
	}

	if (!exists_test(fn))
		printf("Failed loading %s\n", fn);

	m_io_ctx = std::unique_ptr<MMIOContext>(new MMFILEContext(fn));
	m_p_fmt_ctx = ::avformat_alloc_context();
	m_p_fmt_ctx->pb = m_io_ctx->get_avio();

	avformat_open_input(&m_p_fmt_ctx, fn, nullptr, nullptr);
	avformat_find_stream_info(m_p_fmt_ctx, nullptr);

	m_duration = m_p_fmt_ctx->duration;

	for (unsigned i = 0; i < m_p_fmt_ctx->nb_streams; i++)
	{
		if (play_audio && m_p_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			m_a_idx = i;
			m_audio_time_base_num = m_p_fmt_ctx->streams[i]->time_base.num;
			m_audio_time_base_den = m_p_fmt_ctx->streams[i]->time_base.den;
		}
		if (play_video && m_p_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			m_v_idx = i;
			m_video_time_base_num = m_p_fmt_ctx->streams[i]->time_base.num;
			m_video_time_base_den = m_p_fmt_ctx->streams[i]->time_base.den;
		}
	}

	// audio
	if (m_a_idx >= 0)
	{
		AVCodecParameters* p_codec_par = m_p_fmt_ctx->streams[m_a_idx]->codecpar;
		AVCodec* p_codec = avcodec_find_decoder(p_codec_par->codec_id);
		m_p_codec_ctx_audio = avcodec_alloc_context3(p_codec);
		avcodec_parameters_to_context(m_p_codec_ctx_audio, p_codec_par);
		avcodec_open2(m_p_codec_ctx_audio, p_codec, nullptr);

		m_p_frm_raw_audio = av_frame_alloc();
		m_p_frm_s16_audio = av_frame_alloc();

		int64_t layout_in = av_get_default_channel_layout(m_p_codec_ctx_audio->channels);
		m_swr_ctx = swr_alloc_set_opts(nullptr, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, m_p_codec_ctx_audio->sample_rate,
			layout_in, m_p_codec_ctx_audio->sample_fmt, m_p_codec_ctx_audio->sample_rate, 0, nullptr);
		swr_init(m_swr_ctx);
	}

	// video
	if (m_v_idx >= 0)
	{
		AVCodecParameters* p_codec_par = m_p_fmt_ctx->streams[m_v_idx]->codecpar;
		AVCodec* p_codec = avcodec_find_decoder(p_codec_par->codec_id);
		m_p_codec_ctx_video = avcodec_alloc_context3(p_codec);
		avcodec_parameters_to_context(m_p_codec_ctx_video, p_codec_par);
		avcodec_open2(m_p_codec_ctx_video, p_codec, nullptr);

		m_p_frm_raw_video = av_frame_alloc();
		m_p_frm_rgb_video = av_frame_alloc();

		m_video_width = m_p_codec_ctx_video->width;
		m_video_height = m_p_codec_ctx_video->height;
		m_video_buffer = (std::unique_ptr<Image>)(new Image(m_video_width, m_video_height));

		av_image_fill_arrays(m_p_frm_rgb_video->data, m_p_frm_rgb_video->linesize, m_video_buffer->data(), AV_PIX_FMT_RGBA, m_video_width, m_video_height, 1);
		m_sws_ctx = sws_getContext(m_video_width, m_video_height, m_p_codec_ctx_video->pix_fmt, m_video_width, m_video_height, AV_PIX_FMT_RGBA, SWS_BICUBIC, nullptr, nullptr, nullptr);

		m_video_bufs[0] = (std::unique_ptr<Image>)(new Image(m_video_width, m_video_height));
		m_video_bufs[1] = (std::unique_ptr<Image>)(new Image(m_video_width, m_video_height));
		m_video_bufs[2] = (std::unique_ptr<Image>)(new Image(m_video_width, m_video_height));
	}

	if (m_a_idx >= 0)
	{
		m_queue_audio = (std::unique_ptr<PacketQueue>)(new PacketQueue(42));
	}

	if (m_v_idx >= 0)
	{
		m_queue_video = (std::unique_ptr<PacketQueue>)(new PacketQueue(30));
	}

	m_p_packet = std::unique_ptr<AVPacket>(new AVPacket);

	m_sync_progress = 0;
}


MMPlayer::~MMPlayer()
{
	stop();

	if (m_v_idx >= 0)
	{
		sws_freeContext(m_sws_ctx);
		av_frame_free(&m_p_frm_rgb_video);
		av_frame_free(&m_p_frm_raw_video);
		avcodec_free_context(&m_p_codec_ctx_video);
	}

	if (m_a_idx >= 0)
	{
		swr_free(&m_swr_ctx);
		av_frame_free(&m_p_frm_s16_audio);
		av_frame_free(&m_p_frm_raw_audio);
		avcodec_free_context(&m_p_codec_ctx_audio);
	}
	avformat_close_input(&m_p_fmt_ctx);
}


bool MMPlayer::is_playing() const
{
	return m_thread_demux != nullptr;
}

bool MMPlayer::is_eof_reached() const
{
	return m_thread_demux != nullptr && m_audio_eof && m_video_eof;
}


uint64_t MMPlayer::get_position() const
{
	if (m_thread_demux == nullptr)
	{
		return m_sync_progress;
	}
	else if (m_audio_eof && m_video_eof)
	{
		return get_duration();
	}
	else
	{
		uint64_t localtime, progress;
		_get_sync_point(localtime, progress);
		return progress + (time_micro_sec() - localtime) * m_speed;
	}
}

void MMPlayer::stop()
{
	if (m_thread_demux != nullptr)
	{
		uint64_t pos = get_position();

		m_demuxing = false;
		m_audio_playing = false;
		m_video_playing = false;
		m_audio_playback = nullptr;
		m_video_playback = nullptr;

		while (m_a_idx >= 0 && m_queue_audio->Size() > 0)
		{
			AVPacket packet = m_queue_audio->Pop();
			av_packet_unref(&packet);
		}

		while (m_v_idx >= 0 && m_queue_video->Size() > 0)
		{
			AVPacket packet = m_queue_video->Pop();
			av_packet_unref(&packet);
		}

		m_thread_demux->join();
		m_thread_demux = nullptr;

		m_sync_progress = pos;
	}
}

void MMPlayer::_start(uint64_t pos)
{
	stop();
	avformat_seek_file(m_p_fmt_ctx, -1, INT64_MIN, pos, INT64_MAX, 0);
	m_sync_local_time = time_micro_sec();
	m_sync_progress = pos;

	m_demuxing = true;
	m_thread_demux = (std::unique_ptr<std::thread>)(new std::thread(thread_demux, this));

	if (m_v_idx >= 0)
	{
		m_video_playing = true;
		m_video_eof = false;
		m_video_playback = (std::unique_ptr<VideoPlayback>)(new VideoPlayback(this));
	}

	if (m_a_idx >= 0)
	{
		m_audio_playing = true;
		m_audio_eof = false;
		m_audio_playback = (std::unique_ptr<AudioPlayback>)(new AudioPlayback(m_id_audio_device, this));
	}
}

void MMPlayer::start()
{
	if (m_thread_demux == nullptr)
	{
		_start(m_sync_progress);
	}
}

void MMPlayer::set_position(uint64_t pos)
{
	if (m_thread_demux != nullptr)
	{
		_start(pos);
	}
	else
	{
		if (m_v_idx >= 0)
		{
			avformat_seek_file(m_p_fmt_ctx, -1, INT64_MIN, pos, INT64_MAX, 0);

			bool frame_read = false;
			while (av_read_frame(m_p_fmt_ctx, m_p_packet.get()) == 0)
			{
				if (m_p_packet->stream_index == m_v_idx)
				{
					int64_t t_frame = m_p_packet->dts * m_video_time_base_num * AV_TIME_BASE / m_video_time_base_den;
					avcodec_send_packet(m_p_codec_ctx_video, m_p_packet.get());
					avcodec_receive_frame(m_p_codec_ctx_video, m_p_frm_raw_video);
					av_packet_unref(m_p_packet.get());

					if (t_frame >= pos)
					{
						sws_scale(m_sws_ctx, (const uint8_t* const*)m_p_frm_raw_video->data, m_p_frm_raw_video->linesize,
							0, m_p_codec_ctx_video->height, m_p_frm_rgb_video->data, m_p_frm_rgb_video->linesize);
						frame_read = true;
						break;
					}
				}
				else
				{
					av_packet_unref(m_p_packet.get());
				}
			}

			if (frame_read)
			{
				int this_buf = (m_last_video_buf + 1) % 3;
				*m_video_bufs[this_buf] = *m_video_buffer;
				m_last_video_buf = this_buf;
				m_updated = true;
			}
		}
		m_sync_progress = pos;
	}
}

void MMPlayer::set_audio_device(int id_audio_device)
{
	if (id_audio_device != m_id_audio_device)
	{
		m_id_audio_device = id_audio_device;
		if (m_a_idx >= 0 && m_thread_demux != nullptr)
		{
			m_audio_playing = false;
			m_audio_playback = nullptr;
			m_audio_playing = true;
			m_audio_eof = false;
			m_audio_playback = (std::unique_ptr<AudioPlayback>)(new AudioPlayback(m_id_audio_device, this));
		}

	}
}

void MMPlayer::_set_sync_point(uint64_t local_time, uint64_t progress)
{
	m_mutex_sync.lock();
	m_sync_local_time = local_time;
	m_sync_progress = progress;
	m_mutex_sync.unlock();
}

void MMPlayer::_get_sync_point(uint64_t& local_time, uint64_t& progress) const
{
	m_mutex_sync.lock();
	local_time = m_sync_local_time;
	progress = m_sync_progress;
	m_mutex_sync.unlock();
}

void MMPlayer::thread_demux(MMPlayer* self)
{
	while (self->m_demuxing && av_read_frame(self->m_p_fmt_ctx, self->m_p_packet.get()) == 0)
	{
		if (self->m_p_packet->stream_index == self->m_a_idx)
		{
			self->m_queue_audio->Push(*self->m_p_packet);
		}
		else if (self->m_p_packet->stream_index == self->m_v_idx)
		{
			self->m_queue_video->Push(*self->m_p_packet);
		}
		else
		{
			av_packet_unref(self->m_p_packet.get());
		}
	}
	self->m_demuxing = false;
}


const Image* MMPlayer::read_video_frame()
{
	if (m_updated)
	{
		m_updated = false;
		return m_video_bufs[m_last_video_buf].get();
	}
	return nullptr;
}


const std::vector<std::string>& GetNamesAudioPlaybackDevices(bool refresh, int* id_default)
{
	return AudioOut::s_get_list_audio_devices(refresh, id_default);
}

MMAudio::MMAudio(const char* filename, int id_audio_device, int speed)
{
	if (id_audio_device < 0)
	{
		GetNamesAudioPlaybackDevices(false, &id_audio_device);
	}
	m_internal = (std::unique_ptr<MMPlayer>)(new MMPlayer(filename, true, false, id_audio_device, speed));
}

MMAudio::~MMAudio()
{

}

bool MMAudio::is_playing() const
{
	return m_internal->is_playing() && !m_internal->is_eof_reached();
}

double MMAudio::get_total_duration_s() const
{
	return (double)m_internal->get_duration() / 1000000.0;
}


double MMAudio::get_current_pos_s() const
{
	return (double)m_internal->get_position() / 1000000.0;
}

void MMAudio::play()
{
	m_internal->start();
}

void MMAudio::pause()
{
	m_internal->stop();
}

void MMAudio::set_pos_s(double pos)
{
	m_internal->set_position((uint64_t)(pos * 1000000.0));
}

void MMAudio::set_audio_device(int id_audio_device)
{
	m_internal->set_audio_device(id_audio_device);
}

void MMAudio::check_eof()
{
	if (!is_playing())
	{
		if (m_internal->is_eof_reached() && m_is_loop) set_pos_s(0.0);
	}
}

MMVideo::MMVideo(const char* filename, bool play_audio, int id_audio_device, int speed):
	m_tex(new GLTexture2D)
{
	if (play_audio && id_audio_device < 0)
	{
		GetNamesAudioPlaybackDevices(false, &id_audio_device);
	}
	m_internal = (std::unique_ptr<MMPlayer>)(new MMPlayer(filename, play_audio, true, id_audio_device, speed));
}

MMVideo::~MMVideo()
{

}


int MMVideo::width() const
{
	return m_internal->video_width();
}

int MMVideo::height() const
{
	return m_internal->video_height();
}

bool MMVideo::is_playing() const
{
	return m_internal->is_playing() && !m_internal->is_eof_reached();
}

double MMVideo::get_total_duration_s() const
{
	return (double)m_internal->get_duration() / 1000000.0;
}

double MMVideo::get_current_pos_s() const
{
	return (double)m_internal->get_position() / 1000000.0;
}


void MMVideo::play()
{
	m_internal->start();
}

void MMVideo::pause()
{
	m_internal->stop();
}

void MMVideo::set_pos_s(double pos)
{
	m_internal->set_position((uint64_t)(pos * 1000000.0));
}

void MMVideo::set_audio_device(int id_audio_device)
{
	m_internal->set_audio_device(id_audio_device);
}

GLTexture2D* MMVideo::get_texture()
{
	return m_tex.get();
}

void MMVideo::update_texture()
{
	if (!is_playing())
	{
		if (m_internal->is_eof_reached() && m_is_loop)
		{
			set_pos_s(0.0);
		}
		else
		{
			return;
		}
	}

	const Image* img_in = m_internal->read_video_frame();
	if (img_in)
	{
		m_tex->load_memory_rgba(img_in->width(), img_in->height(), img_in->data(), true);
	}
}
