#include "WrapperUtils.hpp"
#include <savers/FileSaver.h>
#include <utils/Utils.h>

#include "WrapperFileSaver.h"

void WrapperFileSaver::define(ObjectDefinition& object)
{
	object.name = "fileSaver";
	object.methods = {
		{ "saveBinaryFile", SaveBinaryFile},
		{ "saveTextFile", SaveTextFile},
	};
}

void WrapperFileSaver::SaveBinaryFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string filename = lctx.jstr_to_str(info[0]);

	if (!writable_test(filename.c_str()))
	{
		info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, false));
		return;
	}

	v8::Local<v8::ArrayBuffer> data = info[1].As<v8::ArrayBuffer>();
	const unsigned char* p_data = (const unsigned char*)data->GetBackingStore()->Data();
	size_t size = data->ByteLength();
	::SaveBinaryFile(filename.c_str(), p_data, size);

	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, true));
}

void WrapperFileSaver::SaveTextFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string filename = lctx.jstr_to_str(info[0]);

	if (!writable_test(filename.c_str()))
	{
		info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, false));
		return;
	}

	std::string data = lctx.jstr_to_str(info[1]);
	::SaveTextFile(filename.c_str(), data.c_str());

	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, true));
}


