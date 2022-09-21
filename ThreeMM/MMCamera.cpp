#include "MMCamera.h"
#include "utils/Image.h"
#include "utils/Utils.h"
#include "renderers/GLUtils.h"


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/log.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}


#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "postproc.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "swresample.lib")

#pragma comment(lib, "strmiids.lib")


#include <dshow.h>
#include <thread>

inline std::string BSTRtoUTF8(BSTR bstr)
{
	int len = (int)SysStringLen(bstr);
	if (len == 0) return "";
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, bstr, len, nullptr, 0, 0, 0);
	std::vector<char> ret(size_needed + 1, (char)0);
	WideCharToMultiByte(CP_UTF8, 0, bstr, len, ret.data(), (int)ret.size(), 0, 0);
	return ret.data();
}

inline void EnumerateCameras(std::vector<std::string>& lst_names)
{
	lst_names.clear();
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	IEnumMoniker* pEnum;
	ICreateDevEnum* pDevEnum;
	CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));
	pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
	if (pEnum != nullptr)
	{
		IMoniker* pMoniker = nullptr;
		while (pEnum->Next(1, &pMoniker, nullptr) == S_OK)
		{
			IPropertyBag* pPropBag;
			pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
			VARIANT var;
			VariantInit(&var);
			pPropBag->Read(L"FriendlyName", &var, 0);
			lst_names.push_back(BSTRtoUTF8(var.bstrVal));
			VariantClear(&var);
			pPropBag->Release();
			pMoniker->Release();
		}
		pEnum->Release();
	}
	CoUninitialize();
}

const std::vector<std::string>& MMCamera::s_get_list_devices(bool refresh)
{
	static std::vector<std::string> s_list_devices;
	if (s_list_devices.size() == 0 || refresh)
		EnumerateCameras(s_list_devices);
	return s_list_devices;
}


MMCamera::MMCamera(int idx)
{
	static bool s_first_time = true;
	if (s_first_time)
	{
		av_log_set_level(AV_LOG_QUIET);
		avdevice_register_all();
		s_first_time = false;
	}

	const auto& lst = s_get_list_devices();
	std::string url = std::string("video=") + lst[idx];

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
	AVCodec* p_codec = avcodec_find_decoder(p_codec_par->codec_id);
	m_p_codec_ctx = avcodec_alloc_context3(p_codec);
	avcodec_parameters_to_context(m_p_codec_ctx, p_codec_par);
	avcodec_open2(m_p_codec_ctx, p_codec, nullptr);

	m_width = m_p_codec_ctx->width;
	m_height = m_p_codec_ctx->height;
	m_tex = (std::unique_ptr<GLTexture2D>)(new GLTexture2D);
	m_img = (std::unique_ptr<Image>)(new Image(m_width, m_height));
	m_bufs[0] = (std::unique_ptr<Image>)(new Image(m_width, m_height));
	m_bufs[1] = (std::unique_ptr<Image>)(new Image(m_width, m_height));
	m_bufs[2] = (std::unique_ptr<Image>)(new Image(m_width, m_height));

	m_p_frm_raw = av_frame_alloc();
	m_p_frm_bgr = av_frame_alloc();

	av_image_fill_arrays(m_p_frm_bgr->data, m_p_frm_bgr->linesize, m_img->data(), AV_PIX_FMT_RGBA, m_width, m_height, 1);
	m_sws_ctx = sws_getContext(m_width, m_height, m_p_codec_ctx->pix_fmt, m_width, m_height, AV_PIX_FMT_RGBA, SWS_BICUBIC, nullptr, nullptr, nullptr);

	m_p_packet = std::unique_ptr<AVPacket>(new AVPacket);
	m_start_time = time_micro_sec();
	m_thread_read = (std::unique_ptr<std::thread>)(new std::thread(thread_read, this));

}

MMCamera::~MMCamera()
{
	m_quit = true;
	m_thread_read->join();

	sws_freeContext(m_sws_ctx);
	av_frame_free(&m_p_frm_bgr);
	av_frame_free(&m_p_frm_raw);
	avcodec_free_context(&m_p_codec_ctx);
	avformat_close_input(&m_p_fmt_ctx);
}


void MMCamera::thread_read(MMCamera* self)
{
	while (!self->m_quit)
	{
		self->m_frame_count++;
		uint64_t target = self->m_start_time + self->m_frame_count * self->m_frame_rate_den * AV_TIME_BASE / self->m_frame_rate_num;
		uint64_t now = time_micro_sec();
		int64_t delta = target - now;
		if (delta > 0)
			std::this_thread::sleep_for(std::chrono::microseconds(delta));

		while (true)
		{
			av_read_frame(self->m_p_fmt_ctx, self->m_p_packet.get());
			if (self->m_p_packet->stream_index == self->m_v_idx) break;
			else
			{
				av_packet_unref(self->m_p_packet.get());
			}
		}
		avcodec_send_packet(self->m_p_codec_ctx, self->m_p_packet.get());
		avcodec_receive_frame(self->m_p_codec_ctx, self->m_p_frm_raw);
		av_packet_unref(self->m_p_packet.get());

		sws_scale(self->m_sws_ctx, (const uint8_t* const*)self->m_p_frm_raw->data, self->m_p_frm_raw->linesize, 0, self->m_p_codec_ctx->height, self->m_p_frm_bgr->data, self->m_p_frm_bgr->linesize);

		int this_buf = (self->m_last_buf + 1) % 3;
		*self->m_bufs[this_buf] = *self->m_img;
		self->m_last_buf = this_buf;
		self->m_updated = true;
	}
}

GLTexture2D* MMCamera::get_texture()
{
	return m_tex.get();
}

void MMCamera::update_texture()
{
	if (m_updated)
	{
		m_updated = false;
		Image* img = m_bufs[m_last_buf].get();
		m_tex->load_memory_rgba(img->width(), img->height(), img->data(), true);
	}
}
