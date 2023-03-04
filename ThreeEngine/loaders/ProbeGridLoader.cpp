#include "ProbeGridLoader.h"
#include "lights/ProbeGrid.h"

void ProbeGridLoader::LoadFile(ProbeGrid* probe_grid, const char* fn)
{
	FILE* fp = fopen(fn, "rb");
	fread(&probe_grid->coverage_min, sizeof(glm::vec3), 1, fp);
	fread(&probe_grid->coverage_max, sizeof(glm::vec3), 1, fp);
	fread(&probe_grid->divisions, sizeof(glm::ivec3), 1, fp);
	fread(&probe_grid->ypower, sizeof(float), 1, fp);
	fread(&probe_grid->vis_res, sizeof(int), 1, fp);
	fread(&probe_grid->pack_size, sizeof(int), 1, fp);
	fread(&probe_grid->pack_res, sizeof(int), 1, fp);

	size_t count = probe_grid->divisions.x * probe_grid->divisions.y * probe_grid->divisions.z;
	probe_grid->m_probe_data.resize(count * 9);
	fread(probe_grid->m_probe_data.data(), sizeof(glm::vec4), count * 9, fp);
	
	int pack_res = probe_grid->pack_res;
	probe_grid->m_visibility_data.resize(pack_res * pack_res);
	fread(probe_grid->m_visibility_data.data(), sizeof(unsigned short), probe_grid->m_visibility_data.size(), fp);

	probe_grid->allocate_probes();
	fclose(fp);
}

void ProbeGridLoader::LoadMemory(ProbeGrid* probe_grid, unsigned char* data, size_t size)
{
	unsigned char* ptr = data;
	probe_grid->coverage_min = *(glm::vec3*)ptr; ptr += sizeof(glm::vec3);
	probe_grid->coverage_max = *(glm::vec3*)ptr; ptr += sizeof(glm::vec3);
	probe_grid->divisions = *(glm::ivec3*)ptr; ptr += sizeof(glm::ivec3);
	probe_grid->ypower = *(float*)ptr; ptr += sizeof(float);
	probe_grid->vis_res = *(int*)ptr; ptr += sizeof(int);
	probe_grid->pack_size = *(int*)ptr; ptr += sizeof(int);
	probe_grid->pack_res = *(int*)ptr; ptr += sizeof(int);

	size_t count = probe_grid->divisions.x * probe_grid->divisions.y * probe_grid->divisions.z;
	probe_grid->m_probe_data.resize(count * 9);
	memcpy(probe_grid->m_probe_data.data(), ptr, sizeof(glm::vec4) * count * 9);
	ptr += sizeof(glm::vec4) * count * 9;
	
	int pack_res = probe_grid->pack_res;
	probe_grid->m_visibility_data.resize(pack_res * pack_res);
	memcpy(probe_grid->m_visibility_data.data(), ptr, sizeof(unsigned short) * probe_grid->m_visibility_data.size());
	
	probe_grid->allocate_probes();
}
