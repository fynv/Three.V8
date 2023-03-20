#include <half.hpp>
#include "ProbeGridSaver.h"
#include "lights/ProbeGrid.h"

void ProbeGridSaver::SaveFile(ProbeGrid* probe_grid, const char* fn)
{
	if (probe_grid->updated)
	{
		probe_grid->download_probes();
	}

	FILE* fp = fopen(fn, "wb");
	fwrite(&probe_grid->coverage_min, sizeof(glm::vec3), 1, fp);
	fwrite(&probe_grid->coverage_max, sizeof(glm::vec3), 1, fp);
	fwrite(&probe_grid->divisions, sizeof(glm::ivec3), 1, fp);
	fwrite(&probe_grid->ypower, sizeof(float), 1, fp);
	fwrite(&probe_grid->vis_res, sizeof(int), 1, fp);
	fwrite(&probe_grid->pack_size, sizeof(int), 1, fp);
	fwrite(&probe_grid->pack_res, sizeof(int), 1, fp);
	fwrite(probe_grid->m_probe_data.data(), sizeof(glm::vec4), probe_grid->m_probe_data.size(), fp);

	std::vector<half_float::half> vec_h(probe_grid->m_visibility_data.size());
	for (size_t i = 0; i < vec_h.size(); i++)
	{
		vec_h[i] = probe_grid->m_visibility_data[i];
	}	

	fwrite(vec_h.data(), sizeof(half_float::half), vec_h.size(), fp);
	fclose(fp);
}

