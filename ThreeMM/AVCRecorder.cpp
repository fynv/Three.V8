#include <d3d11_1.h>
#pragma comment(lib, "DXGI.lib")

#include <thread>
#include <vector>
#include <queue>
#include "utils/Image.h"
#include "utils/Utils.h"
#include "utils/Semaphore.h"
#include "AVCRecorder.h"
#include "MMCamera.h"


extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/log.h>
#include <libswscale/swscale.h>
}

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

static const AVCodecID video_codec_id = AV_CODEC_ID_H264;
static const AVPixelFormat pix_fmt = AV_PIX_FMT_NV12;

class AVCRecorder::Internal
{
public:
	Internal(int id_camera)
	{
		const auto& lst = MMCamera::s_get_list_devices();
		std::string url = std::string("video=") + lst[id_camera];

		AVInputFormat* inFrmt = av_find_input_format("dshow");
		m_p_fmt_ctx = nullptr;
		avformat_open_input(&m_p_fmt_ctx, url.c_str(), inFrmt, nullptr);
		avformat_find_stream_info(m_p_fmt_ctx, nullptr);

		m_v_idx = -1;
		for (unsigned i = 0; i < m_p_fmt_ctx->nb_streams; i++)
		{
			if (m_p_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				m_v_idx = i;
				m_frame_rate_num = m_p_fmt_ctx->streams[i]->avg_frame_rate.num;
				m_frame_rate_den = m_p_fmt_ctx->streams[i]->avg_frame_rate.den;
				break;
			}
		}

		AVCodecParameters* p_codec_par = m_p_fmt_ctx->streams[m_v_idx]->codecpar;
		AVCodec* p_codec_in = avcodec_find_decoder(p_codec_par->codec_id);
		m_p_codec_ctx_in = avcodec_alloc_context3(p_codec_in);
		avcodec_parameters_to_context(m_p_codec_ctx_in, p_codec_par);
		avcodec_open2(m_p_codec_ctx_in, p_codec_in, nullptr);

		m_width = m_p_codec_ctx_in->width;
		m_height = m_p_codec_ctx_in->height;
		
		m_p_frm_raw = av_frame_alloc();

		bool nvidia = false;
		bool intel = false;
		bool amd = false;
		{
			IDXGIFactory* pFactory;
			HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));
			UINT i = 0;
			IDXGIAdapter* pAdapter;
			while (pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
			{
				DXGI_ADAPTER_DESC desc;
				pAdapter->GetDesc(&desc);
				std::wstring name = desc.Description;
				std::transform(name.begin(), name.end(), name.begin(), [](wchar_t c) { return std::tolower(c); });
				if (name.find(L"nvidia") != std::string::npos)
				{
					nvidia = true;
				}
				else if (name.find(L"intel") != std::string::npos)
				{
					intel = true;
				}
				else if (name.find(L"amd") != std::string::npos)
				{
					amd = true;
				}
				++i;
			}
		}		

		const AVCodec* p_codec_out = nullptr;
		if (nvidia)
		{
			p_codec_out = avcodec_find_encoder_by_name("h264_nvenc");
		}
		/*else if (intel)
		{
			p_codec_out = avcodec_find_encoder_by_name("h264_qsv");
		}*/
		else if (amd)
		{
			p_codec_out = avcodec_find_encoder_by_name("h264_amf");
		}
		else
		{
			p_codec_out = avcodec_find_encoder(video_codec_id);
		}

		m_p_codec_ctx_out = avcodec_alloc_context3(p_codec_out);
		m_p_codec_ctx_out->codec_id = video_codec_id;

		static double a = 0.078;
		static double b = 0.2027;
		double mega_pixels = (double)(m_width * m_height) / 1000000.0;
		double mega_bps = (sqrt(b * b + 4 * a * mega_pixels) - b) / (2 * a);

		m_p_codec_ctx_out->bit_rate = (int64_t)(mega_bps * 1000000.0);
		m_p_codec_ctx_out->width = m_width;
		m_p_codec_ctx_out->height = m_height;
		m_p_codec_ctx_out->time_base = { m_frame_rate_den, m_frame_rate_num };
		m_p_codec_ctx_out->framerate = { m_frame_rate_num, m_frame_rate_den };
		m_p_codec_ctx_out->max_b_frames = 0;
		m_p_codec_ctx_out->gop_size = 15;
		m_p_codec_ctx_out->pix_fmt = pix_fmt;

		if (nvidia)
		{
			av_opt_set(m_p_codec_ctx_out->priv_data, "profile", "baseline", 0);
			av_opt_set(m_p_codec_ctx_out->priv_data, "zerolatency", "1", 0);
			av_opt_set(m_p_codec_ctx_out->priv_data, "delay", "0", 0);
		}
		else if (amd)
		{
			av_opt_set(m_p_codec_ctx_out->priv_data, "usage", "ultralowlatency", 0);
			av_opt_set(m_p_codec_ctx_out->priv_data, "profile", "constrained_baseline", 0);
		}
		else
		{
			av_opt_set(m_p_codec_ctx_out->priv_data, "profile", "baseline", 0);
			av_opt_set(m_p_codec_ctx_out->priv_data, "realtime", "1", AV_OPT_SEARCH_CHILDREN);
			av_opt_set(m_p_codec_ctx_out->priv_data, "preset", "ultrafast", 0);
			av_opt_set(m_p_codec_ctx_out->priv_data, "tune", "zerolatency", 0);
		}

