#include "ProbeGridLoader.h"
#include "lights/ProbeGrid.h"

void ProbeGridLoader::LoadFile(ProbeGrid* probe_grid, const char* fn)
{
	FILE* fp = fopen(fn, "rb");
	fread(&probe_grid->coverage_min, sizeof(glm::vec3), 1, fp);
	fread(&probe_grid->coverage_max, sizeof(glm::vec3), 1, fp);
	fread(&probe_grid->divisions, sizeof(glm::ivec3), 1, fp);
	probe_grid->m_probe_data.resize(probe_grid->divisions.x * probe_grid->divisions.y * probe_grid->divisions.z * 9);
	fread(probe_grid->m_probe_data.data(), sizeof(glm::vec4), probe_grid->m_probe_data.size(), fp);
	probe_grid->allocate_probes();
	fclose(fp);
}

