#include <vector>
#include <string>
#include <cstdio>
#include <webp/encode.h>

#include "stb_image.h"
#include "stb_image_write.h"

#include "glm.hpp"

struct Range
{
	glm::vec3 low;
	glm::vec3 high;
};

void get_range(int count, const glm::vec3* hdr, Range& range)
{
	glm::vec3 min_v = glm::vec3(FLT_MAX);
	glm::vec3 max_v = glm::vec3(-FLT_MAX);
	for (int i = 0; i < count; i++)
	{
		glm::vec3 v = hdr[i];
		min_v = glm::min(min_v, v);
		max_v = glm::max(max_v, v);
	}

	glm::vec3 center = (max_v + min_v) * 0.5f;
	glm::vec3 delta = max_v - min_v;
	glm::vec3 scale = glm::max(glm::vec3(1.0f), 0.25f / delta);
	min_v = (min_v - center) * scale + center;
	max_v = (max_v - center) * scale + center;

	range.low = min_v;
	range.high = max_v;
}


void quantize(int count, const glm::vec3* hdr, const Range& range, glm::u8vec3* ldr)
{
	for (int i = 0; i < count; i++)
	{
		glm::vec3 v_in = hdr[i];
		glm::u8vec3 v_out = glm::u8vec3(glm::clamp((v_in - range.low) / (range.high - range.low), 0.0f, 1.0f) * 255.0f + 0.5f);
		ldr[i] = v_out;
	}
}

void subtract(int count, glm::vec3* hdr, const Range& range, const glm::u8vec3* ldr)
{
	for (int i = 0; i < count; i++)
	{
		glm::vec3 v_hdr = hdr[i];
		glm::u8vec3 s_ldr = ldr[i];
		v_hdr -= glm::vec3(s_ldr) / 255.0f * (range.high - range.low) + range.low;
		hdr[i] = v_hdr;
	}
}


glm::u8vec4 RGBMEncode(glm::vec3 rgb, float rate)
{
	glm::vec3 c_rgb = rgb / rate;
	float a = std::min(std::max(std::max(c_rgb.x, c_rgb.y), std::max(c_rgb.z, 1e-6f)), 1.0f);
	a = ceilf(a * 255.0f) / 255.0f;
	return glm::u8vec4(glm::vec4(c_rgb / a, a) * 255.0f + 0.5f);
}


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("LightmapCompressor input.hdr [output_prefix] [jpeg\png\webp]\n");
		return 0;
	}

	std::string output_prefix = "level";

	if (argc > 2)
	{
		output_prefix = argv[2];
	}

	std::string mode = "jpeg";
	if (argc > 3)
	{
		mode = argv[3];
	}

	std::vector<std::string> filenames;
	std::vector<Range> ranges;	

	int width;
	int height;
	int chn;
	glm::vec3* hdr = (glm::vec3*)stbi_loadf(argv[1], &width, &height, &chn, 3);

	int count = width * height;

	if (mode == "jpeg")
	{

		int max_level = 5;

		Range range0;
		get_range(count, hdr, range0);
		ranges.push_back(range0);

		std::vector<glm::u8vec3> image_ldr(count);
		quantize(count, hdr, range0, image_ldr.data());

		char fn_out[64];
		sprintf(fn_out, "%s%d.jpg", output_prefix.c_str(), 0);
		filenames.push_back(fn_out);
		stbi_write_jpg(fn_out, width, height, 3, image_ldr.data(), 80);

		for (int i = 1; i <= max_level; i++)
		{
			{
				glm::u8vec3* dec = (glm::u8vec3*)stbi_load(filenames[i - 1].c_str(), &width, &height, &chn, 3);
				memcpy(image_ldr.data(), dec, count * 3);
				stbi_image_free(dec);
			}

			subtract(count, hdr, ranges[i - 1], image_ldr.data());

			Range range1;
			get_range(count, hdr, range1);
			ranges.push_back(range1);

			quantize(count, hdr, range1, image_ldr.data());
			sprintf(fn_out, "%s%d.jpg", output_prefix.c_str(), i);
			filenames.push_back(fn_out);
			stbi_write_jpg(fn_out, width, height, 3, image_ldr.data(), 80);
		}

		sprintf(fn_out, "%s.csv", output_prefix.c_str());
		FILE* fp = fopen(fn_out, "w");
		for (int i = 0; i <= max_level; i++)
		{
			std::string fn = filenames[i];
			Range range = ranges[i];
			fprintf(fp, fn.c_str());
			fprintf(fp, ", %f, %f, %f", range.low.x, range.low.y, range.low.z);
			fprintf(fp, ", %f, %f, %f\n", range.high.x, range.high.y, range.high.z);
		}

		fclose(fp);
	}
	else
	{
		std::vector<glm::u8vec4> rgbm(width * height);
		for (int i = 0; i < width * height; i++)
		{
			rgbm[i] = RGBMEncode(hdr[i], 16.0f);
		}

		if (mode == "png")
		{
			char fn_out[64];
			sprintf(fn_out, "%s.png", output_prefix.c_str());
			stbi_write_png(fn_out, width, height, 4, rgbm.data(), width * 4);
		}
		else if (mode == "webp")
		{
			uint8_t* data;
			size_t size = WebPEncodeRGBA((uint8_t*)rgbm.data(), width, height, width * 4, 80.0f, &data);

			char fn_out[64];
			sprintf(fn_out, "%s.webp", output_prefix.c_str());
			FILE* fp = fopen(fn_out, "wb");
			fwrite(data, 1, size, fp);
			fclose(fp);

			WebPFree(data);
		}
	}

	stbi_image_free(hdr);

	return 0;
}

