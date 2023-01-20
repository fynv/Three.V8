#include <thread>
#include <queue>
#include "utils/Image.h"
#include "utils/Semaphore.h"
#include "renderers/GLUtils.h"
#include "AVCPlayer.h"


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
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

static const AVCodecID video_codec_id = AV_CODEC_ID_H264;
static const AVPixelFormat pix_fmt = AV_PIX_FMT_NV12;

class AVCPlayer::Internal
{
public:
	Internal()
	{
		static bool s_first_time = true;
		if (s_first_time)
		{
			av_log_set_level(AV_LOG_QUIET);
			s_first_time = false;
		}

		AVCodec* p_codec = avcodec_find_decoder(video_codec_id);
		m_p_codec_ctx_video = avcodec_alloc_context3(p_codec);
		m_p_codec_ctx_video->bit_rate = 1048576;
		m_p_codec_ctx_video->width = 640;
		m_p_codec_ctx_video->height = 480;
		m_p_codec_ctx_video->framerate = {0, 1};
		m_p_codec_ctx_video->pix_fmt = pix_fmt;
		avcodec_open2(m_p_codec_ctx_video, p_codec, nullptr);

		m_p_frm_raw_video = av_frame_alloc();
		m_p_frm_rgb_video = av_frame_alloc();

		m_thread_read = (std::unique_ptr<std::thread>)(new std::thread(thread_read, this));

	}

	~Internal()
	{
		m_playing = false;
		m_thread_read->join();

		if (m_sws_ctx!=nullptr)
		{
			sws_freeContext(m_sws_ctx);
		}
		av_frame_free(&m_p_frm_rgb_video);
		av_frame_free(&m_p_frm_raw_video);
		avcodec_free_context(&m_p_codec_ctx_video);
	}

	void AddPacket(size_t size, const uint8_t* data)
	{
		AVPacket pkt;
		av_new_packet(&pkt, (int)(size-1));
		memcpy(pkt.data, data+1, size-1);
		if (data[0] == 1)
		{
			pkt.flags |= 1;
			key_recieved = true;
		}

		if (key_recieved)
		{
			m_packet_queue.Push(pkt);
		}
	}


	const Image* read_video_frame()
	{
		if (m_updated)
		{
			m_updated = false;
			return m_video_bufs[m_last_video_buf].get();
		}
		return nullptr;
	}

private:
	bool key_recieved = false;
	ConcurrentQueue<AVPacket> m_packet_queue;

	AVCodecContext* m_p_codec_ctx_video;
	AVFrame* m_p_frm_raw_video;
	AVFrame* m_p_frm_rgb_video;
	std::unique_ptr<Image> m_video_buffer;
	SwsContext* m_sws_ctx = nullptr;

	std::unique_ptr<Image> m_video_bufs[3];
	int m_last_video_buf = 2;
	bool m_updated = false;

	bool m_playing = true;
	std::unique_ptr<std::thread> m_thread_read;

	static void thread_read(Internal* self)
	{
		ConcurrentQueue<AVPacket>& queue = self->m_packet_queue;

		while (self->m_playing)
		{
			while (self->m_playing && queue.Size() == 0)
			{
				std::this_thread::sleep_for(std::chrono::microseconds(10));
			}
			if (!self->m_playing) break;			

			while (queue.Size()>0)
			{
				AVPacket packet = queue.Pop();
				avcodec_send_packet(self->m_p_codec_ctx_video, &packet);
				avcodec_receive_frame(self->m_p_codec_ctx_video, self->m_p_frm_raw_video);
				av_packet_unref(&packet);
			}

			int width = self->m_p_frm_raw_video->width;
			int height = self->m_p_frm_raw_video->height;

			if (self->m_sws_ctx == nullptr)
			{
				self->m_video_buffer = (std::unique_ptr<Image>)(new Image(width, height));
				av_image_fill_arrays(self->m_p_frm_rgb_video->data, self->m_p_frm_rgb_video->linesize, self->m_video_buffer->data(), AV_PIX_FMT_RGBA, width, height, 1);
				self->m_sws_ctx = sws_getContext(width, height, (AVPixelFormat)self->m_p_frm_raw_video->format, width, height, AV_PIX_FMT_RGBA, SWS_BICUBIC, nullptr, nullptr, nullptr);

				self->m_video_bufs[0] = (std::unique_ptr<Image>)(new Image(width, height));
				self->m_video_bufs[1] = (std::unique_ptr<Image>)(new Image(width, height));
				self->m_video_bufs[2] = (std::unique_ptr<Image>)(new Image(width, height));
			}
			sws_scale(self->m_sws_ctx, self->m_p_frm_raw_video->data, self->m_p_frm_raw_video->linesize, 0, height, self->m_p_frm_rgb_video->data, self->m_p_frm_rgb_video->linesize);

			int this_buf = (self->m_last_video_buf + 1) % 3;
			*self->m_video_bufs[this_buf] = *self->m_video_buffer;
			self->m_last_video_buf = this_buf;
			self->m_updated = true;
		}

	}
};


AVCPlayer::AVCPlayer() : m_tex(new GLTexture2D)
{	
	m_internal = std::unique_ptr<Internal>(new Internal());
}

AVCPlayer::~AVCPlayer()
{

}

void AVCPlayer::AddPacket(size_t size, const uint8_t* data)
{
	m_internal->AddPacket(size, data);

}

GLTexture2D* AVCPlayer::get_texture()
{
	return m_tex.get();
}


void AVCPlayer::update_texture()
{
	const Image* img_in = m_internal->read_video_frame();
	if (img_in)
	{
		m_width = img_in->width();
		m_height = img_in->height();
		m_tex->load_memory_rgba(img_in->width(), img_in->height(), img_in->data(), true);
	}
}
