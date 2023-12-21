#include "WrapperUtils.hpp"
#include <loaders/ProbeGridLoader.h>
#include <utils/Utils.h>

#include "WrapperProbeGridLoader.h"

void WrapperProbeGridLoader::define(ObjectDefinition& object)
{
	object.name = "probeGridLoader";
	object.methods = {
		{ "loadFile", LoadFile},
		{ "loadMemory", LoadMemory},
	};
}

void WrapperProbeGridLoader::LoadFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string filename = lctx.jstr_to_str(info[0]);
	if (!exists_test(filename.c_str()))
	{
		info.GetReturnValue().Set(v8::Null(lctx.isolate));
		return;
	}
	v8::Local<v8::Object> holder = lctx.instantiate("ProbeGrid");
	ProbeGrid* self = lctx.jobj_to_obj<ProbeGrid>(holder);
	ProbeGridLoader::LoadFile(self, filename.c_str());
	info.GetReturnValue().Set(holder);
}

void WrapperProbeGridLoader::LoadMemory(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Object> holder = lctx.instantiate("ProbeGrid");
	ProbeGrid* self = lctx.jobj_to_obj<ProbeGrid>(holder);
	v8::Local<v8::ArrayBuffer> data = info[0].As<v8::ArrayBuffer>();
	ProbeGridLoader::LoadMemory(self, (unsigned char*)data->GetBackingStore()->Data(), data->ByteLength());
	info.GetReturnValue().Set(holder);
}
