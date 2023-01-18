#pragma once
#include <memory>

class OpusPlayer
{
public:
	OpusPlayer(int id_audio_device = -1);
	~OpusPlayer();

	void AddPacket(size_t size, const uint8_t* data);

private:
	class Internal;
	std::unique_ptr<Internal> m_internal;
};

