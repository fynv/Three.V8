#include "WrapperUtils.hpp"
#include <loaders/FileLoader.h>
#include <utils/Utils.h>

#include "WrapperFileLoader.h"

void WrapperFileLoader::define(ObjectDefinition& object)
{
	object.name = "fileLoader";
	object.methods = {
		{ "loadBinaryFile", LoadBinaryFile},
		{ "loadTextFile", LoadTextFile},
	};
}

void WrapperFileLoader::LoadBinaryFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string filename = lctx.jstr_to_str(info[0]);

	if (!exists_test(filename.c_str()))
	{
		info.GetReturnValue().Set(v8::Null(lctx.isolate));
		return;
	}

	std::vector<unsigned char> data;
	::LoadBinaryFile(filename.c_str(), data);

	v8::Local<v8::ArrayBuffer> ret = v8::ArrayBuffer::New(lctx.isolate, data.size());
	void* p_data = ret->GetBackingStore()->Data();
	memcpy(p_data, data.data(), data.size());

	info.GetReturnValue().Set(ret);
}

void WrapperFileLoader::LoadTextFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string filename = lctx.jstr_to_str(info[0]);

	if (!exists_test(filename.c_str()))
	{
		info.GetReturnValue().Set(v8::Null(lctx.isolate));
		return;
	}

	std::vector<char> data;
	::LoadTextFile(filename.c_str(), data);

	v8::Local<v8::String> ret = lctx.str_to_jstr(data.data());
	info.GetReturnValue().Set(ret);
}
