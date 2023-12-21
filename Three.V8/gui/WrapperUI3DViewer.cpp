#include "WrapperUtils.hpp"
#include <gui/UI3DViewer.h>

#include "WrapperUI3DViewer.h"

void WrapperUI3DViewer::define(ClassDefinition& cls)
{
	cls.name = "UI3DViewer";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.properties = {
		{ "block", GetBlock, SetBlock },
		{ "origin", GetOrigin },
		{ "size", GetSize },
		{ "onRender", GetOnRender, SetOnRender },
		{ "onMouseDown", GetOnMouseDown, SetOnMouseDown },
		{ "onMouseUp", GetOnMouseUp, SetOnMouseUp },
		{ "onMouseMove", GetOnMouseMove, SetOnMouseMove },
		{ "onMouseWheel", GetOnMouseWheel, SetOnMouseWheel },
		{ "onTouchDown", GetOnTouchDown, SetOnTouchDown },
		{ "onTouchUp", GetOnTouchUp, SetOnTouchUp },
		{ "onTouchMove", GetOnTouchMove, SetOnTouchMove },
	};
	cls.methods = {
		{"getOrigin", GetOrigin },
		{"setOrigin", SetOrigin },
		{"getSize", GetSize },
		{"setSize", SetSize },	
	};
}

typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> CallbackT;

struct UI3DViewerCallbackData
{
	GameContext* ctx;
	CallbackT callback;
};

void* WrapperUI3DViewer::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new UI3DViewer;
}

void WrapperUI3DViewer::dtor(void* ptr, GameContext* ctx)
{
	UI3DViewer* self = (UI3DViewer*)ptr;

	if (self->render_data != nullptr)
	{
		UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)self->render_data;
		delete data;
	}

	if (self->mouse_down_data != nullptr)
	{
		UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)self->mouse_down_data;
		delete data;
	}

	if (self->mouse_up_data != nullptr)
	{
		UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)self->mouse_up_data;
		delete data;
	}

	if (self->mouse_move_data != nullptr)
	{
		UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)self->mouse_move_data;
		delete data;
	}

	if (self->mouse_wheel_data != nullptr)
	{
		UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)self->mouse_wheel_data;
		delete data;
	}

	if (self->touch_down_data != nullptr)
	{
		UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)self->touch_down_data;
		delete data;
	}

	if (self->touch_up_data != nullptr)
	{
		UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)self->touch_up_data;
		delete data;
	}

	if (self->touch_move_data != nullptr)
	{
		UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)self->touch_move_data;
		delete data;
	}

	delete self;
}

void WrapperUI3DViewer::GetBlock(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> block = lctx.get_property(lctx.holder, "_block");
	info.GetReturnValue().Set(block);
}

void WrapperUI3DViewer::SetBlock(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	UI3DViewer* self = lctx.self<UI3DViewer>();
	UIBlock* block = lctx.jobj_to_obj<UIBlock>(value);
	self->block = block;
	lctx.set_property(lctx.holder, "_block", value);
}

void WrapperUI3DViewer::GetOrigin(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UI3DViewer* self = lctx.self<UI3DViewer>();
	v8::Local<v8::Object> origin = v8::Object::New(lctx.isolate);
	lctx.vec2_to_jvec2(self->origin, origin);
	info.GetReturnValue().Set(origin);
}


void WrapperUI3DViewer::GetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UI3DViewer* self = lctx.self<UI3DViewer>();
	lctx.vec2_to_jvec2(self->origin, info[0]);
	info.GetReturnValue().Set(info[0]);
}


void WrapperUI3DViewer::SetOrigin(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UI3DViewer* self = lctx.self<UI3DViewer>();
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
	}
}

void WrapperUI3DViewer::GetSize(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UI3DViewer* self = lctx.self<UI3DViewer>();
	v8::Local<v8::Object> size = v8::Object::New(lctx.isolate);
	lctx.vec2_to_jvec2(self->size, size);
	info.GetReturnValue().Set(size);
}


void WrapperUI3DViewer::GetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UI3DViewer* self = lctx.self<UI3DViewer>();
	lctx.vec2_to_jvec2(self->size, info[0]);
	info.GetReturnValue().Set(info[0]);
}


void WrapperUI3DViewer::SetSize(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	UI3DViewer* self = lctx.self<UI3DViewer>();
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
	}
}

