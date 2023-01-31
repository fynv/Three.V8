#pragma once

#include "WrapperUtils.hpp"
#include "GamePlayer.h"

class WrapperGamePlayer
{
public:
	static v8::Local<v8::ObjectTemplate> create_template(v8::Isolate* isolate);

private:
	static void GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	
	static void Message(const v8::FunctionCallbackInfo<v8::Value>& info);	

	static void HasFont(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void CreateFontFromFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void CreateFontFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetPicking(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetPicking(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void PickObject(const v8::FunctionCallbackInfo<v8::Value>& info);
};


v8::Local<v8::ObjectTemplate> WrapperGamePlayer::create_template(v8::Isolate* isolate)
{
	v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
	templ->SetInternalFieldCount(1);
	templ->SetAccessor(v8::String::NewFromUtf8(isolate, "width").ToLocalChecked(), GetWidth, 0);
	templ->SetAccessor(v8::String::NewFromUtf8(isolate, "height").ToLocalChecked(), GetHeight, 0);
	templ->Set(isolate, "message", v8::FunctionTemplate::New(isolate, Message));
	templ->Set(isolate, "hasFont", v8::FunctionTemplate::New(isolate, HasFont));
	templ->Set(isolate, "createFontFromFile", v8::FunctionTemplate::New(isolate, CreateFontFromFile));
	templ->Set(isolate, "createFontFromMemory", v8::FunctionTemplate::New(isolate, CreateFontFromMemory));
	templ->SetAccessor(v8::String::NewFromUtf8(isolate, "picking").ToLocalChecked(), GetPicking, SetPicking);
	templ->Set(isolate, "pickObject", v8::FunctionTemplate::New(isolate, PickObject));
	return templ;
}

void WrapperGamePlayer::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();	
	info.GetReturnValue().Set(lctx.num_to_jnum(self->width()));
}

void WrapperGamePlayer::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->height()));
}

void WrapperGamePlayer::Message(const v8::FunctionCallbackInfo<v8::Value>& info)
{	
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	std::string name = lctx.jstr_to_str(info[0]);
	std::string msg = lctx.jstr_to_str(info[1]);
	std::string res = self->UserMessage(name.c_str(), msg.c_str());
	info.GetReturnValue().Set(lctx.str_to_jstr(res.c_str()));
}

void WrapperGamePlayer::HasFont(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	std::string name = lctx.jstr_to_str(info[0]);
	bool has_font = self->UIRenderer().HasFont(name.c_str());
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, has_font));
}

void WrapperGamePlayer::CreateFontFromFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	std::string name = lctx.jstr_to_str(info[0]);
	std::string filename = lctx.jstr_to_str(info[1]);
	self->UIRenderer().CreateFont(name.c_str(), filename.c_str());
}

void WrapperGamePlayer::CreateFontFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	std::string name = lctx.jstr_to_str(info[0]);	
	v8::Local<v8::ArrayBuffer> data = info[1].As<v8::ArrayBuffer>();
	self->UIRenderer().CreateFont(name.c_str(), (unsigned char*)data->GetBackingStore()->Data(), data->ByteLength());
}

void WrapperGamePlayer::GetPicking(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	bool picking = self->Picking();
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, picking));
}

void WrapperGamePlayer::SetPicking(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	bool picking = value.As<v8::Boolean>()->Value();
	self->SetPicking(picking);
}

void WrapperGamePlayer::PickObject(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	GLPickingTarget* target = self->pickingTarget();

	int x, y;
	lctx.jnum_to_num(info[0], x);
	lctx.jnum_to_num(info[1], y);

	const GLPickingTarget::IdxInfo& idxInfo = target->pick_obj(x, y);
	if (idxInfo.obj != nullptr)
	{
		v8::Local<v8::Object> ret = v8::Object::New(lctx.isolate);
		v8::Local<v8::String> name = lctx.str_to_jstr(idxInfo.obj->name.c_str());
		v8::Local<v8::String> uuid = lctx.str_to_jstr(idxInfo.obj->uuid.c_str());
		lctx.set_property(ret, "name", name);
		lctx.set_property(ret, "uuid", uuid);
		info.GetReturnValue().Set(ret);
	}
	else
	{
		info.GetReturnValue().SetNull();
	}

}