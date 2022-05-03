#pragma once

class IndirectLight
{
public:
	IndirectLight() {}
	virtual ~IndirectLight() {}

	// tone_shading
	float diffuse_thresh = 0.2f;
	float diffuse_high = 0.8f;
	float diffuse_low = 0.2f;
	float specular_thresh = 0.2f;
	float specular_high = 0.8f;
	float specular_low = 0.2f;
};

