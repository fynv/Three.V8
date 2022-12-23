#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class VolumeData;
class GridPartition;

class InitGridPartition
{
public:
	InitGridPartition(int bytes_per_pixel);

	void Init(const VolumeData& data, GridPartition& partition, float blockLogRate);

private:
	std::unique_ptr<GLProgram> m_prog;

};