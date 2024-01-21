#include "WrapperUtils.hpp"
#include <lights/EnvironmentMapCreator.h>
#include <utils/Image.h>
#include <utils/HDRImage.h>
#include "WrapperEnvironmentMapCreator.h"


void WrapperEnvironmentMapCreator::define(ClassDefinition& cls)
{
	cls.name = "EnvironmentMapCreator";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.methods = {
		{"create", Create },	
	};
}

void* WrapperEnvironmentMapCreator::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new EnvironmentMapCreator;
}

void WrapperEnvironmentMapCreator::dtor(void* ptr, GameContext* ctx)
{
	delete (EnvironmentMapCreator*)ptr;
}

void WrapperEnvironmentMapCreator::Create(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	EnvironmentMapCreator* creator = lctx.self<EnvironmentMapCreator>();

	v8::Local<v8::Object> holder = lctx.instantiate("EnvironmentMap");
	EnvironmentMap* self = lctx.jobj_to_obj<EnvironmentMap>(holder);

	v8::Local<v8::Object> holder_image = info[0].As<v8::Object>();

	bool irradiance_only = false;
	if (info.Length() > 1)
	{
		irradiance_only = info[1].As<v8::Boolean>()->Value();
	}

	std::string clsname = lctx.jstr_to_str(holder_image->GetConstructorName());
	if (clsname == "CubeImage")
	{
		CubeImage* image = lctx.jobj_to_obj<CubeImage>(holder_image);
		creator->Create(image, self);
	}
	else if (clsname == "HDRCubeImage")
	{
		HDRCubeImage* image = lctx.jobj_to_obj<HDRCubeImage>(holder_image);
		creator->Create(image, self);
	}
	else if (clsname == "CubeBackground")
	{
		CubeBackground* background = lctx.jobj_to_obj<CubeBackground>(holder_image);
		creator->Create(background, self);
	}
	else  if (clsname == "CubeRenderTarget")
	{
		CubeRenderTarget* target = lctx.jobj_to_obj<CubeRenderTarget>(holder_image);
		creator->Create(target, self, irradiance_only);
	}
	else if (clsname == "Image")
	{
		Image* image = lctx.jobj_to_obj<Image>(holder_image);
		creator->Create(image, self);
	}
	else if (clsname == "HDRImage")
	{
		HDRImage* image = lctx.jobj_to_obj<HDRImage>(holder_image);
		creator->Create(image, self);
	}
	else if (clsname == "PanoramaBackground")
	{
		PanoramaBackground* background = lctx.jobj_to_obj<PanoramaBackground>(holder_image);
		creator->Create(background, self);
	}

	info.GetReturnValue().Set(holder);
}
