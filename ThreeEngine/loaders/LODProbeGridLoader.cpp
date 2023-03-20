#include <half.hpp>
#include "LODProbeGridLoader.h"
#include "lights/LODProbeGrid.h"

void LODProbeGridLoader::LoadFile(LODProbeGrid* probe_grid, const char* fn)
{
	FILE* fp = fopen(fn, "rb");
	fseek(fp, 0, SEEK_END);
	size_t size = (size_t)ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fread(&probe_grid->coverage_min, sizeof(glm::vec3), 1, fp);
	fread(&probe_grid->coverage_max, sizeof(glm::vec3), 1, fp);
	fread(&probe_grid->base_divisions, sizeof(glm::ivec3), 1, fp);
	fread(&probe_grid->sub_division_level, sizeof(int), 1, fp);

	int num_probes, num_indices;
	fread(&num_probes, sizeof(int), 1, fp);
	fread(&num_indices, sizeof(int), 1, fp);

	fread(&probe_grid->vis_res, sizeof(int), 1, fp);
	fread(&probe_grid->pack_size, sizeof(int), 1, fp);
	fread(&probe_grid->pack_res, sizeof(int), 1, fp);

	probe_grid->m_probe_data.resize(num_probes * 10);
	fread(probe_grid->m_probe_data.data(), sizeof(glm::vec4), num_probes * 10, fp);

	probe_grid->m_sub_index.resize(num_indices);
	fread(probe_grid->m_sub_index.data(), sizeof(int), num_indices, fp);

	int pack_res = probe_grid->pack_res;
	std::vector<half_float::half> vec_h(pack_res * pack_res * 2);
	fread(vec_h.data(), sizeof(half_float::half), vec_h.size(), fp);

	probe_grid->m_visibility_data.resize(vec_h.size());
	for (size_t i = 0; i < vec_h.size(); i++)
	{
		probe_grid->m_visibility_data[i] = vec_h[i];
	}

	probe_grid->updateBuffers();
	fclose(fp);
}



void LODProbeGridLoader::LoadMemory(LODProbeGrid* probe_grid, unsigned char* data, size_t size)
{
	unsigned char* ptr = data;
	probe_grid->coverage_min = *(glm::vec3*)ptr; ptr += sizeof(glm::vec3);
	probe_grid->coverage_max = *(glm::vec3*)ptr; ptr += sizeof(glm::vec3);
	probe_grid->base_divisions = *(glm::ivec3*)ptr; ptr += sizeof(glm::ivec3);
	probe_grid->sub_division_level = *(int*)ptr; ptr += sizeof(int);
	int num_probes = *(int*)ptr; ptr += sizeof(int);
	int num_indices = *(int*)ptr; ptr += sizeof(int);
	probe_grid->vis_res = *(int*)ptr; ptr += sizeof(int);
	probe_grid->pack_size = *(int*)ptr; ptr += sizeof(int);
	probe_grid->pack_res = *(int*)ptr; ptr += sizeof(int);

	probe_grid->m_probe_data.resize(num_probes * 10);	
	memcpy(probe_grid->m_probe_data.data(), ptr, sizeof(glm::vec4) * num_probes * 10);
	ptr += sizeof(glm::vec4) * num_probes * 10;

	probe_grid->m_sub_index.resize(num_indices);
	memcpy(probe_grid->m_sub_index.data(), ptr, sizeof(int) * num_indices);
	ptr += sizeof(int) * num_indices;

	int pack_res = probe_grid->pack_res;
	std::vector<half_float::half> vec_h(pack_res * pack_res * 2);
	memcpy(vec_h.data(), ptr, sizeof(half_float::half) * vec_h.size());

	probe_grid->m_visibility_data.resize(vec_h.size());
	for (size_t i = 0; i < vec_h.size(); i++)
	{
		probe_grid->m_visibility_data[i] = vec_h[i];
	}

	probe_grid->updateBuffers();
}