static void UI3DViewerRenderCallback(int width, int height, bool size_changed, void* ptr)
{
	UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)ptr;
	GameContext* ctx = data->ctx;
	v8::Isolate* isolate = ctx->m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = ctx->m_context.Get(isolate);
	v8::Context::Scope context_scope(context);
	v8::Local<v8::Function> callback = data->callback.Get(isolate);
	std::vector<v8::Local<v8::Value>> args(3);
	args[0] = v8::Number::New(isolate, (double)width);
	args[1] = v8::Number::New(isolate, (double)height);
	args[2] = v8::Boolean::New(isolate, size_changed);
	ctx->InvokeCallback(*callback, args);

}

void WrapperUI3DViewer::GetOnRender(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> onRender = lctx.get_property(lctx.holder, "_onRender");
	info.GetReturnValue().Set(onRender);
}

void WrapperUI3DViewer::SetOnRender(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onRender", value);

	UI3DViewer* self = lctx.self<UI3DViewer>();
	self->render_callback = UI3DViewerRenderCallback;

	if (self->render_data != nullptr)
	{
		UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)self->render_data;
		delete data;
	}

	UI3DViewerCallbackData* data = new UI3DViewerCallbackData;
	data->ctx = lctx.ctx();

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(lctx.isolate, callback);

	self->render_data = data;
}


static v8::Local<v8::Object> g_CreateMouseEvent(v8::Isolate* isolate, v8::Local<v8::Context> context, int button, int clicks, int delta, int x, int y)
{
	v8::Local<v8::Object> e = v8::Object::New(isolate);
	e->Set(context, v8::String::NewFromUtf8(isolate, "button").ToLocalChecked(), v8::Number::New(isolate, (double)button));
	e->Set(context, v8::String::NewFromUtf8(isolate, "clicks").ToLocalChecked(), v8::Number::New(isolate, (double)clicks));
	e->Set(context, v8::String::NewFromUtf8(isolate, "delta").ToLocalChecked(), v8::Number::New(isolate, (double)delta));
	e->Set(context, v8::String::NewFromUtf8(isolate, "x").ToLocalChecked(), v8::Number::New(isolate, (double)x + 0.5));
	e->Set(context, v8::String::NewFromUtf8(isolate, "y").ToLocalChecked(), v8::Number::New(isolate, (double)y + 0.5));
	return e;
}


static void UI3DViewerMouseCallback(int button, int clicks, int delta, int x, int y, void* ptr)
{
	UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)ptr;
	GameContext* ctx = data->ctx;
	v8::Isolate* isolate = ctx->m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = ctx->m_context.Get(isolate);
	v8::Context::Scope context_scope(context);
	v8::Local<v8::Function> callback = data->callback.Get(isolate);
	std::vector<v8::Local<v8::Value>> args(1);
	args[0] = g_CreateMouseEvent(isolate, context, button, clicks, delta, x, y);
	ctx->InvokeCallback(*callback, args);
}

void WrapperUI3DViewer::GetOnMouseDown(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> onMouseDown = lctx.get_property(lctx.holder, "_onMouseDown");
	info.GetReturnValue().Set(onMouseDown);
}

void WrapperUI3DViewer::SetOnMouseDown(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onMouseDown", value);

	UI3DViewer* self = lctx.self<UI3DViewer>();
	self->mouse_down_callback = UI3DViewerMouseCallback;

	if (self->mouse_down_data != nullptr)
	{
		UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)self->mouse_down_data;
		delete data;
	}

	UI3DViewerCallbackData* data = new UI3DViewerCallbackData;
	data->ctx = lctx.ctx();

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(lctx.isolate, callback);

	self->mouse_down_data = data;
}

void WrapperUI3DViewer::GetOnMouseUp(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> onMouseUp = lctx.get_property(lctx.holder, "_onMouseUp");
	info.GetReturnValue().Set(onMouseUp);
}

void WrapperUI3DViewer::SetOnMouseUp(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onMouseUp", value);

	UI3DViewer* self = lctx.self<UI3DViewer>();
	self->mouse_up_callback = UI3DViewerMouseCallback;

	if (self->mouse_up_data != nullptr)
	{
		UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)self->mouse_up_data;
		delete data;
	}

	UI3DViewerCallbackData* data = new UI3DViewerCallbackData;
	data->ctx = lctx.ctx();

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(lctx.isolate, callback);

	self->mouse_up_data = data;

}

void WrapperUI3DViewer::GetOnMouseMove(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> onMouseMove = lctx.get_property(lctx.holder, "_onMouseMove");
	info.GetReturnValue().Set(onMouseMove);
}

void WrapperUI3DViewer::SetOnMouseMove(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onMouseMove", value);

	UI3DViewer* self = lctx.self<UI3DViewer>();
	self->mouse_move_callback = UI3DViewerMouseCallback;

	if (self->mouse_move_data != nullptr)
	{
		UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)self->mouse_move_data;
		delete data;
	}

	UI3DViewerCallbackData* data = new UI3DViewerCallbackData;
	data->ctx = lctx.ctx();

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(lctx.isolate, callback);

	self->mouse_move_data = data;

}

