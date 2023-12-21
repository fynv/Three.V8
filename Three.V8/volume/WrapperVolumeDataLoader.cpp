#include "WrapperUtils.hpp"
#include <volume/VolumeDataLoader.h>

#include "WrapperVolumeDataLoader.h"

void WrapperVolumeDataLoader::define(ObjectDefinition& object)
{
	object.name = "volumeDataLoader";
	object.methods = {
		{ "loadRawVolumeFile", LoadRawVolumeFile},		
	};
}

void WrapperVolumeDataLoader::LoadRawVolumeFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Object> holder = lctx.instantiate("VolumeData");
	VolumeData* self = lctx.jobj_to_obj<VolumeData>(holder);
	std::string filename = lctx.jstr_to_str(info[0]);
	glm::ivec3 size;
	lctx.jnum_to_num(info[1], size.x);
	lctx.jnum_to_num(info[2], size.y);
	lctx.jnum_to_num(info[3], size.z);
	glm::vec3 spacing;
	lctx.jvec3_to_vec3(info[4], spacing);
	int byte_per_pixel = 1;
	if (info.Length() > 5)
	{
		lctx.jnum_to_num(info[5], byte_per_pixel);
	}
	VolumeDataLoader::LoadRawVolumeFile(self, filename.c_str(), size, spacing, byte_per_pixel);
	info.GetReturnValue().Set(holder);
}


