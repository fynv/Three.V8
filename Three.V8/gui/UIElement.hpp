#pragma once

#include "WrapperUtils.hpp"
#include <gui/UIElement.h>

class WrapperUIElement
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor);
	static void dtor(void* ptr, GameContext* ctx);

private:
	static void GetBlock(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetBlock(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetOrigin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetOnPointerDown(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetOnPointerDown(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetOnPointerUp(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetOnPointerUp(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetOnPointerMove(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetOnPointerMove(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

};


v8::Local<v8::FunctionTemplate> WrapperUIElement::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "block").ToLocalChecked(), GetBlock, SetBlock);

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "origin").ToLocalChecked(), GetOrigin, 0);
	templ->InstanceTemplate()->Set(isolate, "getOrigin", v8::FunctionTemplate::New(isolate, GetOrigin));
	templ->InstanceTemplate()->Set(isolate, "setOrigin", v8::FunctionTemplate::New(isolate, SetOrigin));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "onPointerDown").ToLocalChecked(), GetOnPointerDown, SetOnPointerDown);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "onPointerUp").ToLocalChecked(), GetOnPointerUp, SetOnPointerUp);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "onPointerMove").ToLocalChecked(), GetOnPointerMove, SetOnPointerMove);

	return templ;
}


struct UIElementCallbackData
{
	GameContext* ctx;
	CallbackT callback;
};

void WrapperUIElement::dtor(void* ptr, GameContext* ctx)
{
	UIElement* self = (UIElement*)ptr;
	if (self->pointer_down_callback != nullptr)
	{
		UIElementCallbackData* data = (UIElementCallbackData*)self->pointer_down_callback_data;
		delete data;
	}

	if (self->pointer_up_callback != nullptr)
	{
		UIElementCallbackData* data = (UIElementCallbackData*)self->pointer_up_callback_data;
		delete data;
	}

	if (self->pointer_move_callback != nullptr)
	{
		UIElementCallbackData* data = (UIElementCallbackData*)self->pointer_move_callback_data;
		delete data;
	}

	delete (UIElement*)ptr;
}


void WrapperUIElement::GetBlock(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> block = lctx.get_property(lctx.holder, "_block");
	info.GetReturnValue().Set(block);
}

void WrapperUIElement::SetBlock(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	UIElement* self = lctx.self<UIElement>();
	UIBlock* block = lctx.jobj_to_obj<UIBlock>(value);
	self->block = block;
	lctx.set_property(lctx.holder, "_block", value);	
}

void WrapperUIElement::GetOrigin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIElement* self = lctx.self<UIElement>();
	v8::Local<v8::Object> origin = v8::Object::New(lctx.isolate);
	lctx.vec2_to_jvec2(self->origin, origin);
	info.GetReturnValue().Set(origin);
}


void WrapperUIElement::GetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIElement* self = lctx.self<UIElement>();
	lctx.vec2_to_jvec2(self->origin, info[0]);
	info.GetReturnValue().Set(info[0]);
}


void WrapperUIElement::SetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIElement* self = lctx.self<UIElement>();
	glm::vec2 origin;
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], origin.x);
		lctx.jnum_to_num(info[1], origin.y);
	}
	else
	{
		lctx.jvec2_to_vec2(info[0], origin);
	}

	if (self->origin != origin)
	{
		self->origin = origin;
		self->appearance_changed = true;
	}
}


static void UIElementPointerCallback(float x, float y, void* ptr)
{
	UIElementCallbackData* data = (UIElementCallbackData*)ptr;
	GameContext* ctx = data->ctx;
	v8::Isolate* isolate = ctx->m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = ctx->m_context.Get(isolate);
	v8::Context::Scope context_scope(context);
	v8::Local<v8::Function> callback = data->callback.Get(isolate);
	v8::Local<v8::Object> global = context->Global();

	std::vector<v8::Local<v8::Value>> args(2);
	args[0] = v8::Number::New(isolate, (double)x);
	args[1] = v8::Number::New(isolate, (double)y);	
	callback->Call(context, global, 2, args.data());
}

void WrapperUIElement::GetOnPointerDown(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> callback = lctx.get_property(lctx.holder, "_onPointerDown");
	info.GetReturnValue().Set(callback);
}

void WrapperUIElement::SetOnPointerDown(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onPointerDown", value);

	UIElement* self = lctx.self<UIElement>();

	if (self->pointer_down_callback_data != nullptr)
	{
		UIElementCallbackData* data = (UIElementCallbackData*)self->pointer_down_callback_data;
		delete data;
	}

	if (!value->IsNullOrUndefined())
	{
		self->pointer_down_callback = UIElementPointerCallback;
		UIElementCallbackData* data = new UIElementCallbackData;
		data->ctx = lctx.ctx();

		v8::Local<v8::Function> callback = value.As<v8::Function>();
		data->callback = CallbackT(lctx.isolate, callback);

		self->pointer_down_callback_data = data;
	}
	else
	{
		self->pointer_down_callback = nullptr;
		self->pointer_down_callback_data = nullptr;
	}
}

void WrapperUIElement::GetOnPointerUp(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> callback = lctx.get_property(lctx.holder, "_onPointerUp");
	info.GetReturnValue().Set(callback);
}

void WrapperUIElement::SetOnPointerUp(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onPointerUp", value);

	UIElement* self = lctx.self<UIElement>();
	if (self->pointer_up_callback_data != nullptr)
	{
		UIElementCallbackData* data = (UIElementCallbackData*)self->pointer_up_callback_data;
		delete data;
	}

	if (!value->IsNullOrUndefined())
	{
		self->pointer_up_callback = UIElementPointerCallback;

		UIElementCallbackData* data = new UIElementCallbackData;
		data->ctx = lctx.ctx();

		v8::Local<v8::Function> callback = value.As<v8::Function>();
		data->callback = CallbackT(lctx.isolate, callback);

		self->pointer_up_callback_data = data;
	}
	else
	{
		self->pointer_up_callback = nullptr;
		self->pointer_up_callback_data = nullptr;
	}	
}


void WrapperUIElement::GetOnPointerMove(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> callback = lctx.get_property(lctx.holder, "_onPointerMove");
	info.GetReturnValue().Set(callback);
}

void WrapperUIElement::SetOnPointerMove(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onPointerMove", value);

	UIElement* self = lctx.self<UIElement>();
	if (self->pointer_move_callback_data != nullptr)
	{
		UIElementCallbackData* data = (UIElementCallbackData*)self->pointer_move_callback_data;
		delete data;
	}

	if (!value->IsNullOrUndefined())
	{
		self->pointer_move_callback = UIElementPointerCallback;

		UIElementCallbackData* data = new UIElementCallbackData;
		data->ctx = lctx.ctx();

		v8::Local<v8::Function> callback = value.As<v8::Function>();
		data->callback = CallbackT(lctx.isolate, callback);

		self->pointer_move_callback_data = data;
	}
	else
	{
		self->pointer_move_callback = nullptr;
		self->pointer_move_callback_data = nullptr;
	}	

}