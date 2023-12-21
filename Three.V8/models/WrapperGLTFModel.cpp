#include "WrapperUtils.hpp"
#include "core/WrapperObject3D.h"
#include <renderers/GLRenderTarget.h>
#if THREE_MM
#include <MMCamera.h>
#include <MMLazyVideo.h>
#include <MMPlayer.h>
#endif
#include <models/GLTFModel.h>
#include <models/GeometryCreator.h>
#include <utils/Image.h>
#include <renderers/LightmapRenderTarget.h>

#include "WrapperGLTFModel.h"


void WrapperGLTFModel::define(ClassDefinition& cls)
{
	WrapperObject3D::define(cls);
	cls.name = "GLTFModel";
	cls.ctor = ctor;

	std::vector<AccessorDefinition> props = {
		{ "minPos",  GetMinPos },
		{ "maxPos", GetMaxPos },
		{ "meshes", GetMeshes },
		{ "animations", GetAnimations },
		{ "isBakable", GetIsBakable },
	};
	cls.properties.insert(cls.properties.end(), props.begin(), props.end());

	std::vector<FunctionDefinition> methods = {
		{"setTexture", SetTexture },
		{"batchPrimitives", BatchPrimitives },
		{"setAnimationFrame", SetAnimationFrame },
		{"getAnimation", GetAnimation },
		{"getAnimations", GetAnimations },
		{"addAnimation", AddAnimation },
		{"addAnimations", AddAnimations },
		{"playAnimation", PlayAnimation },
		{"stopAnimation", StopAnimation },
		{"updateAnimation", UpdateAnimation },
		{"setToonShading", SetToonShading },
		{"setLightmap", SetLightmap },
		{"getLightmap", GetLightmap },
		{"initializeLightmap", InitializeLightmap },
	};
	cls.methods.insert(cls.methods.end(), methods.begin(), methods.end());
}

void* WrapperGLTFModel::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	return new GLTFModel;
}

void WrapperGLTFModel::GetMinPos(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();
	v8::Local<v8::Object> minPos = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->m_min_pos, minPos);
	info.GetReturnValue().Set(minPos);
}

void WrapperGLTFModel::GetMaxPos(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();
	v8::Local<v8::Object> maxPos = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->m_max_pos, maxPos);
	info.GetReturnValue().Set(maxPos);
}

void WrapperGLTFModel::GetMeshes(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();
	v8::Local<v8::Array> jmeshes = v8::Array::New(lctx.isolate, (int)self->m_mesh_dict.size());

	int i = 0;
	auto iter = self->m_mesh_dict.begin();
	while (iter != self->m_mesh_dict.end())
	{
		std::string name = iter->first;
		Mesh& mesh = self->m_meshs[iter->second];

		v8::Local<v8::Object> jmesh = v8::Object::New(lctx.isolate);
		lctx.set_property(jmesh, "name", lctx.str_to_jstr(name.c_str()));

		v8::Local<v8::Array> jprimitives = v8::Array::New(lctx.isolate, (int)mesh.primitives.size());
		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];

			v8::Local<v8::Object> jprimitive = v8::Object::New(lctx.isolate);
			lctx.set_property(jprimitive, "vertices", lctx.num_to_jnum(primitive.num_pos));
			lctx.set_property(jprimitive, "triangles", lctx.num_to_jnum(primitive.num_face));
			lctx.set_property(jprimitive, "targets", lctx.num_to_jnum(primitive.num_targets));
			jprimitives->Set(lctx.context, (unsigned)j, jprimitive);
		}
		lctx.set_property(jmesh, "primitives", jprimitives);

		v8::Local<v8::Array> jweights = v8::Array::New(lctx.isolate, (int)mesh.primitives.size());

		for (size_t j = 0; j < mesh.weights.size(); j++)
		{
			jweights->Set(lctx.context, (unsigned)j, lctx.num_to_jnum(mesh.weights[j]));
		}
		lctx.set_property(jmesh, "morphWeights", jweights);

		jmeshes->Set(lctx.context, i, jmesh);

		i++;
		iter++;
	}
	info.GetReturnValue().Set(jmeshes);

}


