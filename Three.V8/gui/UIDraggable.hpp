#pragma once

#include "WrapperUtils.hpp"
#include "UIPanel.hpp"
#include <gui/UIDraggable.h>


class WrapperUIDraggable
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

private:	
	static void GetDraggableHorizontal(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetDraggableHorizontal(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
	static void GetDraggableVertical(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetDraggableVertical(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetOriginMin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetOriginMin(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetOriginMin(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetOriginMax(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetOriginMax(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetOriginMax(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetValue(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetValue(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetValue(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetOnDrag(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetOnDrag(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
};


v8::Local<v8::FunctionTemplate> WrapperUIDraggable::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperUIPanel::create_template(isolate, constructor);

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "draggableHorizontal").ToLocalChecked(), GetDraggableHorizontal, SetDraggableHorizontal);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "draggableVertical").ToLocalChecked(), GetDraggableVertical, SetDraggableVertical);

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "originMin").ToLocalChecked(), GetOriginMin, 0);
	templ->InstanceTemplate()->Set(isolate, "getOriginMin", v8::FunctionTemplate::New(isolate, GetOriginMin));
	templ->InstanceTemplate()->Set(isolate, "setOriginMin", v8::FunctionTemplate::New(isolate, SetOriginMin));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "originMax").ToLocalChecked(), GetOriginMax, 0);
	templ->InstanceTemplate()->Set(isolate, "getOriginMax", v8::FunctionTemplate::New(isolate, GetOriginMax));
	templ->InstanceTemplate()->Set(isolate, "setOriginMax", v8::FunctionTemplate::New(isolate, SetOriginMax));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "value").ToLocalChecked(), GetValue, 0);
	templ->InstanceTemplate()->Set(isolate, "getValue", v8::FunctionTemplate::New(isolate, GetValue));
	templ->InstanceTemplate()->Set(isolate, "setValue", v8::FunctionTemplate::New(isolate, SetValue));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "onDrag").ToLocalChecked(), GetOnDrag, SetOnDrag);	
	
	return templ;
}

void WrapperUIDraggable::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIDraggable* self = new UIDraggable();
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), dtor);
}


typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> CallbackT;

struct UIDragData
{
	GameContext* ctx;
	CallbackT callback;
};

void WrapperUIDraggable::dtor(void* ptr, GameContext* ctx)
{
	UIDraggable* self = (UIDraggable*)ptr;
	if (self->drag_callback_data != nullptr)
	{
		UIDragData* data = (UIDragData*)self->drag_callback_data;
		delete data;
	}	
	WrapperUIElement::dtor(ptr, ctx);
}

void WrapperUIDraggable::GetDraggableHorizontal(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIDraggable* self = lctx.self<UIDraggable>();
	v8::Local<v8::Boolean> ret = v8::Boolean::New(lctx.isolate, self->draggable_horizontal);
	info.GetReturnValue().Set(ret);
}

void WrapperUIDraggable::SetDraggableHorizontal(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	UIDraggable* self = lctx.self<UIDraggable>();
	self->draggable_horizontal = value.As<v8::Boolean>()->Value();
}

void WrapperUIDraggable::GetDraggableVertical(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIDraggable* self = lctx.self<UIDraggable>();
	v8::Local<v8::Boolean> ret = v8::Boolean::New(lctx.isolate, self->draggable_vertical);
	info.GetReturnValue().Set(ret);
}

void WrapperUIDraggable::SetDraggableVertical(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	UIDraggable* self = lctx.self<UIDraggable>();
	self->draggable_vertical = value.As<v8::Boolean>()->Value();
}

void WrapperUIDraggable::GetOriginMin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIDraggable* self = lctx.self<UIDraggable>();
	v8::Local<v8::Object> pos = v8::Object::New(lctx.isolate);
	lctx.vec2_to_jvec2(self->origin_min, pos);
	info.GetReturnValue().Set(pos);
}

void WrapperUIDraggable::GetOriginMin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIDraggable* self = lctx.self<UIDraggable>();
	lctx.vec2_to_jvec2(self->origin_min, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperUIDraggable::SetOriginMin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIDraggable* self = lctx.self<UIDraggable>();
	glm::vec2 pos;
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], pos.x);
		lctx.jnum_to_num(info[1], pos.y);
	}
	else
	{
		lctx.jvec2_to_vec2(info[0], pos);
	}
	if (self->origin_min != pos)
	{
		self->origin_min = pos;
		self->appearance_changed = true;
	}
}


void WrapperUIDraggable::GetOriginMax(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIDraggable* self = lctx.self<UIDraggable>();
	v8::Local<v8::Object> pos = v8::Object::New(lctx.isolate);
	lctx.vec2_to_jvec2(self->origin_max, pos);
	info.GetReturnValue().Set(pos);
}

void WrapperUIDraggable::GetOriginMax(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIDraggable* self = lctx.self<UIDraggable>();
	lctx.vec2_to_jvec2(self->origin_max, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperUIDraggable::SetOriginMax(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIDraggable* self = lctx.self<UIDraggable>();
	glm::vec2 pos;
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], pos.x);
		lctx.jnum_to_num(info[1], pos.y);
	}
	else
	{
		lctx.jvec2_to_vec2(info[0], pos);
	}
	if (self->origin_max != pos)
	{
		self->origin_max = pos;
		self->appearance_changed = true;
	}
}


void WrapperUIDraggable::GetValue(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIDraggable* self = lctx.self<UIDraggable>();
	v8::Local<v8::Object> pos = v8::Object::New(lctx.isolate);
	glm::vec2 val;
	self->get_value(val);
	lctx.vec2_to_jvec2(val, pos);
	info.GetReturnValue().Set(pos);
}

void WrapperUIDraggable::GetValue(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIDraggable* self = lctx.self<UIDraggable>();
	glm::vec2 val;
	self->get_value(val);
	lctx.vec2_to_jvec2(val, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperUIDraggable::SetValue(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UIDraggable* self = lctx.self<UIDraggable>();
	glm::vec2 val;
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], val.x);
		lctx.jnum_to_num(info[1], val.y);
	}
	else
	{
		lctx.jvec2_to_vec2(info[0], val);
	}
	self->set_value(val);
}

static void UIDragCallback(void* ptr, const glm::vec2& value)
{
	UIDragData* data = (UIDragData*)ptr;
	GameContext* ctx = data->ctx;
	v8::Isolate* isolate = ctx->m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = ctx->m_context.Get(isolate);
	v8::Context::Scope context_scope(context);
	v8::Local<v8::Function> callback = data->callback.Get(isolate);	
	std::vector<v8::Local<v8::Value>> args(2);
	args[0] = v8::Number::New(isolate, (double)value.x);
	args[1] = v8::Number::New(isolate, (double)value.y);	
	ctx->InvokeCallback(*callback, args);
}

void WrapperUIDraggable::GetOnDrag(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> onClick = lctx.get_property(lctx.holder, "_onDrag");
	info.GetReturnValue().Set(onClick);
}

void WrapperUIDraggable::SetOnDrag(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onDrag", value);

	UIDraggable* self = lctx.self<UIDraggable>();
	self->drag_callback = UIDragCallback;

	if (self->drag_callback_data != nullptr)
	{
		UIDragData* data = (UIDragData*)self->drag_callback_data;
		delete data;
	}

	UIDragData* data = new UIDragData;
	data->ctx = lctx.ctx();

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(lctx.isolate, callback);

	self->drag_callback_data = data;

}
