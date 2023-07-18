#pragma once

#include "WrapperUtils.hpp"
#include "core/Object3D.hpp"
#include <cameras/Reflector.h>


class WrapperReflector
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetWidth(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetHeight(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetPrimitiveReferences(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void AddPrimitiveReference(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void ClearPrimitiveReferences(const v8::FunctionCallbackInfo<v8::Value>& info);

};

v8::Local<v8::FunctionTemplate> WrapperReflector::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperObject3D::create_template(isolate, constructor);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "width").ToLocalChecked(), GetWidth, SetWidth);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "height").ToLocalChecked(), GetHeight, SetHeight);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "prim_refs").ToLocalChecked(), GetPrimitiveReferences, 0);
	templ->InstanceTemplate()->Set(isolate, "addPrimitiveReference", v8::FunctionTemplate::New(isolate, AddPrimitiveReference));
	templ->InstanceTemplate()->Set(isolate, "clearPrimitiveReferences", v8::FunctionTemplate::New(isolate, ClearPrimitiveReferences));
	return templ;
}

void WrapperReflector::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Reflector* self = new Reflector();
	info.This()->SetAlignedPointerInInternalField(0, self);
	lctx.ctx()->regiter_object(info.This(), WrapperObject3D::dtor);
}

void WrapperReflector::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Reflector* self = lctx.self<Reflector>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->width));
}

void WrapperReflector::SetWidth(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Reflector* self = lctx.self<Reflector>();
	lctx.jnum_to_num(value, self->width);	
}


void WrapperReflector::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Reflector* self = lctx.self<Reflector>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->height));
}

void WrapperReflector::SetHeight(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Reflector* self = lctx.self<Reflector>();
	lctx.jnum_to_num(value, self->height);
}

void WrapperReflector::GetPrimitiveReferences(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> prim_refs = lctx.get_property(info.Holder(), "_prim_refs");
	if (prim_refs->IsNull())
	{
		prim_refs = v8::Array::New(lctx.isolate);
		lctx.set_property(info.Holder(), "_prim_refs", prim_refs);
	}
	info.GetReturnValue().Set(prim_refs);
}


void WrapperReflector::AddPrimitiveReference(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Reflector* self = lctx.self<Reflector>();
	Object3D* object = lctx.jobj_to_obj<Object3D>(info[0]);
	int mesh_id = 0;
	int prim_id = 0;
	if (info.Length() > 1)
	{
		lctx.jnum_to_num(info[1], mesh_id);
		if (info.Length() > 2)
		{
			lctx.jnum_to_num(info[2], prim_id);
		}
	}
	self->m_prim_refs.push_back({ object,mesh_id,prim_id });

	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Object> prim_ref = v8::Object::New(lctx.isolate);	
	lctx.set_property(prim_ref, "model", info[0]);
	if (info.Length() > 1)
	{
		lctx.set_property(prim_ref, "mesh_id", info[1]);
		if (info.Length() > 2)
		{
			lctx.set_property(prim_ref, "prim_id", info[2]);
		}
	}	
	v8::Local<v8::Array> prim_refs = lctx.get_property(holder, "prim_refs").As<v8::Array>();
	prim_refs->Set(lctx.context, prim_refs->Length(), prim_ref);
}

void WrapperReflector::ClearPrimitiveReferences(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Reflector* self = lctx.self<Reflector>();
	self->m_prim_refs.clear();

	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Array> prim_refs = lctx.get_property(holder, "prim_refs").As<v8::Array>();

	for (unsigned i = 0; i < prim_refs->Length(); i++)
	{
		v8::Local<v8::Object> obj_i = prim_refs->Get(lctx.context, i).ToLocalChecked().As<v8::Object>();		
		prim_refs->Delete(lctx.context, i);
	}
	lctx.set_property(prim_refs, "length", lctx.num_to_jnum(0));
	info.GetReturnValue().Set(holder);

}

