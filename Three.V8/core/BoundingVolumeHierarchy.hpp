#pragma once

#define ENABLE_TEST 0

#include "WrapperUtils.hpp"
#include <core/Object3D.h>
#include <core/BoundingVolumeHierarchy.h>

class WrappeBoundingVolumeHierarchy
{
public:
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
	static void dtor(void* ptr);
	static void Intersect(const v8::FunctionCallbackInfo<v8::Value>& info);
	
#if ENABLE_TEST
	static void Test(const v8::FunctionCallbackInfo<v8::Value>& info);
#endif

};

v8::Local<v8::FunctionTemplate> WrappeBoundingVolumeHierarchy::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));
	templ->InstanceTemplate()->Set(isolate, "intersect", v8::FunctionTemplate::New(isolate, Intersect));

#if ENABLE_TEST
	templ->InstanceTemplate()->Set(isolate, "test", v8::FunctionTemplate::New(isolate, Test));
#endif

	return templ;
}


void WrappeBoundingVolumeHierarchy::dtor(void* ptr)
{
	delete (BoundingVolumeHierarchy*)ptr;
}

void WrappeBoundingVolumeHierarchy::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	v8::Local<v8::Array> objects = info[0].As<v8::Array>();
	unsigned num_objects = objects->Length();

	std::vector<Object3D*> p_objs(num_objects);

	for (unsigned i = 0; i < num_objects; i++)
	{
		v8::Local<v8::Object> holder = objects->Get(context, i).ToLocalChecked().As<v8::Object>();	
		p_objs[i] = (Object3D*)holder->GetAlignedPointerFromInternalField(0);
	}

	BoundingVolumeHierarchy* self = new BoundingVolumeHierarchy(p_objs);
	info.This()->SetAlignedPointerInInternalField(0, self);
	GameContext* ctx = get_context(info);
	ctx->regiter_object(info.This(), dtor);
}


void WrappeBoundingVolumeHierarchy::Intersect(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();

	v8::Local<v8::Object> ray = info[0].As<v8::Object>();
	v8::Local<v8::Object> jorigin = ray->Get(context, v8::String::NewFromUtf8(isolate, "origin").ToLocalChecked()).ToLocalChecked().As<v8::Object>();
	v8::Local<v8::Object> jdirection = ray->Get(context, v8::String::NewFromUtf8(isolate, "direction").ToLocalChecked()).ToLocalChecked().As<v8::Object>();
	glm::vec3 origin, direction;
	jvec3_to_vec3(isolate, jorigin, origin);
	jvec3_to_vec3(isolate, jdirection, direction);
	float near = 0.0f;
	if (ray->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "near").ToLocalChecked()).ToChecked())
	{
		near = (float)ray->Get(context, v8::String::NewFromUtf8(isolate, "near").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	}
	float far = std::numeric_limits<float>::max();
	if (ray->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "far").ToLocalChecked()).ToChecked())
	{
		far = (float)ray->Get(context, v8::String::NewFromUtf8(isolate, "far").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	}

	BoundingVolumeHierarchy* self = get_self<BoundingVolumeHierarchy>(info);
	bvh::Ray<float> bvh_ray = {
		bvh::Vector3<float>(origin.x, origin.y, origin.z),
		bvh::Vector3<float>(direction.x, direction.y, direction.z),
		near,
		far
	};

	auto intersection = self->intersect(bvh_ray);

	v8::Local<v8::Object> ret = v8::Object::New(isolate);
	if (intersection.has_value())
	{	
		v8::Local<v8::String> name = v8::String::NewFromUtf8(isolate, intersection->object->name.c_str()).ToLocalChecked();
		ret->Set(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(), name);
		ret->Set(context, v8::String::NewFromUtf8(isolate, "distance").ToLocalChecked(), v8::Number::New(isolate, (double)intersection->distance()));
		info.GetReturnValue().Set(ret);		
	}
	else
	{
		info.GetReturnValue().SetNull();
	}
	
}

#if ENABLE_TEST
void WrappeBoundingVolumeHierarchy::Test(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);

	v8::Local<v8::Object> holder_camera = info[0].As<v8::Object>();	
	PerspectiveCamera* camera = holder_camera->GetAlignedPointerFromInternalField(0);

	int width = 800;
	int height = (int)(800.0f / camera->aspect);

	printf("%d %d\n", width, height);

	glm::vec3 origin = camera->getWorldPosition();
	BoundingVolumeHierarchy* self = get_self<BoundingVolumeHierarchy>(info);

	std::vector<uint8_t> depth(width * height);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{			
			float fx = ((float)x + 0.0f) / (float)width * 2.0f - 1.0f;
			float fy = 1.0f - ((float)y + 0.0f) / (float)height * 2.0f;

			glm::vec4 view_pos = camera->projectionMatrixInverse * glm::vec4(fx, fy, 0.0f, 1.0f);
			view_pos /= view_pos.w;			
			glm::vec3 world_pos = camera->localToWorld(view_pos);
			glm::vec3 direction = glm::normalize(world_pos - origin);			

			bvh::Ray<float> bvh_ray = bvh::Ray<float>(
				bvh::Vector3<float>(origin.x, origin.y, origin.z),
				bvh::Vector3<float>(direction.x, direction.y, direction.z)			
			);

			float d = 1.0f;

			auto intersection = self->intersect(bvh_ray);
			if (intersection.has_value())
			{
				//d = (intersection->t - camera->z_near) / (camera->z_far - camera->z_near);
				d = intersection->t / 15.0f;
				if (d < 0.0f) d = 0.0f;
				if (d > 1.0f) d = 1.0f;
			}

			size_t i = x + y * width;
			depth[i] = (uint8_t)(d * 255.0f + 0.5f);
		}
	}

	FILE* fp = fopen("dmp_depth.raw", "wb");
	fwrite(depth.data(), 1, width * height, fp);
	fclose(fp);

}
#endif 

