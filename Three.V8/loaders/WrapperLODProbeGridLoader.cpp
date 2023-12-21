#include "WrapperUtils.hpp"
#include <loaders/LODProbeGridLoader.h>
#include <utils/Utils.h>

#include "WrapperLODProbeGridLoader.h"

void WrapperLODProbeGridLoader::define(ObjectDefinition& object)
{
	object.name = "LODProbeGridLoader";
	object.methods = {
		{ "loadFile", LoadFile},
		{ "loadMemory", LoadMemory},
	};
}

void WrapperLODProbeGridLoader::LoadFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string filename = lctx.jstr_to_str(info[0]);
	if (!exists_test(filename.c_str()))
	{
		info.GetReturnValue().Set(v8::Null(lctx.isolate));
		return;
	}
	v8::Local<v8::Object> holder = lctx.instantiate("LODProbeGrid");
	LODProbeGrid* self = lctx.jobj_to_obj<LODProbeGrid>(holder);
	LODProbeGridLoader::LoadFile(self, filename.c_str());
	info.GetReturnValue().Set(holder);
}

void WrapperLODProbeGridLoader::LoadMemory(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Object> holder = lctx.instantiate("LODProbeGrid");
	LODProbeGrid* self = lctx.jobj_to_obj<LODProbeGrid>(holder);
	v8::Local<v8::ArrayBuffer> data = info[0].As<v8::ArrayBuffer>();
	LODProbeGridLoader::LoadMemory(self, (unsigned char*)data->GetBackingStore()->Data(), data->ByteLength());
	info.GetReturnValue().Set(holder);
}

