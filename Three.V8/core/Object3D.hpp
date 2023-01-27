#pragma once

#include "WrapperUtils.hpp"
#include <core/Object3D.h>

class WrapperObject3D
{
public:	
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);

private:	
	static void GetName(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetName(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetUuid(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetUuid(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetParent(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetParent(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);
	static void GetChildren(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);

	static void GetUp(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetUp(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetUp(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetPosition(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetPosition(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetPosition(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetRotation(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetRotation(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetRotation(const v8::FunctionCallbackInfo<v8::Value>& info);
	
	static void GetQuaternion(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetQuaternion(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetQuaternion(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetScale(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetScale(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetScale(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetMatrix(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetMatrix(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetMatrixWorld(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetMatrixWorld(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void UpdateMatrix(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void UpdateMatrixWorld(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void UpdateWorldMatrix(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void ApplyMatrix4(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void ApplyQuaternion(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetRotationFromAxisAngle(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void SetRotationFromMatrix(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RotateOnAxis(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RotateOnWorldAxis(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RotateX(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RotateY(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RotateZ(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void TranslateOnAxis(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void TranslateX(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void TranslateY(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void TranslateZ(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void LocalToWorld(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void WorldToLocal(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetWorldPosition(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetWorldQuaternion(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetWorldScale(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GetWorldDirection(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void LookAt(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void Add(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Remove(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void RemoveFromParent(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Clear(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetObjectByName(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void Traverse(const v8::FunctionCallbackInfo<v8::Value>& info);

};

v8::Local<v8::FunctionTemplate> WrapperObject3D::create_template(v8::Isolate* isolate, v8::FunctionCallback constructor)
{
	v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, constructor);
	templ->InstanceTemplate()->SetInternalFieldCount(2);

	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(), GetName, SetName);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "uuid").ToLocalChecked(), GetUuid, SetUuid);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "parent").ToLocalChecked(), GetParent, SetParent);
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "children").ToLocalChecked(), GetChildren, 0);

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "up").ToLocalChecked(), GetUp, 0);
	templ->InstanceTemplate()->Set(isolate, "getUp", v8::FunctionTemplate::New(isolate, GetUp));
	templ->InstanceTemplate()->Set(isolate, "setUp", v8::FunctionTemplate::New(isolate, SetUp));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "position").ToLocalChecked(), GetPosition, 0);
	templ->InstanceTemplate()->Set(isolate, "getPosition", v8::FunctionTemplate::New(isolate, GetPosition));
	templ->InstanceTemplate()->Set(isolate, "setPosition", v8::FunctionTemplate::New(isolate, SetPosition));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "rotation").ToLocalChecked(), GetRotation, 0);
	templ->InstanceTemplate()->Set(isolate, "getRotation", v8::FunctionTemplate::New(isolate, GetRotation));
	templ->InstanceTemplate()->Set(isolate, "setRotation", v8::FunctionTemplate::New(isolate, SetRotation));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "quaternion").ToLocalChecked(), GetQuaternion, 0);
	templ->InstanceTemplate()->Set(isolate, "getQuaternion", v8::FunctionTemplate::New(isolate, GetQuaternion));
	templ->InstanceTemplate()->Set(isolate, "setQuaternion", v8::FunctionTemplate::New(isolate, SetQuaternion));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "scale").ToLocalChecked(), GetScale, 0);
	templ->InstanceTemplate()->Set(isolate, "getScale", v8::FunctionTemplate::New(isolate, GetScale));
	templ->InstanceTemplate()->Set(isolate, "setScale", v8::FunctionTemplate::New(isolate, SetScale));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "matrix").ToLocalChecked(), GetMatrix, 0);
	templ->InstanceTemplate()->Set(isolate, "getMatrix", v8::FunctionTemplate::New(isolate, GetMatrix));

	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "matrixWorld").ToLocalChecked(), GetMatrixWorld, 0);
	templ->InstanceTemplate()->Set(isolate, "getMatrixWorld", v8::FunctionTemplate::New(isolate, GetMatrixWorld));

	templ->InstanceTemplate()->Set(isolate, "updateMatrix", v8::FunctionTemplate::New(isolate, UpdateMatrix));
	templ->InstanceTemplate()->Set(isolate, "updateMatrixWorld", v8::FunctionTemplate::New(isolate, UpdateMatrixWorld));
	templ->InstanceTemplate()->Set(isolate, "updateWorldMatrix", v8::FunctionTemplate::New(isolate, UpdateWorldMatrix));

	templ->InstanceTemplate()->Set(isolate, "applyMatrix4", v8::FunctionTemplate::New(isolate, ApplyMatrix4));
	templ->InstanceTemplate()->Set(isolate, "applyQuaternion", v8::FunctionTemplate::New(isolate, ApplyQuaternion));
	templ->InstanceTemplate()->Set(isolate, "setRotationFromAxisAngle", v8::FunctionTemplate::New(isolate, SetRotationFromAxisAngle));
	templ->InstanceTemplate()->Set(isolate, "setRotationFromMatrix", v8::FunctionTemplate::New(isolate, SetRotationFromMatrix));
	templ->InstanceTemplate()->Set(isolate, "rotateOnAxis", v8::FunctionTemplate::New(isolate, RotateOnAxis));
	templ->InstanceTemplate()->Set(isolate, "rotateOnWorldAxis", v8::FunctionTemplate::New(isolate, RotateOnWorldAxis));
	templ->InstanceTemplate()->Set(isolate, "rotateX", v8::FunctionTemplate::New(isolate, RotateX));
	templ->InstanceTemplate()->Set(isolate, "rotateY", v8::FunctionTemplate::New(isolate, RotateY));
	templ->InstanceTemplate()->Set(isolate, "rotateZ", v8::FunctionTemplate::New(isolate, RotateZ));
	templ->InstanceTemplate()->Set(isolate, "translateOnAxis", v8::FunctionTemplate::New(isolate, TranslateOnAxis));
	templ->InstanceTemplate()->Set(isolate, "translateX", v8::FunctionTemplate::New(isolate, TranslateX));
	templ->InstanceTemplate()->Set(isolate, "translateY", v8::FunctionTemplate::New(isolate, TranslateY));
	templ->InstanceTemplate()->Set(isolate, "translateZ", v8::FunctionTemplate::New(isolate, TranslateZ));
	templ->InstanceTemplate()->Set(isolate, "localToWorld", v8::FunctionTemplate::New(isolate, LocalToWorld));
	templ->InstanceTemplate()->Set(isolate, "worldToLocal", v8::FunctionTemplate::New(isolate, WorldToLocal));
	templ->InstanceTemplate()->Set(isolate, "getWorldPosition", v8::FunctionTemplate::New(isolate, GetWorldPosition));
	templ->InstanceTemplate()->Set(isolate, "getWorldQuaternion", v8::FunctionTemplate::New(isolate, GetWorldQuaternion));
	templ->InstanceTemplate()->Set(isolate, "getWorldScale", v8::FunctionTemplate::New(isolate, GetWorldScale));
	templ->InstanceTemplate()->Set(isolate, "getWorldDirection", v8::FunctionTemplate::New(isolate, GetWorldDirection));

	templ->InstanceTemplate()->Set(isolate, "lookAt", v8::FunctionTemplate::New(isolate, LookAt));

	templ->InstanceTemplate()->Set(isolate, "add", v8::FunctionTemplate::New(isolate, Add));
	templ->InstanceTemplate()->Set(isolate, "remove", v8::FunctionTemplate::New(isolate, Remove));
	templ->InstanceTemplate()->Set(isolate, "removeFromParent", v8::FunctionTemplate::New(isolate, RemoveFromParent));
	templ->InstanceTemplate()->Set(isolate, "clear", v8::FunctionTemplate::New(isolate, Clear));

	templ->InstanceTemplate()->Set(isolate, "getObjectByName", v8::FunctionTemplate::New(isolate, GetObjectByName));
	templ->InstanceTemplate()->Set(isolate, "traverse", v8::FunctionTemplate::New(isolate, Traverse));
	
	return templ;
}


void WrapperObject3D::dtor(void* ptr, GameContext* ctx)
{
	delete (Object3D*)ptr;
}

void WrapperObject3D::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = new Object3D();
	info.This()->SetAlignedPointerInInternalField(0, self);	
	lctx.ctx()->regiter_object(info.This(), dtor);
}

void WrapperObject3D::GetName(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);	
	Object3D* self = lctx.self<Object3D>();
	info.GetReturnValue().Set(lctx.str_to_jstr(self->name.c_str()));
}

void WrapperObject3D::SetName(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info) 
{	
	LocalContext lctx(info);	
	Object3D* self = lctx.self<Object3D>();
	self->name = lctx.jstr_to_str(value);
}


void WrapperObject3D::GetUuid(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	info.GetReturnValue().Set(lctx.str_to_jstr(self->uuid.c_str()));
}

void WrapperObject3D::SetUuid(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	self->uuid = lctx.jstr_to_str(value);
}


void WrapperObject3D::GetParent(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> parent = lctx.get_property(info.Holder(), "_parent");
	info.GetReturnValue().Set(parent);
}

void WrapperObject3D::SetParent(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	lctx.set_property(info.Holder(), "_parent", value);

	Object3D* self = lctx.self<Object3D>();
	Object3D* parent = lctx.jobj_to_obj<Object3D>(value);
	self->parent = parent;
}


void WrapperObject3D::GetChildren(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Value> children = lctx.get_property(info.Holder(), "_children");
	if (children->IsNull())
	{
		children = v8::Array::New(lctx.isolate);
		lctx.set_property(info.Holder(), "_children", children);
	}
	info.GetReturnValue().Set(children);
}


void WrapperObject3D::GetUp(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	v8::Local<v8::Object> up = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->up, up);
	info.GetReturnValue().Set(up);
}

void WrapperObject3D::GetUp(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();	
	lctx.vec3_to_jvec3(self->up, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperObject3D::SetUp(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], self->up.x);
		lctx.jnum_to_num(info[1], self->up.y);
		lctx.jnum_to_num(info[2], self->up.z);
	}
	else
	{
		lctx.jvec3_to_vec3(info[0], self->up);
	}
}

void WrapperObject3D::GetPosition(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	v8::Local<v8::Object> position = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->position, position);
	info.GetReturnValue().Set(position);
}

void WrapperObject3D::GetPosition(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	lctx.vec3_to_jvec3(self->position, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperObject3D::SetPosition(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], self->position.x);
		lctx.jnum_to_num(info[1], self->position.y);
		lctx.jnum_to_num(info[2], self->position.z);
	}
	else
	{
		lctx.jvec3_to_vec3(info[0], self->position);
	}
}

void WrapperObject3D::GetRotation(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();	
	v8::Local<v8::Object> rotation = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->rotation, rotation);
	info.GetReturnValue().Set(rotation);
}

void WrapperObject3D::GetRotation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	lctx.vec3_to_jvec3(self->rotation, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperObject3D::SetRotation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	glm::vec3 rotation;
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], rotation.x);
		lctx.jnum_to_num(info[1], rotation.y);
		lctx.jnum_to_num(info[2], rotation.z);
	}
	else
	{
		lctx.jvec3_to_vec3(info[0], rotation);
	}
	self->set_rotation(rotation);
}

void WrapperObject3D::GetQuaternion(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	v8::Local<v8::Object> quaternion = v8::Object::New(lctx.isolate);
	lctx.quat_to_jquat(self->quaternion, quaternion);
	info.GetReturnValue().Set(quaternion);
}

void WrapperObject3D::GetQuaternion(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	lctx.quat_to_jquat(self->quaternion, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperObject3D::SetQuaternion(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	glm::quat quaternion;	
	if (info[0]->IsNumber())
	{	
		lctx.jnum_to_num(info[0], quaternion.x);
		lctx.jnum_to_num(info[1], quaternion.y);
		lctx.jnum_to_num(info[2], quaternion.z);
		lctx.jnum_to_num(info[3], quaternion.w);
	}
	else
	{
		lctx.jquat_to_quat(info[0], quaternion);
	}
	self->set_quaternion(quaternion);
}


void WrapperObject3D::GetScale(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	v8::Local<v8::Object> scale = v8::Object::New(lctx.isolate);
	lctx.vec3_to_jvec3(self->scale, scale);
	info.GetReturnValue().Set(scale);
}

void WrapperObject3D::GetScale(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	lctx.vec3_to_jvec3(self->scale, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperObject3D::SetScale(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();	
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], self->scale.x);
		lctx.jnum_to_num(info[1], self->scale.y);
		lctx.jnum_to_num(info[2], self->scale.z);
	}
	else
	{
		lctx.jvec3_to_vec3(info[0], self->scale);
	}
}

void WrapperObject3D::GetMatrix(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	v8::Local<v8::Object> matrix = v8::Object::New(lctx.isolate);
	lctx.mat4_to_jmat4(self->matrix, matrix);
	info.GetReturnValue().Set(matrix);
}

void WrapperObject3D::GetMatrix(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	lctx.mat4_to_jmat4(self->matrix, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperObject3D::GetMatrixWorld(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	v8::Local<v8::Object> matrix = v8::Object::New(lctx.isolate);
	lctx.mat4_to_jmat4(self->matrixWorld, matrix);
	info.GetReturnValue().Set(matrix);
}


void WrapperObject3D::GetMatrixWorld(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();	
	lctx.mat4_to_jmat4(self->matrixWorld, info[0]);
	info.GetReturnValue().Set(info[0]);
}


void WrapperObject3D::UpdateMatrix(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	self->updateMatrix();
}


void WrapperObject3D::UpdateMatrixWorld(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	bool force = info[0].As<v8::Boolean>()->Value();
	self->updateMatrixWorld(force);
}


void WrapperObject3D::UpdateWorldMatrix(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	bool updateParents = info[0].As<v8::Boolean>()->Value();
	bool updateChildren = info[1].As<v8::Boolean>()->Value();
	self->updateWorldMatrix(updateParents, updateChildren);
}

void WrapperObject3D::ApplyMatrix4(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();	
	glm::mat4 matrix;
	lctx.jmat4_to_mat4(info[0], matrix);
	self->applyMatrix4(matrix);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::ApplyQuaternion(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();	
	glm::quat quaternion;
	lctx.jquat_to_quat(info[0], quaternion);
	self->applyQuaternion(quaternion);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::SetRotationFromAxisAngle(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();	
	glm::vec3 axis;
	lctx.jvec3_to_vec3(info[0], axis);
	float angle;
	lctx.jnum_to_num(info[1], angle);	
	self->setRotationFromAxisAngle(axis, angle);
	info.GetReturnValue().Set(info.Holder());
}


void WrapperObject3D::SetRotationFromMatrix(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();	
	glm::mat4 matrix;
	lctx.jmat4_to_mat4(info[0], matrix);
	self->setRotationFromMatrix(matrix);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::RotateOnAxis(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();	
	glm::vec3 axis;
	lctx.jvec3_to_vec3(info[0], axis);
	float angle;
	lctx.jnum_to_num(info[1], angle);
	self->rotateOnAxis(axis, angle);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::RotateOnWorldAxis(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();	
	glm::vec3 axis;
	lctx.jvec3_to_vec3(info[0], axis);
	float angle;
	lctx.jnum_to_num(info[1], angle);
	self->rotateOnWorldAxis(axis, angle);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::RotateX(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	float angle;
	lctx.jnum_to_num(info[0], angle);
	self->rotateX(angle);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::RotateY(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	float angle;
	lctx.jnum_to_num(info[0], angle);
	self->rotateY(angle);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::RotateZ(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	float angle;
	lctx.jnum_to_num(info[0], angle);
	self->rotateZ(angle);
	info.GetReturnValue().Set(info.Holder());
}


void WrapperObject3D::TranslateOnAxis(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	glm::vec3 axis;
	lctx.jvec3_to_vec3(info[0], axis);
	float distance;
	lctx.jnum_to_num(info[1], distance);
	self->translateOnAxis(axis, distance);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::TranslateX(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	float distance;
	lctx.jnum_to_num(info[0], distance);
	self->translateX(distance);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::TranslateY(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	float distance;
	lctx.jnum_to_num(info[0], distance);
	self->translateY(distance);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::TranslateZ(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	float distance;
	lctx.jnum_to_num(info[0], distance);
	self->translateZ(distance);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::LocalToWorld(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	glm::vec3 vector;
	lctx.jvec3_to_vec3(info[0], vector);
	vector = self->localToWorld(vector);
	lctx.vec3_to_jvec3(vector, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperObject3D::WorldToLocal(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	glm::vec3 vector;
	lctx.jvec3_to_vec3(info[0], vector);
	vector = self->worldToLocal(vector);
	lctx.vec3_to_jvec3(vector, info[0]);
	info.GetReturnValue().Set(info[0]);
}


void WrapperObject3D::GetWorldPosition(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	glm::vec3 vector = self->getWorldPosition();
	lctx.vec3_to_jvec3(vector, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperObject3D::GetWorldQuaternion(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();	
	glm::quat quaternion = self->getWorldQuaternion();
	lctx.quat_to_jquat(quaternion, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperObject3D::GetWorldScale(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	glm::vec3 vector = self->getWorldScale();
	lctx.vec3_to_jvec3(vector, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperObject3D::GetWorldDirection(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	glm::vec3 vector = self->getWorldDirection();
	lctx.vec3_to_jvec3(vector, info[0]);
	info.GetReturnValue().Set(info[0]);
}

void WrapperObject3D::LookAt(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	glm::vec3 target;
	if (info[0]->IsNumber())
	{
		lctx.jnum_to_num(info[0], target.x);
		lctx.jnum_to_num(info[1], target.y);
		lctx.jnum_to_num(info[2], target.z);
	}
	else
	{
		lctx.jvec3_to_vec3(info[0], target);
	}
	self->lookAt(target);
}

void WrapperObject3D::Add(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	Object3D* object = lctx.jobj_to_obj<Object3D>(info[0]);
	self->add(object);

	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Object> holder_object = info[0].As<v8::Object>();

	if (lctx.has_property(holder_object, "_parent"))
	{
		v8::Local<v8::Object> holder_parent = lctx.get_property(holder_object, "_parent").As<v8::Object>();
		v8::Local<v8::Function> remove = lctx.get_property(holder_parent, "remove").As<v8::Function>();
		v8::Local<v8::Value> args[1];
		args[0] = holder_object;
		remove->Call(lctx.context, holder_parent, 1, args);
	}
	lctx.set_property(holder_object, "_parent", holder);
	v8::Local<v8::Array> children = lctx.get_property(holder, "children").As<v8::Array>();
	children->Set(lctx.context, children->Length(), holder_object);

	info.GetReturnValue().Set(holder);
}

void WrapperObject3D::Remove(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	Object3D* object = lctx.jobj_to_obj<Object3D>(info[0]);
	self->remove(object);

	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Object> holder_object = info[0].As<v8::Object>();	

	v8::Local<v8::Array> children = lctx.get_property(holder, "children").As<v8::Array>();

	for (unsigned i = 0; i < children->Length(); i++)
	{
		v8::Local<v8::Object> obj_i = children->Get(lctx.context, i).ToLocalChecked().As<v8::Object>();
		if (obj_i == holder_object)
		{
			lctx.del_property(holder_object, "_parent");
			for (unsigned j = i; j < children->Length() - 1; j++)
			{
				children->Set(lctx.context, j, children->Get(lctx.context, j+1).ToLocalChecked());
			}				
			children->Delete(lctx.context, children->Length()-1);
			lctx.set_property(children, "length", lctx.num_to_jnum(children->Length() - 1));
			break;
		}
	}

	info.GetReturnValue().Set(holder);
}

void WrapperObject3D::RemoveFromParent(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	self->removeFromParent();

	v8::Local<v8::Object> holder = info.Holder();

	if (lctx.has_property(holder, "_parent"))
	{
		v8::Local<v8::Object> holder_parent = lctx.get_property(holder, "_parent").As<v8::Object>();
		v8::Local<v8::Function> remove = lctx.get_property(holder_parent, "remove").As<v8::Function>();
		v8::Local<v8::Value> args[1];
		args[0] = holder;
		remove->Call(lctx.context, holder_parent, 1, args);
	}

	info.GetReturnValue().Set(holder);
}

void WrapperObject3D::Clear(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	Object3D* self = lctx.self<Object3D>();
	self->clear();

	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Array> children = lctx.get_property(holder, "children").As<v8::Array>();

	for (unsigned i = 0; i < children->Length(); i++)
	{
		v8::Local<v8::Object> obj_i = children->Get(lctx.context, i).ToLocalChecked().As<v8::Object>();
		lctx.del_property(obj_i, "_parent");
		children->Delete(lctx.context, i);
	}
	lctx.set_property(children, "length", lctx.num_to_jnum(0));
	info.GetReturnValue().Set(holder);

}

void WrapperObject3D::GetObjectByName(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Object> holder = info.Holder();

	std::string name = lctx.jstr_to_str(info[0]);
	std::string name2 = lctx.jstr_to_str(lctx.get_property(holder, "name"));

	if (name2 == name)
	{
		info.GetReturnValue().Set(holder);
		return;
	}

	v8::Local<v8::Array> children = lctx.get_property(holder, "children").As<v8::Array>();
	for (unsigned i = 0; i < children->Length(); i++)
	{
		v8::Local<v8::Object> obj_i = children->Get(lctx.context, i).ToLocalChecked().As<v8::Object>();
		v8::Local<v8::Function> getObjectByName = lctx.get_property(obj_i, "getObjectByName").As<v8::Function>();
		v8::Local<v8::Value> args[1];
		args[0] = info[0];

		v8::Local<v8::Value> ret = getObjectByName->Call(lctx.context, obj_i, 1, args).ToLocalChecked();
		if (!ret->IsNullOrUndefined())
		{
			info.GetReturnValue().Set(ret);
			return;
		}
	}

	info.GetReturnValue().SetNull();
}

void WrapperObject3D::Traverse(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	v8::Local<v8::Object> holder = info.Holder();	

	v8::Local<v8::Function> callback = info[0].As<v8::Function>();
	v8::Local<v8::Value> args[1];
	args[0] = holder;
	callback->Call(lctx.context, lctx.context->Global(), 1, args);

	v8::Local<v8::Array> children = lctx.get_property(holder, "children").As<v8::Array>();
	for (unsigned i = 0; i < children->Length(); i++)
	{
		v8::Local<v8::Object> obj_i = children->Get(lctx.context, i).ToLocalChecked().As<v8::Object>();
		v8::Local<v8::Function> traverse = lctx.get_property(obj_i, "traverse").As<v8::Function>();
		v8::Local<v8::Value> args[1];
		args[0] = callback;
		traverse->Call(lctx.context, obj_i, 1, args);
	}
}

