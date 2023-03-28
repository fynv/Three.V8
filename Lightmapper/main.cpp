#include <cstdio>
#include <string>
#include "DataModel.h"

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Lightmapper input.glb [output.glb] [texels_per_unit]\n");
		return 0;
	}

	const char* fn_input = argv[1];
	std::string fn_output = "output.glb";
	if (argc > 2)
	{
		fn_output = argv[2];
	}

	int texels_per_unit = 128;
	if (argc > 3)
	{
		texels_per_unit = atoi(argv[3]);
	}

	DataModel model;
	model.LoadGlb(fn_input);
	model.CreateAtlas(texels_per_unit);
	model.SaveGlb(fn_output.c_str());

	printf("lightmap width: %d\n", model.lightmap_width);
	printf("lightmap height: %d\n", model.lightmap_height);
	printf("lightmap texels_per_unit: %d\n", model.lightmap_texels_per_unit);

	return 0;
}