		avcodec_open2(m_p_codec_ctx_out, p_codec_out, nullptr);

		m_p_frm_yuv = av_frame_alloc();
		m_p_frm_yuv->format = pix_fmt;
		m_p_frm_yuv->width = m_width;
		m_p_frm_yuv->height = m_height;
		av_frame_get_buffer(m_p_frm_yuv, 0);

		m_sws_ctx = sws_getContext(m_width, m_height, m_p_codec_ctx_in->pix_fmt, m_width, m_height, pix_fmt, SWS_BILINEAR, nullptr, nullptr, nullptr);		

		m_start_time = time_micro_sec();
		m_thread_encode = (std::unique_ptr<std::thread>)(new std::thread(thread_encode, this));
	}

	~Internal()
	{
		m_running = false;
		m_thread_encode->join();

		sws_freeContext(m_sws_ctx);
		av_frame_free(&m_p_frm_yuv);
		avcodec_free_context(&m_p_codec_ctx_out);

		av_frame_free(&m_p_frm_raw);
		avcodec_free_context(&m_p_codec_ctx_in);
		avformat_close_input(&m_p_fmt_ctx);
	}

	ConcurrentQueue<std::vector<uint8_t>> m_pkt_queue;

private:
	int m_width, m_height;
	int m_frame_rate_num, m_frame_rate_den;
	uint64_t m_start_time;
	size_t m_frame_count = 0;

	int m_v_idx;
	AVFormatContext* m_p_fmt_ctx;
	AVCodecContext* m_p_codec_ctx_in;
	AVFrame* m_p_frm_raw;

	AVCodecContext* m_p_codec_ctx_out;
	AVFrame* m_p_frm_yuv;
	int64_t m_next_pts = 0;

	struct SwsContext* m_sws_ctx = nullptr;

	static void thread_encode(Internal* self)
	{
		while (self->m_running)
		{
			self->m_frame_count++;			
			uint64_t target = self->m_start_time + self->m_frame_count * self->m_frame_rate_den * AV_TIME_BASE / self->m_frame_rate_num;

			uint64_t now = time_micro_sec();
			int64_t delta = target - now;
			if (delta > 0)
				std::this_thread::sleep_for(std::chrono::microseconds(delta));
			{
				AVPacket pkt_in = {};
				while (true)
				{
					av_read_frame(self->m_p_fmt_ctx, &pkt_in);
					if (pkt_in.stream_index == self->m_v_idx) break;
					else
					{
						av_packet_unref(&pkt_in);
					}
				}
				avcodec_send_packet(self->m_p_codec_ctx_in, &pkt_in);
				avcodec_receive_frame(self->m_p_codec_ctx_in, self->m_p_frm_raw);
				av_packet_unref(&pkt_in);
			}
			{

				av_frame_make_writable(self->m_p_frm_yuv);
				sws_scale(self->m_sws_ctx, self->m_p_frm_raw->data, self->m_p_frm_raw->linesize, 0, self->m_height, self->m_p_frm_yuv->data, self->m_p_frm_yuv->linesize);
				self->m_p_frm_yuv->pts = self->m_next_pts++;

				int ret = avcodec_send_frame(self->m_p_codec_ctx_out, self->m_p_frm_yuv);

				while (ret >= 0)
				{
					AVPacket pkt = {};
					ret = avcodec_receive_packet(self->m_p_codec_ctx_out, &pkt);
					if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
						break;

					std::vector<uint8_t> packet(pkt.size);
					memcpy(packet.data(), pkt.data, pkt.size);
					self->m_pkt_queue.Push(packet);

					av_packet_unref(&pkt);
				}
			}
		}
	}

	bool m_running = true;
	std::unique_ptr<std::thread> m_thread_encode;
};


AVCRecorder::AVCRecorder(int id_camera)
{
	static bool s_first_time = true;
	if (s_first_time)
	{
		av_log_set_level(AV_LOG_QUIET);
		avdevice_register_all();
		s_first_time = false;
	}

	m_internal = std::unique_ptr<Internal>(new Internal(id_camera));	
}


AVCRecorder::~AVCRecorder()
{

}

void AVCRecorder::SetCallback(PacketCallback callback, void* callback_data)
{
	this->callback = callback;
	this->callback_data = callback_data;
}

void* AVCRecorder::GetCallbackData()
{
	return callback_data;
}

void AVCRecorder::CheckPending()
{
	while (m_internal->m_pkt_queue.Size() > 0)
	{
		std::vector<uint8_t> pkt = m_internal->m_pkt_queue.Pop();
		if (callback != nullptr)
		{
			callback(pkt.data(), pkt.size(), callback_data);
		}
	}
}

