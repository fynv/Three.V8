#include "WrapperUtils.hpp"
#include <loaders/GLTFLoader.h>
#include <utils/Utils.h>

#include "WrapperGLTFLoader.h"


void WrapperGLTFLoader::define(ObjectDefinition& object)
{
	object.name = "gltfLoader";
	object.methods = {
		{ "loadModelFromFile", LoadModelFromFile},
		{ "loadAnimationsFromFile", LoadAnimationsFromFile},
		{ "loadModelFromMemory", LoadModelFromMemory},
		{ "loadAnimationsFromMemory", LoadAnimationsFromMemory},
	};
}

void WrapperGLTFLoader::LoadModelFromFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string filename = lctx.jstr_to_str(info[0]);
	if (!exists_test(filename.c_str()))
	{
		info.GetReturnValue().Set(v8::Null(lctx.isolate));
		return;
	}
	v8::Local<v8::Object> holder = lctx.instantiate("GLTFModel");
	GLTFModel* self = lctx.jobj_to_obj<GLTFModel>(holder);
	GLTFLoader::LoadModelFromFile(self, filename.c_str());
	info.GetReturnValue().Set(holder);
}

void WrapperGLTFLoader::LoadAnimationsFromFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string filename = lctx.jstr_to_str(info[0]);

	if (!exists_test(filename.c_str()))
	{
		info.GetReturnValue().Set(v8::Null(lctx.isolate));
		return;
	}

	std::vector<AnimationClip> animations;
	GLTFLoader::LoadAnimationsFromFile(animations, filename.c_str());

	v8::Local<v8::Array> janims = v8::Array::New(lctx.isolate, (int)animations.size());
	for (size_t i = 0; i < animations.size(); i++)
	{
		AnimationClip& anim = animations[i];
		v8::Local<v8::Object> janim = v8::Object::New(lctx.isolate);
		lctx.anim_to_janim(anim, janim);
		janims->Set(lctx.context, (unsigned)i, janim);
	}

	info.GetReturnValue().Set(janims);
}


void WrapperGLTFLoader::LoadModelFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Object> holder = lctx.instantiate("GLTFModel");
	GLTFModel* self = lctx.jobj_to_obj<GLTFModel>(holder);

	v8::Local<v8::ArrayBuffer> data = info[0].As<v8::ArrayBuffer>();
	GLTFLoader::LoadModelFromMemory(self, (unsigned char*)data->GetBackingStore()->Data(), data->ByteLength());
	info.GetReturnValue().Set(holder);
}


void WrapperGLTFLoader::LoadAnimationsFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);

	std::vector<AnimationClip> animations;

	v8::Local<v8::ArrayBuffer> data = info[0].As<v8::ArrayBuffer>();
	GLTFLoader::LoadAnimationsFromMemory(animations, (unsigned char*)data->GetBackingStore()->Data(), data->ByteLength());

	v8::Local<v8::Array> janims = v8::Array::New(lctx.isolate, (int)animations.size());

	for (size_t i = 0; i < animations.size(); i++)
	{
		AnimationClip& anim = animations[i];
		v8::Local<v8::Object> janim = v8::Object::New(lctx.isolate);
		lctx.anim_to_janim(anim, janim);
		janims->Set(lctx.context, (unsigned)i, janim);
	}

	info.GetReturnValue().Set(janims);
}

