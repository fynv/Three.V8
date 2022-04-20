#pragma once

#include "WrapperUtils.hpp"
#include <loaders/GLTFLoader.h>

class WrapperGLTFLoader
{
public:
	static v8::Local<v8::ObjectTemplate> create_template(v8::Isolate* isolate);

private:
	static void LoadModelFromFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadAnimationsFromFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadModelFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LoadAnimationsFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info);
};


v8::Local<v8::ObjectTemplate> WrapperGLTFLoader::create_template(v8::Isolate* isolate)
{
	v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
	templ->Set(isolate, "loadModelFromFile", v8::FunctionTemplate::New(isolate, LoadModelFromFile));
	templ->Set(isolate, "loadAnimationsFromFile", v8::FunctionTemplate::New(isolate, LoadAnimationsFromFile));
	templ->Set(isolate, "loadModelFromMemory", v8::FunctionTemplate::New(isolate, LoadModelFromMemory));
	templ->Set(isolate, "loadAnimationsFromMemory", v8::FunctionTemplate::New(isolate, LoadAnimationsFromMemory));
	return templ;
}

void WrapperGLTFLoader::LoadModelFromFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> global = context->Global();
	v8::Local<v8::Function> ctor_model = global->Get(context, v8::String::NewFromUtf8(isolate, "GLTFModel").ToLocalChecked()).ToLocalChecked().As<v8::Function>();

	v8::Local<v8::Object> holder = ctor_model->CallAsConstructor(context, 0, nullptr).ToLocalChecked().As<v8::Object>();
	v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(holder->GetInternalField(0));
	GLTFModel* self = (GLTFModel*)wrap->Value();

	v8::String::Utf8Value filename(isolate, info[0]);
	GLTFLoader::LoadModelFromFile(self, *filename);
	info.GetReturnValue().Set(holder);
}

void WrapperGLTFLoader::LoadAnimationsFromFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	v8::String::Utf8Value filename(isolate, info[0]);
	std::vector<AnimationClip> animations;
	GLTFLoader::LoadAnimationsFromFile(animations, *filename);

	v8::Local<v8::Array> janims = v8::Array::New(isolate, (int)animations.size());

	for (size_t i = 0; i < animations.size(); i++)
	{
		AnimationClip& anim = animations[i];
		v8::Local<v8::Object> janim = v8::Object::New(isolate);
		anim_to_janim(isolate, anim, janim);
		janims->Set(context, (unsigned)i, janim);
	}

	info.GetReturnValue().Set(janims);
}


void WrapperGLTFLoader::LoadModelFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> global = context->Global();
	v8::Local<v8::Function> ctor_model = global->Get(context, v8::String::NewFromUtf8(isolate, "GLTFModel").ToLocalChecked()).ToLocalChecked().As<v8::Function>();

	v8::Local<v8::Object> holder = ctor_model->CallAsConstructor(context, 0, nullptr).ToLocalChecked().As<v8::Object>();
	v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(holder->GetInternalField(0));
	GLTFModel* self = (GLTFModel*)wrap->Value();

	v8::Local<v8::ArrayBuffer> data = info[0].As<v8::ArrayBuffer>();
	GLTFLoader::LoadModelFromMemory(self, (unsigned char*)data->GetBackingStore()->Data(), data->ByteLength());
	info.GetReturnValue().Set(holder);
}



void WrapperGLTFLoader::LoadAnimationsFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();	
	std::vector<AnimationClip> animations;
	
	v8::Local<v8::ArrayBuffer> data = info[0].As<v8::ArrayBuffer>();
	GLTFLoader::LoadAnimationsFromMemory(animations, (unsigned char*)data->GetBackingStore()->Data(), data->ByteLength());

	v8::Local<v8::Array> janims = v8::Array::New(isolate, (int)animations.size());

	for (size_t i = 0; i < animations.size(); i++)
	{
		AnimationClip& anim = animations[i];
		v8::Local<v8::Object> janim = v8::Object::New(isolate);
		anim_to_janim(isolate, anim, janim);
		janims->Set(context, (unsigned)i, janim);
	}

	info.GetReturnValue().Set(janims);
}

