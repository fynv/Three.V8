#pragma once

#include "WrapperUtils.hpp"
#include "core/Object3D.hpp"
#include <models/GLTFModel.h>
#include <models/GeometryCreator.h>
#include <utils/Image.h>

class WrapperGLTFModel
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void GetMinPos(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetMaxPos(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);

	static void GetMeshes(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);

	static void SetAnimationFrame(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetAnimations(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetAnimations(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void AddAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void AddAnimations(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void PlayAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void StopAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void UpdateAnimation(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void SetToonShading(const v8::FunctionCallbackInfo<v8::Value>& info);
};


v8::Local<v8::FunctionTemplate> WrapperGLTFModel::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = WrapperObject3D::create_template(isolate, constructor);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "minPos").ToLocalChecked(), GetMinPos, 0);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "maxPos").ToLocalChecked(), GetMaxPos, 0);

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "meshes").ToLocalChecked(), GetMeshes, 0);

	templ->InstanceTemplate()->Set(isolate, "setAnimationFrame", v8::FunctionTemplate::New(isolate, SetAnimationFrame));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "animations").ToLocalChecked(), GetAnimations, 0);
	templ->InstanceTemplate()->Set(isolate, "getAnimation", v8::FunctionTemplate::New(isolate, GetAnimation));
	templ->InstanceTemplate()->Set(isolate, "getAnimations", v8::FunctionTemplate::New(isolate, GetAnimations));

	templ->InstanceTemplate()->Set(isolate, "addAnimation", v8::FunctionTemplate::New(isolate, AddAnimation));
	templ->InstanceTemplate()->Set(isolate, "addAnimations", v8::FunctionTemplate::New(isolate, AddAnimations));

	templ->InstanceTemplate()->Set(isolate, "playAnimation", v8::FunctionTemplate::New(isolate, PlayAnimation));
	templ->InstanceTemplate()->Set(isolate, "stopAnimation", v8::FunctionTemplate::New(isolate, StopAnimation));
	templ->InstanceTemplate()->Set(isolate, "updateAnimation", v8::FunctionTemplate::New(isolate, UpdateAnimation));

	templ->InstanceTemplate()->Set(isolate, "setToonShading", v8::FunctionTemplate::New(isolate, SetToonShading));

	return templ;
}

void WrapperGLTFModel::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	GLTFModel* self = new GLTFModel();
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), WrapperObject3D::dtor);
}

void WrapperGLTFModel::GetMinPos(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	GLTFModel* self = get_self<GLTFModel>(info);
	v8::Local<v8::Object> minPos = v8::Object::New(isolate);
	vec3_to_jvec3(isolate, self->m_min_pos, minPos);
	info.GetReturnValue().Set(minPos);
}

void WrapperGLTFModel::GetMaxPos(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	GLTFModel* self = get_self<GLTFModel>(info);
	v8::Local<v8::Object> maxPos = v8::Object::New(isolate);
	vec3_to_jvec3(isolate, self->m_max_pos, maxPos);
	info.GetReturnValue().Set(maxPos);
}

void WrapperGLTFModel::GetMeshes(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	GLTFModel* self = get_self<GLTFModel>(info);
	v8::Local<v8::Array> jmeshes = v8::Array::New(isolate, (int)self->m_mesh_dict.size());

	int i = 0;
	auto iter = self->m_mesh_dict.begin();
	while (iter != self->m_mesh_dict.end())
	{
		std::string name = iter->first;
		Mesh& mesh = self->m_meshs[iter->second];

		v8::Local<v8::Object> jmesh = v8::Object::New(isolate);
		jmesh->Set(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(), v8::String::NewFromUtf8(isolate, name.c_str()).ToLocalChecked());

		v8::Local<v8::Array> jprimitives = v8::Array::New(isolate, (int)mesh.primitives.size());
		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];

			v8::Local<v8::Object> jprimitive = v8::Object::New(isolate);
			jprimitive->Set(context, v8::String::NewFromUtf8(isolate, "vertices").ToLocalChecked(), v8::Number::New(isolate, (double)primitive.num_pos));
			jprimitive->Set(context, v8::String::NewFromUtf8(isolate, "triangles").ToLocalChecked(), v8::Number::New(isolate, (double)primitive.num_face));
			jprimitive->Set(context, v8::String::NewFromUtf8(isolate, "targets").ToLocalChecked(), v8::Number::New(isolate, (double)primitive.num_targets));
			jprimitives->Set(context, (unsigned)j, jprimitive);
		}
		jmesh->Set(context, v8::String::NewFromUtf8(isolate, "primitives").ToLocalChecked(), jprimitives);

		v8::Local<v8::Array> jweights = v8::Array::New(isolate, (int)mesh.primitives.size());

		for (size_t j = 0; j < mesh.weights.size(); j++)
		{
			jweights->Set(context, (unsigned)j, v8::Number::New(isolate, (double)mesh.weights[j]));
		}

		jmesh->Set(context, v8::String::NewFromUtf8(isolate, "morphWeights").ToLocalChecked(), jweights);

		jmeshes->Set(context, i, jmesh);

		i++;
		iter++;
	}
	info.GetReturnValue().Set(jmeshes);

}

