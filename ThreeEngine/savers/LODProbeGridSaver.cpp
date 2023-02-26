#include "LODProbeGridSaver.h"
#include "lights/LODProbeGrid.h"

void LODProbeGridSaver::SaveFile(const LODProbeGrid* probe_grid, const char* fn)
{
	FILE* fp = fopen(fn, "wb");
	fwrite(&probe_grid->coverage_min, sizeof(glm::vec3), 1, fp);
	fwrite(&probe_grid->coverage_max, sizeof(glm::vec3), 1, fp);
	fwrite(&probe_grid->base_divisions, sizeof(glm::ivec3), 1, fp);
	fwrite(&probe_grid->sub_division_level, sizeof(int), 1, fp);
	int num_probes = probe_grid->getNumberOfProbes();
	int num_indices = (int)probe_grid->m_sub_index.size();
	fwrite(&num_probes, sizeof(int), 1, fp);
	fwrite(&num_indices, sizeof(int), 1, fp);
	fwrite(probe_grid->m_probe_data.data(), sizeof(glm::vec4), probe_grid->m_probe_data.size(), fp);
	fwrite(probe_grid->m_sub_index.data(), sizeof(int), probe_grid->m_sub_index.size(), fp);
	fclose(fp);
}


