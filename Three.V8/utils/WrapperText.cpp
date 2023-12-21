#include "WrapperUtils.hpp"
#include "WrapperText.h"


void WrapperText::define(ObjectDefinition& object)
{
	object.name = "text";
	object.methods = {
		{ "toUTF8Buffer", ToUTF8Buffer},
		{ "fromUTF8Buffer", FromUTF8Buffer},
	};
}

void WrapperText::ToUTF8Buffer(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	std::string text = lctx.jstr_to_str(info[0]);

	v8::Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(lctx.isolate, text.length());
	memcpy(buf->GetBackingStore()->Data(), text.data(), text.length());

	info.GetReturnValue().Set(buf);

}

void WrapperText::FromUTF8Buffer(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::ArrayBuffer> buf = info[0].As<v8::ArrayBuffer>();

	std::vector<char> str(buf->ByteLength() + 1, 0);
	memcpy(str.data(), buf->GetBackingStore()->Data(), buf->ByteLength());

	v8::Local<v8::String> ret = lctx.str_to_jstr(str.data());

	info.GetReturnValue().Set(ret);
}


