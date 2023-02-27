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

	probe_grid->m_probe_data.resize(num_probes * 10);
	fread(probe_grid->m_probe_data.data(), sizeof(glm::vec4), num_probes * 10, fp);

	probe_grid->m_sub_index.resize(num_indices);
	fread(probe_grid->m_sub_index.data(), sizeof(int), num_indices, fp);

	size_t pos = (size_t)ftell(fp);
	if (size - pos >= sizeof(float) * 26 * num_probes)
	{
		probe_grid->m_visibility_data.resize(num_probes * 26);
		fread(probe_grid->m_visibility_data.data(), sizeof(float), num_probes * 26, fp);
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

	probe_grid->m_probe_data.resize(num_probes * 10);	
	memcpy(probe_grid->m_probe_data.data(), ptr, sizeof(glm::vec4) * num_probes * 10);
	ptr += sizeof(glm::vec4) * num_probes * 10;

	probe_grid->m_sub_index.resize(num_indices);
	memcpy(probe_grid->m_sub_index.data(), ptr, sizeof(int) * num_indices);
	ptr += sizeof(int) * num_indices;

	size_t pos = ptr - data;
	if (size - pos >= sizeof(float) * 26 * num_probes)
	{
		probe_grid->m_visibility_data.resize(num_probes * 26);
		memcpy(probe_grid->m_visibility_data.data(), ptr, sizeof(float) * 26 * num_probes);
	}

	probe_grid->updateBuffers();
}
