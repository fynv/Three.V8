#pragma once

#include <memory>
#include <vector>
#include <string>

const std::vector<std::string>& GetNamesAudioPlaybackDevices(bool refresh, int* id_default = nullptr);

class MMPlayer;
class MMAudio
{
public:
	bool m_is_loop = false;

	MMAudio(const char* filename, int id_audio_device=-1, int speed = 1);
	MMAudio(HttpClient* http, const char* url, int id_audio_device = -1, int speed = 1);
	~MMAudio();

	bool is_playing() const;	
	double get_total_duration_s() const;
	double get_current_pos_s() const;

	void play();
	void pause();
	void set_pos_s(double pos);
	void set_audio_device(int id_audio_device);

	void check_eof();

private:
	std::unique_ptr<MMPlayer> m_internal;

};

class HttpClient;
class GLTexture2D;
class MMVideo
{
public:
	bool m_is_loop = false;

	MMVideo(const char* filename, bool play_audio, int id_audio_device = -1, int speed = 1);
	MMVideo(HttpClient* http, const char* url, bool play_audio, int id_audio_device = -1, int speed = 1);
	~MMVideo();

	int width() const;
	int height() const;

	bool is_playing() const;
	double get_total_duration_s() const;
	double get_current_pos_s() const;

	void play();
	void pause();
	void set_pos_s(double pos);
	void set_audio_device(int id_audio_device);

	GLTexture2D* get_texture();
	void update_texture();

private:
	std::unique_ptr<MMPlayer> m_internal;
	std::unique_ptr<GLTexture2D> m_tex;
};

