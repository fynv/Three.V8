#pragma once

#include "WrapperUtils.hpp"
#include <models/HeightField.h>

class WrapperHeightField
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

private:
	static void SaveFile(const v8::FunctionCallbackInfo<v8::Value>& info);

};


v8::Local<v8::FunctionTemplate> WrapperHeightField::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);

	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));
	templ->InstanceTemplate()->Set(isolate, "saveFile", v8::FunctionTemplate::New(isolate, SaveFile));

	return templ;
}


void WrapperHeightField::dtor(void* ptr, GameContext* ctx)
{
	delete (HeightField*)ptr;
}

void WrapperHeightField::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);

	glm::vec3 min_pos, max_pos;
	lctx.jvec3_to_vec3(info[0], min_pos);
	lctx.jvec3_to_vec3(info[1], max_pos);
	int width, height;
	lctx.jnum_to_num(info[2], width);
	lctx.jnum_to_num(info[3], height);

	HeightField* self = new HeightField(min_pos, max_pos, width, height);
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), dtor);
}

void WrapperHeightField::SaveFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HeightField* self = lctx.self<HeightField>();
	std::string fn = lctx.jstr_to_str(info[0]);
	self->saveFile(fn.c_str());
}


