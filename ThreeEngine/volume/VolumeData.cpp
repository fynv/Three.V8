#include "VolumeData.h"

void VolumeData::GetMinMax(glm::vec3& min_pos, glm::vec3& max_pos)
{
	glm::vec3 dims = glm::vec3(size) * spacing;
	min_pos = -dims * 0.5f;
	max_pos = dims * 0.5f;
}
