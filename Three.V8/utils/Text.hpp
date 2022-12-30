#pragma once

#include "WrapperUtils.hpp"

class WrapperText
{
public:
	static v8::Local<v8::ObjectTemplate> create_template(v8::Isolate* isolate);

private:
	static void ToUTF8Buffer(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void FromUTF8Buffer(const v8::FunctionCallbackInfo<v8::Value>& info);

};

v8::Local<v8::ObjectTemplate> WrapperText::create_template(v8::Isolate* isolate)
{
	v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
	templ->Set(isolate, "toUTF8Buffer", v8::FunctionTemplate::New(isolate, ToUTF8Buffer));
	templ->Set(isolate, "fromUTF8Buffer", v8::FunctionTemplate::New(isolate, FromUTF8Buffer));
	return templ;
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


