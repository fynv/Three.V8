#pragma once

#include <memory>
class GLTexture2D;


class MMLazyVideo
{
public:
	bool m_is_loop = false;

	MMLazyVideo(const char* fn, double speed);
	~MMLazyVideo();

	int width() const;
	int height() const;

	bool is_playing() { return m_is_playing; }
	double get_total_duration_s() { return m_duration_s; }
	double get_current_pos_s();
	
	void play();
	void pause();
	void set_pos_s(double pos);

	GLTexture2D* get_texture();
	void update_texture();

private:
	class Internal;
	std::unique_ptr<Internal> m_internal;
	std::unique_ptr<GLTexture2D> m_tex;
	bool m_is_playing = false;
	double m_duration_s = 0.0;
	double m_start_pos_s = 0.0;
	double m_start_time_s = 0.0;	
	bool m_updated = false;
	double m_speed;
};