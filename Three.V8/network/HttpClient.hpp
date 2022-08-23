#pragma once

#include "WrapperUtils.hpp"
#include <network/HttpClient.h>


class WrapperHttpClient
{
public:
	static v8::Local<v8::ObjectTemplate> create_template(v8::Isolate* isolate);

private:
	static void Get(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetAsync(const v8::FunctionCallbackInfo<v8::Value>& info);
};



v8::Local<v8::ObjectTemplate> WrapperHttpClient::create_template(v8::Isolate* isolate)
{
	v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
	templ->SetInternalFieldCount(1);
	templ->Set(isolate, "get", v8::FunctionTemplate::New(isolate, Get));
	templ->Set(isolate, "getAsync", v8::FunctionTemplate::New(isolate, GetAsync));
	return templ;
}

void WrapperHttpClient::Get(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);	
	HttpClient* self = get_self<HttpClient>(info);	

	v8::String::Utf8Value url(isolate, info[0]);
	std::vector<unsigned char> data;
	self->Get(*url, data);

	bool is_string = false;
	if (info.Length() > 1)
	{
		is_string = info[1].As<v8::Boolean>()->Value();
	}

	if (is_string)
	{
		std::vector<char> str(data.size() + 1, 0);
		memcpy(str.data(), data.data(), data.size());
		v8::Local<v8::String> ret = v8::String::NewFromUtf8(isolate, str.data()).ToLocalChecked();
		info.GetReturnValue().Set(ret);
	}
	else
	{
		v8::Local<v8::ArrayBuffer> ret = v8::ArrayBuffer::New(isolate, data.size());
		void* p_data = ret->GetBackingStore()->Data();
		memcpy(p_data, data.data(), data.size());
		info.GetReturnValue().Set(ret);
	}

}

typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> CallbackT;

struct V8GetData
{
	GameContext* ctx;
	bool is_string;
	CallbackT callback;
};

static void HttpGetCallback(const GetResult& result, void* userData)
{
	V8GetData* v8data = (V8GetData*)(userData);
	GameContext* ctx = v8data->ctx;
	v8::Isolate* isolate = ctx->m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Context::Scope context_scope(ctx->m_context.Get(isolate));
	v8::Function* callback = *v8data->callback.Get(isolate);

	std::vector<v8::Local<v8::Value>> args(2);
	args[0] = v8::Boolean::New(isolate, result.result);

	if (result.result)
	{
		if (v8data->is_string)
		{
			std::vector<char> str(result.data.size() + 1, 0);
			memcpy(str.data(), result.data.data(), result.data.size());
			v8::Local<v8::String> ret = v8::String::NewFromUtf8(isolate, str.data()).ToLocalChecked();
			args[1] = ret;
		}
		else
		{
			v8::Local<v8::ArrayBuffer> ret = v8::ArrayBuffer::New(isolate, result.data.size());
			void* p_data = ret->GetBackingStore()->Data();
			memcpy(p_data, result.data.data(), result.data.size());
			args[1] = ret;
		}
	}
	else
	{
		args[1] = v8::Null(isolate);
	}
	ctx->InvokeCallback(callback, args);	

	delete v8data;
}

void WrapperHttpClient::GetAsync(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	HttpClient* self = get_self<HttpClient>(info);

	v8::String::Utf8Value url(isolate, info[0]);

	V8GetData* v8data = new V8GetData;
	v8data->ctx = get_context(info);
	v8data->is_string = info[1].As<v8::Boolean>()->Value();

	v8::Local<v8::Function> callback = info[2].As<v8::Function>();
	v8data->callback = CallbackT(isolate, callback);

	self->GetAsync(*url, HttpGetCallback, v8data);
}