void WrapperGLTFModel::SetTexture(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();

	std::string name = lctx.jstr_to_str(info[0]);

	int idx = 0;
	auto iter = self->m_tex_dict.find(name);
	if (iter != self->m_tex_dict.end())
	{
		idx = iter->second;
	}

	v8::Local<v8::Object> holder_image = info[1].As<v8::Object>();
	std::string clsname = lctx.jstr_to_str(holder_image->GetConstructorName());

	if (clsname == "Image")
	{
		Image* image = lctx.jobj_to_obj<Image>(holder_image);
		if (image != nullptr)
		{
			self->m_textures[idx]->load_memory_rgba(image->width(), image->height(), image->data(), true);
		}
	}
	else if (clsname == "GLRenderTarget")
	{
		GLRenderTarget* target = lctx.jobj_to_obj<GLRenderTarget>(holder_image);
		if (target != nullptr)
		{
			self->m_repl_textures[idx] = target->m_tex_video.get();
		}
	}
#if THREE_MM
	else if (clsname == "MMCamera")
	{
		MMCamera* cam = lctx.jobj_to_obj<MMCamera>(holder_image);
		if (cam != nullptr)
		{
			self->m_repl_textures[idx] = cam->get_texture();
		}
	}
	else if (clsname == "MMLazyVideo")
	{
		MMLazyVideo* video = lctx.jobj_to_obj<MMLazyVideo>(holder_image);
		if (video != nullptr)
		{
			self->m_repl_textures[idx] = video->get_texture();
		}
	}
	else if (clsname == "MMVideo")
	{
		MMVideo* video = lctx.jobj_to_obj<MMVideo>(holder_image);
		if (video != nullptr)
		{
			self->m_repl_textures[idx] = video->get_texture();
		}
	}
#endif
}

void WrapperGLTFModel::BatchPrimitives(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();
	self->batch_primitives();
}

void WrapperGLTFModel::SetAnimationFrame(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();

	AnimationFrame frame;
	lctx.jframe_to_frame(info[0], frame);
	self->setAnimationFrame(frame);
}

void WrapperGLTFModel::GetAnimations(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();

	v8::Local<v8::Array> janims = v8::Array::New(lctx.isolate, (int)self->m_animations.size());

	for (size_t i = 0; i < self->m_animations.size(); i++)
	{
		AnimationClip& anim = self->m_animations[i];

		v8::Local<v8::Object> janim = v8::Object::New(lctx.isolate);
		lctx.set_property(janim, "name", lctx.str_to_jstr(anim.name.c_str()));
		lctx.set_property(janim, "duration", lctx.num_to_jnum(anim.duration));

		v8::Local<v8::Array> jmorphs = v8::Array::New(lctx.isolate, (int)anim.morphs.size());
		for (size_t j = 0; j < anim.morphs.size(); j++)
		{
			const MorphTrack& morph = anim.morphs[j];

			v8::Local<v8::Object> jmorph = v8::Object::New(lctx.isolate);
			lctx.set_property(jmorph, "name", lctx.str_to_jstr(morph.name.c_str()));
			lctx.set_property(jmorph, "targets", lctx.num_to_jnum(morph.num_targets));
			lctx.set_property(jmorph, "frames", lctx.num_to_jnum(morph.times.size()));
			jmorphs->Set(lctx.context, (unsigned)j, jmorph);
		}
		lctx.set_property(janim, "morphs", jmorphs);
		janims->Set(lctx.context, (unsigned)i, janim);
	}

	info.GetReturnValue().Set(janims);
}

