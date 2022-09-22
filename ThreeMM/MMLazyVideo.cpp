#include "MMIOContext.h"
#include "MMLazyVideo.h"
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

class MMLazyVideo::Internal
{
public:
	Internal(const char* fn)
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
			if (m_p_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				m_v_idx = i;
				m_video_time_base_num = m_p_fmt_ctx->streams[i]->time_base.num;
				m_video_time_base_den = m_p_fmt_ctx->streams[i]->time_base.den;
			}
		}

		if (m_v_idx < 0)
		{
			printf("%s is not a video file.\n", fn);
		}

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

		m_p_packet = std::unique_ptr<AVPacket>(new AVPacket);

		m_position = 0;
	}

	~Internal()
	{
		sws_freeContext(m_sws_ctx);
		av_frame_free(&m_p_frm_rgb_video);
		av_frame_free(&m_p_frm_raw_video);
		avcodec_free_context(&m_p_codec_ctx_video);
		avformat_close_input(&m_p_fmt_ctx);
	}

	int width() const { return m_video_width; }
	int height() const { return m_video_height; }
	uint64_t get_duration() const { return m_duration; }
	uint64_t get_position() const { return m_position; }
	void set_position(uint64_t pos)
	{
		m_position = pos;
		avformat_seek_file(m_p_fmt_ctx, -1, INT64_MIN, pos, INT64_MAX, 0);
	}

	bool fetch()
	{
		bool frame_read = false;
		while (av_read_frame(m_p_fmt_ctx, m_p_packet.get()) == 0)
		{
			if (m_p_packet->stream_index == m_v_idx)
			{
				int64_t t = m_p_packet->dts * m_video_time_base_num * AV_TIME_BASE / m_video_time_base_den;
				avcodec_send_packet(m_p_codec_ctx_video, m_p_packet.get());
				avcodec_receive_frame(m_p_codec_ctx_video, m_p_frm_raw_video);
				av_packet_unref(m_p_packet.get());
				if (t >= (int64_t)m_position)
				{
					m_position = (uint64_t)t;
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
		return frame_read;
	}

	const Image* get_image() const
	{
		return m_video_buffer.get();
	}

private:
	std::unique_ptr<MMIOContext> m_io_ctx;

	AVFormatContext* m_p_fmt_ctx = nullptr;
	int m_v_idx = -1;
	int m_video_time_base_num, m_video_time_base_den;
	int m_video_width, m_video_height;
	uint64_t m_duration;
	uint64_t m_position;

	AVCodecContext* m_p_codec_ctx_video;
	AVFrame* m_p_frm_raw_video;
	AVFrame* m_p_frm_rgb_video;
	std::unique_ptr<Image> m_video_buffer;
	SwsContext* m_sws_ctx;

	std::unique_ptr<AVPacket> m_p_packet;
};


MMLazyVideo::MMLazyVideo(const char* fn, double speed)
	: m_internal(new Internal(fn))
	, m_speed(speed)
	, m_tex(new GLTexture2D)
{
	m_duration_s = (double)m_internal->get_duration() / 1000000.0;
}

MMLazyVideo::~MMLazyVideo()
{
	
}

int MMLazyVideo::width() const
{
	return m_internal->width();
}

int MMLazyVideo::height() const
{
	return m_internal->height();
}


double MMLazyVideo::get_current_pos_s()
{
	if (m_is_playing)
	{
		return m_start_pos_s + (time_sec() - m_start_time_s) * m_speed;
	}
	else
	{
		return m_start_pos_s;
	}
}


void MMLazyVideo::play()
{
	if (!m_is_playing)
	{
		m_start_time_s = time_sec();
		m_is_playing = true;
	}
}

void MMLazyVideo::pause()
{
	if (m_is_playing)
	{
		m_start_pos_s = get_current_pos_s();
		m_start_time_s = time_sec();
		m_is_playing = false;
	}
}

void MMLazyVideo::set_pos_s(double pos)
{
	m_start_pos_s = pos;
	m_start_time_s = time_sec();
	m_internal->set_position((uint64_t)(pos * 1000000.0));
	if (m_internal->fetch())
	{
		m_updated = true;
	}
}


GLTexture2D* MMLazyVideo::get_texture()
{
	return m_tex.get();
}

void MMLazyVideo::update_texture()
{
	double cur_pos = get_current_pos_s();
	uint64_t cur_pos_micro = (uint64_t)(cur_pos * 1000000.0);

	if (cur_pos >= m_duration_s)
	{
		if (m_is_loop)
		{
			set_pos_s(0.0);
			cur_pos = get_current_pos_s();
			cur_pos_micro = (uint64_t)(cur_pos * 1000000.0);
		}
		else
		{
			return;
		}
	}

	while (m_internal->get_position() < cur_pos_micro)
	{
		if (!m_internal->fetch()) break;
		m_updated = true;
	}	

	if (m_updated)
	{
		const Image* img_in = m_internal->get_image();
		m_tex->load_memory_rgba(img_in->width(), img_in->height(), img_in->data(), true);
		m_updated = false;
	}
}