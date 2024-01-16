#define ENABLE_TEST 0

#include "WrapperUtils.hpp"
#include <core/Object3D.h>
#include <core/BoundingVolumeHierarchy.h>
#include "WrapperBoundingVolumeHierarchy.h"

void WrapperBoundingVolumeHierarchy::define(ClassDefinition& cls)
{
	cls.name = "BoundingVolumeHierarchy";
	cls.ctor = ctor;
	cls.dtor = dtor;
	cls.methods = {
		{"update", Update },
		{"remove", Remove },
		{"intersect", Intersect },
		{"collide", Collide },
		{"saveFile", SaveFile },	
	};
}

void* WrapperBoundingVolumeHierarchy::ctor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);

	if (info[0]->IsArray())
	{
		v8::Local<v8::Array> objects = info[0].As<v8::Array>();
		unsigned num_objects = objects->Length();

		std::vector<Object3D*> p_objs(num_objects);

		for (unsigned i = 0; i < num_objects; i++)
		{
			v8::Local<v8::Value> holder = objects->Get(lctx.context, i).ToLocalChecked();
			p_objs[i] = lctx.jobj_to_obj<Object3D>(holder);
		}

		BoundingVolumeHierarchy* self = new BoundingVolumeHierarchy(p_objs);
		return self;
	}
	else if (info[0]->IsString())
	{
		std::string fn = lctx.jstr_to_str(info[0]);
		FILE* fp = fopen(fn.c_str(), "rb");
		BoundingVolumeHierarchy* self = new BoundingVolumeHierarchy(fp);
		fclose(fp);
		return self;
	}
}

void WrapperBoundingVolumeHierarchy::dtor(void* ptr, GameContext* ctx)
{
	delete (BoundingVolumeHierarchy*)ptr;
}

void WrapperBoundingVolumeHierarchy::Update(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	BoundingVolumeHierarchy* self = lctx.self<BoundingVolumeHierarchy>();

	v8::Local<v8::Value> holder_obj = info[0];
	Object3D* obj = lctx.jobj_to_obj<Object3D>(holder_obj);
	self->update(obj);
}

void WrapperBoundingVolumeHierarchy::Remove(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	BoundingVolumeHierarchy* self = lctx.self<BoundingVolumeHierarchy>();

	v8::Local<v8::Value> holder_obj = info[0];
	Object3D* obj = lctx.jobj_to_obj<Object3D>(holder_obj);
	self->remove(obj);
}

void WrapperBoundingVolumeHierarchy::Intersect(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	BoundingVolumeHierarchy* self = lctx.self<BoundingVolumeHierarchy>();

	v8::Local<v8::Object> ray = info[0].As<v8::Object>();
	v8::Local<v8::Value> jorigin = lctx.get_property(ray, "origin");
	v8::Local<v8::Value> jdirection = lctx.get_property(ray, "direction");
	glm::vec3 origin, direction;
	lctx.jvec3_to_vec3(jorigin, origin);
	lctx.jvec3_to_vec3(jdirection, direction);
	float near = 0.0f;
	if (lctx.has_property(ray, "near"))
	{
		lctx.jnum_to_num(lctx.get_property(ray, "near"), near);
	}
	float far = std::numeric_limits<float>::max();
	if (lctx.has_property(ray, "far"))
	{
		lctx.jnum_to_num(lctx.get_property(ray, "far"), far);
	}

	bvh::Ray<float> bvh_ray = {
		bvh::Vector3<float>(origin.x, origin.y, origin.z),
		bvh::Vector3<float>(direction.x, direction.y, direction.z),
		near,
		far
	};

	auto intersection = self->intersect(bvh_ray);

	if (intersection.has_value())
	{
		v8::Local<v8::Object> ret = v8::Object::New(lctx.isolate);
		v8::Local<v8::String> name = lctx.str_to_jstr(intersection->object->name.c_str());
		v8::Local<v8::String> uuid = lctx.str_to_jstr(intersection->object->uuid.c_str());
		lctx.set_property(ret, "name", name);
		lctx.set_property(ret, "uuid", uuid);
		lctx.set_property(ret, "distance", lctx.num_to_jnum(intersection->distance()));
		info.GetReturnValue().Set(ret);
	}
	else
	{
		info.GetReturnValue().SetNull();
	}

}

#if ENABLE_TEST
void WrapperBoundingVolumeHierarchy::Test(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	BoundingVolumeHierarchy* self = lctx.self<BoundingVolumeHierarchy>();
	PerspectiveCamera* camera = lctx.jobj_to_obj<PerspectiveCamera>(info[0]);

	int width = 800;
	int height = (int)(800.0f / camera->aspect);

	printf("%d %d\n", width, height);

	glm::vec3 origin = camera->getWorldPosition();

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


void WrapperBoundingVolumeHierarchy::Collide(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	BoundingVolumeHierarchy* self = lctx.self<BoundingVolumeHierarchy>();

	v8::Local<v8::Object> ray = info[0].As<v8::Object>();
	v8::Local<v8::Value> jorigin = lctx.get_property(ray, "origin");
	v8::Local<v8::Value> jradius = lctx.get_property(ray, "radius");
	glm::vec3 origin;
	float radius;

	lctx.jvec3_to_vec3(jorigin, origin);
	lctx.jnum_to_num(jradius, radius);

	bvh::Sphere<float> sphere = {
		bvh::Vector3<float>(origin.x, origin.y, origin.z),
		radius
	};

	auto intersection = self->collide(sphere);

	if (intersection.has_value())
	{
		v8::Local<v8::Object> ret = v8::Object::New(lctx.isolate);
		v8::Local<v8::String> name = lctx.str_to_jstr(intersection->object->name.c_str());
		v8::Local<v8::String> uuid = lctx.str_to_jstr(intersection->object->uuid.c_str());
		lctx.set_property(ret, "name", name);
		lctx.set_property(ret, "uuid", uuid);
		lctx.set_property(ret, "distance", lctx.num_to_jnum(intersection->distance()));
		v8::Local<v8::Object> pos = v8::Object::New(lctx.isolate);
		lctx.vec3_to_jvec3(self->get_intersection_pos(*intersection), pos);
		lctx.set_property(ret, "position", pos);
		info.GetReturnValue().Set(ret);
	}
	else
	{
		info.GetReturnValue().SetNull();
	}

}

void WrapperBoundingVolumeHierarchy::SaveFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	BoundingVolumeHierarchy* self = lctx.self<BoundingVolumeHierarchy>();
	std::string fn = lctx.jstr_to_str(info[0]);
	FILE* fp = fopen(fn.c_str(), "wb");
	self->serialize(fp);
	fclose(fp);
}

