#include "WrapperUtils.hpp"
#include <models/HeightField.h>
#include "WrapperHeightField.h"

void WrapperHeightField::define(ClassDefinition& cls)
{
	cls.name = "HeightField";
	cls.ctor = ctor;
	cls.dtor = dtor;	
	cls.methods = {
		{"saveFile", SaveFile }	
	};
}

void* WrapperHeightField::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);

	glm::vec3 min_pos, max_pos;
	lctx.jvec3_to_vec3(info[0], min_pos);
	lctx.jvec3_to_vec3(info[1], max_pos);
	int width, height;
	lctx.jnum_to_num(info[2], width);
	lctx.jnum_to_num(info[3], height);

	return new HeightField(min_pos, max_pos, width, height);
}

void WrapperHeightField::dtor(void* ptr, GameContext* ctx)
{
	delete (HeightField*)ptr;
}

void WrapperHeightField::SaveFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HeightField* self = lctx.self<HeightField>();
	std::string fn = lctx.jstr_to_str(info[0]);
	self->saveFile(fn.c_str());
}

