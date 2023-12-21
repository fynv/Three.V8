#include "WrapperUtils.hpp"
#include "core/WrapperObject3D.h"
#include <cameras/Reflector.h>

#include "WrapperReflector.h"


void WrapperReflector::define(ClassDefinition& cls)
{
	WrapperObject3D::define(cls);
	cls.name = "Reflector";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "width",  GetWidth, SetWidth },
		{ "height",  GetHeight, SetHeight },
		{ "prim_refs",  GetPrimitiveReferences },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"addPrimitiveReference", AddPrimitiveReference },
		{"clearPrimitiveReferences", ClearPrimitiveReferences },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());
}

void* WrapperReflector::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{	
	return new Reflector;
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



