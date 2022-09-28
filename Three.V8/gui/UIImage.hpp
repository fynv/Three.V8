#pragma once

#include "WrapperUtils.hpp"
#include "UIElement.hpp"
#include <gui/UIImage.h>


class WrapperUIImage
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

private:
	static void GetSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetSize(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetSize(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void SetImage(const v8::FunctionCallbackInfo<v8::Value>& info);
};


v8::Local<v8::FunctionTemplate> WrapperUIImage::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperUIElement::create_template(isolate, constructor);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "size").ToLocalChecked(), GetSize, 0);
	templ->InstanceTemplate()->Set(isolate, "getSize", v8::FunctionTemplate::New(isolate, GetSize));
	templ->InstanceTemplate()->Set(isolate, "setSize", v8::FunctionTemplate::New(isolate, SetSize));
	templ->InstanceTemplate()->Set(isolate, "setImage", v8::FunctionTemplate::New(isolate, SetImage));
	return templ;
}

void WrapperUIImage::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIImage* self = new UIImage();
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), dtor);
}

void WrapperUIImage::dtor(void* ptr, GameContext* ctx)
{
	UIImage* self = (UIImage*)ptr;
	GamePlayer* player = ctx->GetGamePlayer();
	GLUIRenderer& renderer = player->UIRenderer();
	if (self->id_image != -1)
	{
		renderer.DeleteImage(self->id_image);
	}
	delete self;
}

void WrapperUIImage::GetSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIImage* self = lctx.self<UIImage>();
	v8::Local<v8::Object> size = v8::Object::New(lctx.isolate);
	lctx.vec2_to_jvec2(self->size, size);
	info.GetReturnValue().Set(size);
}


void WrapperUIImage::GetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIImage* self = lctx.self<UIImage>();
	lctx.vec2_to_jvec2(self->size, info[0]);
	info.GetReturnValue().Set(info[0]);
}


void WrapperUIImage::SetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIImage* self = lctx.self<UIImage>();
	glm::vec2 size;
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], size.x);
		lctx.jnum_to_num(info[1], size.y);
	}
	else
	{
		lctx.jvec2_to_vec2(info[0], size);
	}
	if (self->size != size)
	{
		self->size = size;
		self->appearance_changed = true;
	}
}

void WrapperUIImage::SetImage(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIImage* self = lctx.self<UIImage>();
	Image* image = lctx.jobj_to_obj<Image>(info[0]);
	GLUIRenderer& renderer = lctx.player()->UIRenderer();

	if (self->id_image != -1)
	{
		renderer.DeleteImage(self->id_image);
	}
	self->id_image = renderer.CreateImage(image);
	self->size.x = (float)image->width();
	self->size.y = (float)image->height();
	self->appearance_changed = true;

}