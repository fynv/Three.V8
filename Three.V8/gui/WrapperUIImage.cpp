#include "WrapperUtils.hpp"
#include "WrapperUIElement.h"
#include <gui/UIImage.h>
#include <utils/Image.h>

#include "WrapperUIImage.h"

void WrapperUIImage::define(ClassDefinition& cls)
{
	WrapperUIElement::define(cls);
	cls.name = "UIImage";
	cls.ctor = ctor;
	cls.dtor = dtor;

	std::vector<AccessorDefinition> props = {
		{ "size", GetSize },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"getSize", GetSize },
		{"setSize", SetSize },
		{"setImage", SetImage },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());
}

void* WrapperUIImage::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new UIImage;
}

void WrapperUIImage::dtor(void* ptr, GameContext* ctx)
{
	v8::Isolate* isolate = ctx->m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Context::Scope context_scope(ctx->m_context.Get(isolate));
	LocalContext lctx(isolate);

	UIImage* self = (UIImage*)ptr;
	GamePlayer* player = lctx.player();
	GLUIRenderer& renderer = player->UIRenderer();
	if (self->id_image != -1)
	{
		renderer.DeleteImage(self->id_image);
	}
	WrapperUIElement::dtor(ptr, ctx);
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