void WrapperUI3DViewer::GetOnMouseWheel(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> onMouseWheel = lctx.get_property(lctx.holder, "_onMouseWheel");
	info.GetReturnValue().Set(onMouseWheel);
}

void WrapperUI3DViewer::SetOnMouseWheel(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onMouseWheel", value);

	UI3DViewer* self = lctx.self<UI3DViewer>();
	self->mouse_wheel_callback = UI3DViewerMouseCallback;

	if (self->mouse_wheel_data != nullptr)
	{
		UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)self->mouse_wheel_data;
		delete data;
	}

	UI3DViewerCallbackData* data = new UI3DViewerCallbackData;
	data->ctx = lctx.ctx();

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(lctx.isolate, callback);

	self->mouse_wheel_data = data;

}

static v8::Local<v8::Object> g_CreateTouchEvent(v8::Isolate* isolate, v8::Local<v8::Context> context, int pointerId, float x, float y)
{
	v8::Local<v8::Object> e = v8::Object::New(isolate);
	e->Set(context, v8::String::NewFromUtf8(isolate, "pointerId").ToLocalChecked(), v8::Number::New(isolate, (double)pointerId));
	e->Set(context, v8::String::NewFromUtf8(isolate, "x").ToLocalChecked(), v8::Number::New(isolate, (double)x));
	e->Set(context, v8::String::NewFromUtf8(isolate, "y").ToLocalChecked(), v8::Number::New(isolate, (double)y));
	return e;
}


static void UI3DViewerTouchCallback(int pointerId, float x, float y, void* ptr)
{
	UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)ptr;
	GameContext* ctx = data->ctx;
	v8::Isolate* isolate = ctx->m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = ctx->m_context.Get(isolate);
	v8::Context::Scope context_scope(context);
	v8::Local<v8::Function> callback = data->callback.Get(isolate);
	std::vector<v8::Local<v8::Value>> args(1);
	args[0] = g_CreateTouchEvent(isolate, context, pointerId, x, y);
	ctx->InvokeCallback(*callback, args);
}

void WrapperUI3DViewer::GetOnTouchDown(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> onTouchDown = lctx.get_property(lctx.holder, "_onTouchDown");
	info.GetReturnValue().Set(onTouchDown);
}

void WrapperUI3DViewer::SetOnTouchDown(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onTouchDown", value);

	UI3DViewer* self = lctx.self<UI3DViewer>();
	self->touch_down_callback = UI3DViewerTouchCallback;

	if (self->touch_down_data != nullptr)
	{
		UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)self->touch_down_data;
		delete data;
	}

	UI3DViewerCallbackData* data = new UI3DViewerCallbackData;
	data->ctx = lctx.ctx();

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(lctx.isolate, callback);

	self->touch_down_data = data;

}

void WrapperUI3DViewer::GetOnTouchUp(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> onTouchUp = lctx.get_property(lctx.holder, "_onTouchUp");
	info.GetReturnValue().Set(onTouchUp);
}

void WrapperUI3DViewer::SetOnTouchUp(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onTouchUp", value);

	UI3DViewer* self = lctx.self<UI3DViewer>();
	self->touch_up_callback = UI3DViewerTouchCallback;

	if (self->touch_up_data != nullptr)
	{
		UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)self->touch_up_data;
		delete data;
	}

	UI3DViewerCallbackData* data = new UI3DViewerCallbackData;
	data->ctx = lctx.ctx();

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(lctx.isolate, callback);

	self->touch_up_data = data;

}

void WrapperUI3DViewer::GetOnTouchMove(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> onTouchMove = lctx.get_property(lctx.holder, "_onTouchMove");
	info.GetReturnValue().Set(onTouchMove);
}

void WrapperUI3DViewer::SetOnTouchMove(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(lctx.holder, "_onTouchMove", value);

	UI3DViewer* self = lctx.self<UI3DViewer>();
	self->touch_move_callback = UI3DViewerTouchCallback;

	if (self->touch_move_data != nullptr)
	{
		UI3DViewerCallbackData* data = (UI3DViewerCallbackData*)self->touch_move_data;
		delete data;
	}

	UI3DViewerCallbackData* data = new UI3DViewerCallbackData;
	data->ctx = lctx.ctx();

	v8::Local<v8::Function> callback = value.As<v8::Function>();
	data->callback = CallbackT(lctx.isolate, callback);

	self->touch_move_data = data;

}