void WrapperGLTFModel::GetAnimation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();

	std::string name = lctx.jstr_to_str(info[0]);
	auto iter = self->m_animation_dict.find(name);
	if (iter != self->m_animation_dict.end())
	{
		AnimationClip& anim = self->m_animations[iter->second];
		v8::Local<v8::Object> janim = v8::Object::New(lctx.isolate);
		lctx.anim_to_janim(anim, janim);
		info.GetReturnValue().Set(janim);
	}
}

void WrapperGLTFModel::GetAnimations(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();

	v8::Local<v8::Array> janims = v8::Array::New(lctx.isolate, (int)self->m_animations.size());

	for (size_t i = 0; i < self->m_animations.size(); i++)
	{
		AnimationClip& anim = self->m_animations[i];
		v8::Local<v8::Object> janim = v8::Object::New(lctx.isolate);
		lctx.anim_to_janim(anim, janim);
		janims->Set(lctx.context, (unsigned)i, janim);
	}

	info.GetReturnValue().Set(janims);
}

void WrapperGLTFModel::AddAnimation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();

	AnimationClip anim;
	lctx.janim_to_anim(info[0], anim);
	self->addAnimation(anim);
}

void WrapperGLTFModel::AddAnimations(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();

	v8::Local<v8::Array> jAnims = info[0].As<v8::Array>();
	for (unsigned i = 0; i < jAnims->Length(); i++)
	{
		v8::Local<v8::Value> jAnim = jAnims->Get(lctx.context, i).ToLocalChecked();
		AnimationClip anim;
		lctx.janim_to_anim(jAnim, anim);
		self->addAnimation(anim);
	}
}

void WrapperGLTFModel::PlayAnimation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();
	self->playAnimation(lctx.jstr_to_str(info[0]).c_str());
}

void WrapperGLTFModel::StopAnimation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();
	self->stopAnimation(lctx.jstr_to_str(info[0]).c_str());
}

void WrapperGLTFModel::UpdateAnimation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();
	self->updateAnimation();
}

void WrapperGLTFModel::SetToonShading(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();
	int mode;
	lctx.jnum_to_num(info[0], mode);
	float width = 1.5f;
	glm::vec3 wire_color = { 0.0f, 0.0f, 0.0f };
	if (info.Length() > 1)
	{
		lctx.jnum_to_num(info[1], width);
		if (info.Length() > 2)
		{
			lctx.jvec3_to_vec3(info[2], wire_color);
		}
	}
	self->set_toon_shading(mode, width, wire_color);
}

void WrapperGLTFModel::GetIsBakable(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();
	v8::Local<v8::Boolean> bakable = v8::Boolean::New(lctx.isolate, self->lightmap != nullptr);
	info.GetReturnValue().Set(bakable);
}

void WrapperGLTFModel::SetLightmap(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();

	v8::Local<v8::Object> holder_image = info[0].As<v8::Object>();
	std::string clsname = lctx.jstr_to_str(holder_image->GetConstructorName());
	if (clsname == "HDRImage")
	{
		HDRImage* image = lctx.jobj_to_obj<HDRImage>(holder_image);
		self->load_lightmap(*image);
	}
	else if (clsname == "DDSImage")
	{
		DDSImage* image = lctx.jobj_to_obj<DDSImage>(holder_image);
		self->load_lightmap(*image);
	}
}

void WrapperGLTFModel::GetLightmap(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();

	v8::Local<v8::Object> holder_image = lctx.instantiate("HDRImage");
	HDRImage* image = lctx.jobj_to_obj<HDRImage>(holder_image);
	self->lightmap->GetImage(*image);

	info.GetReturnValue().Set(holder_image);
}

void WrapperGLTFModel::InitializeLightmap(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GLTFModel* self = lctx.self<GLTFModel>();
	GLRenderer* renderer = lctx.jobj_to_obj<GLRenderer>(info[0]);
	self->init_lightmap(renderer);

	int count = self->lightmap_target->count_valid;
	info.GetReturnValue().Set(lctx.num_to_jnum(count));
}

