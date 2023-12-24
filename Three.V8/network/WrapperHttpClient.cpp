#include "WrapperUtils.hpp"
#include <network/HttpClient.h>

#include "WrapperHttpClient.h"


void WrapperHttpClient::define(ObjectDefinition& object)
{
	object.name = "http";
	object.ctor = ctor;
	object.dtor = dtor;
	object.methods = {
		{ "get", Get},
		{ "getAsync", GetAsync},
	};
}


void* WrapperHttpClient::ctor()
{
	return new HttpClient;
}

void WrapperHttpClient::dtor(void* ptr, GameContext* ctx)
{	
	delete (HttpClient*)ptr;
}

void WrapperHttpClient::Get(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HttpClient* self = lctx.self<HttpClient>();

	std::string url = lctx.jstr_to_str(info[0]);
	std::vector<unsigned char> data;
	self->Get(url.c_str(), data);

	bool is_string = false;
	if (info.Length() > 1)
	{
		is_string = info[1].As<v8::Boolean>()->Value();
	}

	if (is_string)
	{
		std::vector<char> str(data.size() + 1, 0);
		memcpy(str.data(), data.data(), data.size());
		info.GetReturnValue().Set(lctx.str_to_jstr(str.data()));
	}
	else
	{
		v8::Local<v8::ArrayBuffer> ret = v8::ArrayBuffer::New(lctx.isolate, data.size());
		void* p_data = ret->GetBackingStore()->Data();
		memcpy(p_data, data.data(), data.size());
		info.GetReturnValue().Set(ret);
	}

}

typedef v8::Persistent<v8::Promise::Resolver, v8::CopyablePersistentTraits<v8::Promise::Resolver>> ResolverT;

struct V8GetData
{
	GameContext* ctx;
	bool is_string;
	ResolverT resolver;
};

static void HttpGetCallback(const GetResult& result, void* userData)
{
	V8GetData* v8data = (V8GetData*)(userData);
	GameContext* ctx = v8data->ctx;

	v8::Isolate* isolate = ctx->m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Context::Scope context_scope(ctx->m_context.Get(isolate));
	LocalContext lctx(isolate);

	v8::Promise::Resolver* resolver = *v8data->resolver.Get(isolate);

	if (result.result)
	{
		if (v8data->is_string)
		{
			std::vector<char> str(result.data.size() + 1, 0);
			memcpy(str.data(), result.data.data(), result.data.size());
			resolver->Resolve(lctx.context, lctx.str_to_jstr(str.data()));
		}
		else
		{
			v8::Local<v8::ArrayBuffer> ret = v8::ArrayBuffer::New(isolate, result.data.size());
			void* p_data = ret->GetBackingStore()->Data();
			memcpy(p_data, result.data.data(), result.data.size());
			resolver->Resolve(lctx.context, ret);			
		}
	}
	else
	{
		resolver->Reject(lctx.context, v8::Null(isolate));
	}

	delete v8data;
}

void WrapperHttpClient::GetAsync(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	HttpClient* self = lctx.self<HttpClient>();

	std::string url = lctx.jstr_to_str(info[0]);
	v8::Local<v8::Promise::Resolver> resolver = v8::Promise::Resolver::New(lctx.context).ToLocalChecked();

	V8GetData* v8data = new V8GetData;
	v8data->ctx = lctx.ctx();
	v8data->is_string = info[1].As<v8::Boolean>()->Value();	
	v8data->resolver = ResolverT(lctx.isolate, resolver);

	self->GetAsync(url.c_str(), HttpGetCallback, v8data);

	info.GetReturnValue().Set(resolver->GetPromise());
}