void WrapperGLTFModel::SetAnimationFrame(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);	
	GLTFModel* self = get_self<GLTFModel>(info);

	v8::Local<v8::Object> jFrame = info[0].As<v8::Object>();	
	AnimationFrame frame;
	jframe_to_frame(isolate, jFrame, frame);
	self->setAnimationFrame(frame);
}

void WrapperGLTFModel::GetAnimations(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	GLTFModel* self = get_self<GLTFModel>(info);

	v8::Local<v8::Array> janims = v8::Array::New(isolate, (int)self->m_animations.size());

	for (size_t i = 0; i < self->m_animations.size(); i++)
	{
		AnimationClip& anim = self->m_animations[i];

		v8::Local<v8::Object> janim = v8::Object::New(isolate);
		janim->Set(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(), v8::String::NewFromUtf8(isolate, anim.name.c_str()).ToLocalChecked());
		janim->Set(context, v8::String::NewFromUtf8(isolate, "duration").ToLocalChecked(), v8::Number::New(isolate, anim.duration));		

		v8::Local<v8::Array> jmorphs = v8::Array::New(isolate, (int)anim.morphs.size());
		for (size_t j = 0; j < anim.morphs.size(); j++)
		{
			const MorphTrack& morph = anim.morphs[j];

			v8::Local<v8::Object> jmorph = v8::Object::New(isolate);
			jmorph->Set(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(), v8::String::NewFromUtf8(isolate, morph.name.c_str()).ToLocalChecked());
			jmorph->Set(context, v8::String::NewFromUtf8(isolate, "targets").ToLocalChecked(), v8::Number::New(isolate, (double)morph.num_targets));
			jmorph->Set(context, v8::String::NewFromUtf8(isolate, "frames").ToLocalChecked(), v8::Number::New(isolate, (double)morph.times.size()));
			jmorphs->Set(context, (unsigned)j, jmorph);
		}

		janim->Set(context, v8::String::NewFromUtf8(isolate, "morphs").ToLocalChecked(), jmorphs);

		janims->Set(context, (unsigned)i, janim);
	}

	info.GetReturnValue().Set(janims);
}

void WrapperGLTFModel::GetAnimation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	GLTFModel* self = get_self<GLTFModel>(info);

	v8::String::Utf8Value name(isolate, info[0]);
	auto iter = self->m_animation_dict.find(*name);
	if (iter != self->m_animation_dict.end())
	{
		AnimationClip& anim = self->m_animations[iter->second];
		v8::Local<v8::Object> janim = v8::Object::New(isolate);
		anim_to_janim(isolate, anim, janim);
		info.GetReturnValue().Set(janim);
	}
}

void WrapperGLTFModel::GetAnimations(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	GLTFModel* self = get_self<GLTFModel>(info);
	
	v8::Local<v8::Array> janims = v8::Array::New(isolate, (int)self->m_animations.size());

	for (size_t i = 0; i < self->m_animations.size(); i++)
	{
		AnimationClip& anim = self->m_animations[i];
		v8::Local<v8::Object> janim = v8::Object::New(isolate);
		anim_to_janim(isolate, anim, janim);
		janims->Set(context, (unsigned)i, janim);
	}
	
	info.GetReturnValue().Set(janims);
}

void WrapperGLTFModel::AddAnimation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	GLTFModel* self = get_self<GLTFModel>(info);

	v8::Local<v8::Object> jAnim = info[0].As<v8::Object>();
	AnimationClip anim;
	janim_to_anim(isolate, jAnim, anim);
	self->addAnimation(anim);
}

void WrapperGLTFModel::AddAnimations(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	GLTFModel* self = get_self<GLTFModel>(info);

	v8::Local<v8::Array> jAnims = info[0].As<v8::Array>();
	for (unsigned i = 0; i < jAnims->Length(); i++)
	{
		v8::Local<v8::Object> jAnim = jAnims->Get(context, i).ToLocalChecked().As<v8::Object>();
		AnimationClip anim;
		janim_to_anim(isolate, jAnim, anim);
		self->addAnimation(anim);
	}
}

void WrapperGLTFModel::PlayAnimation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	GLTFModel* self = get_self<GLTFModel>(info);
	v8::String::Utf8Value name(isolate, info[0]);
	self->playAnimation(*name);
}

void WrapperGLTFModel::StopAnimation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	GLTFModel* self = get_self<GLTFModel>(info);
	v8::String::Utf8Value name(isolate, info[0]);
	self->stopAnimation(*name);
}

void WrapperGLTFModel::UpdateAnimation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	GLTFModel* self = get_self<GLTFModel>(info);
	self->updateAnimation();
}

void WrapperGLTFModel::SetToonShading(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	GLTFModel* self = get_self<GLTFModel>(info);
	int mode = (int)info[0].As<v8::Number>()->Value();
	float width = 1.5f;
	if (info.Length() > 1)
	{
		width = (float)info[1].As<v8::Number>()->Value();
	}
	self->set_toon_shading(mode, width);
}
