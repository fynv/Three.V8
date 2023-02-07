#include "ProbeGridSaver.h"
#include "lights/ProbeGrid.h"

void ProbeGridSaver::SaveFile(const ProbeGrid* probe_grid, const char* fn)
{
	FILE* fp = fopen(fn, "wb");
	fwrite(&probe_grid->coverage_min, sizeof(glm::vec3), 1, fp);
	fwrite(&probe_grid->coverage_max, sizeof(glm::vec3), 1, fp);
	fwrite(&probe_grid->divisions, sizeof(glm::ivec3), 1, fp);
	fwrite(probe_grid->m_probe_data.data(), sizeof(glm::vec4), probe_grid->m_probe_data.size(), fp);
	fclose(fp);
}

