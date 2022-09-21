#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>


namespace std
{
	class thread;
}

class Image;
class GLTexture2D;

struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct SwsContext;
struct AVPacket;

class MMCamera
{
public:
	static const std::vector<std::string>& s_get_list_devices(bool refresh = false);


	MMCamera(int idx = 0);
	~MMCamera();

	int idx() const { return m_idx; }
	int width() const { return m_width; }
	int height() const { return m_height; }
	GLTexture2D* get_texture();
	void update_texture();

private:
	int m_idx;
	int m_width, m_height;
	std::unique_ptr<GLTexture2D> m_tex;
	std::unique_ptr<Image> m_img;
	std::unique_ptr<Image> m_bufs[3];
	int m_last_buf = 2;
	bool m_updated = true;
	bool m_quit = false;
	int m_frame_rate_num, m_frame_rate_den;
	uint64_t m_start_time;
	size_t m_frame_count = 0;

	static void thread_read(MMCamera* self);
	std::unique_ptr<std::thread> m_thread_read;

	int m_v_idx;
	AVFormatContext* m_p_fmt_ctx;
	AVCodecContext* m_p_codec_ctx;
	AVFrame* m_p_frm_raw;
	AVFrame* m_p_frm_bgr;
	SwsContext* m_sws_ctx;
	std::unique_ptr<AVPacket> m_p_packet;

};


