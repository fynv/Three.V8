#pragma once

#include "WrapperUtils.hpp"
#include <core/Object3D.h>

class WrapperObject3D
{
public:	
	static v8::Local<v8::FunctionTemplate> create_template(v8::Isolate* isolate, v8::FunctionCallback constructor = New);
	static void New(const v8::FunctionCallbackInfo<v8::Value>& info);

private:	
	static void Dispose(const v8::FunctionCallbackInfo<v8::Value>& info);
	
	static void GetName(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetName(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

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
	templ->InstanceTemplate()->SetInternalFieldCount(1);
	templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, Dispose));
	templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, "name").ToLocalChecked(), GetName, SetName);
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

void WrapperObject3D::New(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	Object3D* self = new Object3D();
	info.This()->SetInternalField(0, v8::External::New(info.GetIsolate(), self));
}

void WrapperObject3D::Dispose(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	Object3D* self = get_self<Object3D>(info);
	delete self;
}

void WrapperObject3D::GetName(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::String> ret = v8::String::NewFromUtf8(info.GetIsolate(), self->name.c_str()).ToLocalChecked();
	info.GetReturnValue().Set(ret);
}

void WrapperObject3D::SetName(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info) 
{	
	v8::HandleScope handle_scope(info.GetIsolate());
	Object3D* self = get_self<Object3D>(info);
	v8::String::Utf8Value name(info.GetIsolate(), value);
	self->name = *name;
}

void WrapperObject3D::GetParent(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);	
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();	
	v8::Local<v8::Value> parent = v8::Null(isolate);
	if (holder->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "_parent").ToLocalChecked()).ToChecked())
	{
		parent = holder->Get(context, v8::String::NewFromUtf8(isolate, "_parent").ToLocalChecked()).ToLocalChecked();
	}
	info.GetReturnValue().Set(parent);
}

void WrapperObject3D::SetParent(v8::Local<v8::String> property, v8::Local<v8::Value> value,
	const v8::PropertyCallbackInfo<void>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	Object3D* self = (Object3D*)v8::Local<v8::External>::Cast(holder->GetInternalField(0))->Value();
	holder->Set(context, v8::String::NewFromUtf8(isolate, "_parent").ToLocalChecked(), value);
	Object3D* parent = (Object3D*)v8::Local<v8::External>::Cast(value.As<v8::Object>()->GetInternalField(0))->Value();	
	self->parent = parent;
}


void WrapperObject3D::GetChildren(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::Value> children;
	if (holder->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "_children").ToLocalChecked()).ToChecked())
	{
		children = holder->Get(context, v8::String::NewFromUtf8(isolate, "_children").ToLocalChecked()).ToLocalChecked();
	}
	else
	{
		children = v8::Array::New(isolate);
		holder->Set(context, v8::String::NewFromUtf8(isolate, "_children").ToLocalChecked(), children);
	}
	info.GetReturnValue().Set(children);
}


void WrapperObject3D::GetUp(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> up = v8::Object::New(isolate);
	vec3_to_jvec3(isolate, self->up, up);
	info.GetReturnValue().Set(up);
}

void WrapperObject3D::GetUp(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec3_to_jvec3(isolate, self->up, out);
	info.GetReturnValue().Set(out);
}

void WrapperObject3D::SetUp(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	if (info[0]->IsNumber())
	{
		self->up.x = (float)info[0].As<v8::Number>()->Value();
		self->up.y = (float)info[1].As<v8::Number>()->Value();
		self->up.z = (float)info[2].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec3_to_vec3(isolate, in, self->up);
	}
}

void WrapperObject3D::GetPosition(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> position = v8::Object::New(isolate);
	vec3_to_jvec3(isolate, self->position, position);
	info.GetReturnValue().Set(position);
}

void WrapperObject3D::GetPosition(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec3_to_jvec3(isolate, self->position, out);	
	info.GetReturnValue().Set(out);
}

void WrapperObject3D::SetPosition(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);	
	if (info[0]->IsNumber())
	{
		self->position.x = (float)info[0].As<v8::Number>()->Value();
		self->position.y = (float)info[1].As<v8::Number>()->Value();
		self->position.z = (float)info[2].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec3_to_vec3(isolate, in, self->position);
	}
}

void WrapperObject3D::GetRotation(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> rotation = v8::Object::New(isolate);
	vec3_to_jvec3(isolate, self->rotation, rotation);
	info.GetReturnValue().Set(rotation);
}

void WrapperObject3D::GetRotation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec3_to_jvec3(isolate, self->rotation, out);
	info.GetReturnValue().Set(out);
}

void WrapperObject3D::SetRotation(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	glm::vec3 rotation;
	if (info[0]->IsNumber())
	{
		rotation.x = (float)info[0].As<v8::Number>()->Value();
		rotation.y = (float)info[1].As<v8::Number>()->Value();
		rotation.z = (float)info[2].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec3_to_vec3(isolate, in, rotation);
	}
	self->set_rotation(rotation);
}

void WrapperObject3D::GetQuaternion(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> quaternion = v8::Object::New(isolate);
	quat_to_jquat(isolate, self->quaternion, quaternion);
	info.GetReturnValue().Set(quaternion);
}

void WrapperObject3D::GetQuaternion(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();	
	quat_to_jquat(isolate, self->quaternion, out);
	info.GetReturnValue().Set(out);
}

void WrapperObject3D::SetQuaternion(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	glm::quat quaternion;	
	if (info[0]->IsNumber())
	{		
		quaternion.x = (float)info[0].As<v8::Number>()->Value();
		quaternion.y = (float)info[1].As<v8::Number>()->Value();
		quaternion.z = (float)info[2].As<v8::Number>()->Value();
		quaternion.w = (float)info[3].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jquat_to_quat(isolate, in, quaternion);
	}
	self->set_quaternion(quaternion);
}



void WrapperObject3D::GetScale(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> scale = v8::Object::New(isolate);
	vec3_to_jvec3(isolate, self->scale, scale);
	info.GetReturnValue().Set(scale);
}

void WrapperObject3D::GetScale(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> out = info[0].As<v8::Object>();
	vec3_to_jvec3(isolate, self->scale, out);
	info.GetReturnValue().Set(out);
}

void WrapperObject3D::SetScale(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	if (info[0]->IsNumber())
	{
		self->scale.x = (float)info[0].As<v8::Number>()->Value();
		self->scale.y = (float)info[1].As<v8::Number>()->Value();
		self->scale.z = (float)info[2].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec3_to_vec3(isolate, in, self->scale);
	}
}

void WrapperObject3D::GetMatrix(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> matrix = v8::Object::New(isolate);
	mat4_to_jmat4(isolate, self->matrix, matrix);
	info.GetReturnValue().Set(matrix);
}

void WrapperObject3D::GetMatrix(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> matrix = info[0].As<v8::Object>();
	mat4_to_jmat4(isolate, self->matrix, matrix);
	info.GetReturnValue().Set(matrix);
}

void WrapperObject3D::GetMatrixWorld(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> matrix = v8::Object::New(isolate);
	mat4_to_jmat4(isolate, self->matrixWorld, matrix);
	info.GetReturnValue().Set(matrix);
}


void WrapperObject3D::GetMatrixWorld(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> matrix = info[0].As<v8::Object>();
	mat4_to_jmat4(isolate, self->matrixWorld, matrix);	
	info.GetReturnValue().Set(matrix);
}


void WrapperObject3D::UpdateMatrix(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Object3D* self = get_self<Object3D>(info);
	self->updateMatrix();
}


void WrapperObject3D::UpdateMatrixWorld(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Object3D* self = get_self<Object3D>(info);
	bool force = info[0].As<v8::Boolean>()->Value();
	self->updateMatrixWorld(force);
}


void WrapperObject3D::UpdateWorldMatrix(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	Object3D* self = get_self<Object3D>(info);
	bool updateParents = info[0].As<v8::Boolean>()->Value();
	bool updateChildren = info[1].As<v8::Boolean>()->Value();
	self->updateWorldMatrix(updateParents, updateChildren);
}

void WrapperObject3D::ApplyMatrix4(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> in = info[0].As<v8::Object>();
	glm::mat4 matrix;
	jmat4_to_mat4(isolate, in, matrix);
	self->applyMatrix4(matrix);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::ApplyQuaternion(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> in = info[0].As<v8::Object>();
	glm::quat quaternion;
	jquat_to_quat(isolate, in, quaternion);
	self->applyQuaternion(quaternion);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::SetRotationFromAxisAngle(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> in_axis = info[0].As<v8::Object>();
	glm::vec3 axis;
	jvec3_to_vec3(isolate, in_axis, axis);
	float angle = (float)info[1].As<v8::Number>()->Value();
	self->setRotationFromAxisAngle(axis, angle);
	info.GetReturnValue().Set(info.Holder());
}


void WrapperObject3D::SetRotationFromMatrix(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> in = info[0].As<v8::Object>();
	glm::mat4 matrix;
	jmat4_to_mat4(isolate, in, matrix);
	self->setRotationFromMatrix(matrix);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::RotateOnAxis(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> in_axis = info[0].As<v8::Object>();
	glm::vec3 axis;
	jvec3_to_vec3(isolate, in_axis, axis);
	float angle = (float)info[1].As<v8::Number>()->Value();
	self->rotateOnAxis(axis, angle);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::RotateOnWorldAxis(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> in_axis = info[0].As<v8::Object>();
	glm::vec3 axis;
	jvec3_to_vec3(isolate, in_axis, axis);
	float angle = (float)info[1].As<v8::Number>()->Value();
	self->rotateOnWorldAxis(axis, angle);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::RotateX(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	float angle = (float)info[0].As<v8::Number>()->Value();
	self->rotateX(angle);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::RotateY(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	float angle = (float)info[0].As<v8::Number>()->Value();
	self->rotateY(angle);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::RotateZ(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	float angle = (float)info[0].As<v8::Number>()->Value();
	self->rotateZ(angle);
	info.GetReturnValue().Set(info.Holder());
}


void WrapperObject3D::TranslateOnAxis(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> in_axis = info[0].As<v8::Object>();
	glm::vec3 axis;
	jvec3_to_vec3(isolate, in_axis, axis);
	float distance = (float)info[1].As<v8::Number>()->Value();
	self->translateOnAxis(axis, distance);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::TranslateX(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	float distance = (float)info[0].As<v8::Number>()->Value();
	self->translateX(distance);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::TranslateY(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	float distance = (float)info[0].As<v8::Number>()->Value();
	self->translateY(distance);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::TranslateZ(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	float distance = (float)info[0].As<v8::Number>()->Value();
	self->translateZ(distance);
	info.GetReturnValue().Set(info.Holder());
}

void WrapperObject3D::LocalToWorld(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> jvector = info[0].As<v8::Object>();
	glm::vec3 vector;
	jvec3_to_vec3(isolate, jvector, vector);
	vector = self->localToWorld(vector);
	vec3_to_jvec3(isolate, vector, jvector);
	info.GetReturnValue().Set(jvector);
}

void WrapperObject3D::WorldToLocal(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> jvector = info[0].As<v8::Object>();
	glm::vec3 vector;
	jvec3_to_vec3(isolate, jvector, vector);
	vector = self->worldToLocal(vector);
	vec3_to_jvec3(isolate, vector, jvector);
	info.GetReturnValue().Set(jvector);
}


void WrapperObject3D::GetWorldPosition(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> jvector = info[0].As<v8::Object>();
	glm::vec3 vector = self->getWorldPosition();
	vec3_to_jvec3(isolate, vector, jvector);
	info.GetReturnValue().Set(jvector);
}

void WrapperObject3D::GetWorldQuaternion(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> jquaternion = info[0].As<v8::Object>();
	glm::quat quaternion = self->getWorldQuaternion();
	quat_to_jquat(isolate, quaternion, jquaternion);
	info.GetReturnValue().Set(jquaternion);
}

void WrapperObject3D::GetWorldScale(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> jvector = info[0].As<v8::Object>();
	glm::vec3 vector = self->getWorldScale();
	vec3_to_jvec3(isolate, vector, jvector);
	info.GetReturnValue().Set(jvector);
}

void WrapperObject3D::GetWorldDirection(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	v8::Local<v8::Object> jvector = info[0].As<v8::Object>();
	glm::vec3 vector = self->getWorldDirection();
	vec3_to_jvec3(isolate, vector, jvector);
	info.GetReturnValue().Set(jvector);
}

void WrapperObject3D::LookAt(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	Object3D* self = get_self<Object3D>(info);
	glm::vec3 target;
	if (info[0]->IsNumber())
	{
		target.x = (float)info[0].As<v8::Number>()->Value();
		target.y = (float)info[1].As<v8::Number>()->Value();
		target.z = (float)info[2].As<v8::Number>()->Value();
	}
	else
	{
		v8::Local<v8::Object> in = info[0].As<v8::Object>();
		jvec3_to_vec3(isolate, in, target);
	}
	self->lookAt(target);
}

void WrapperObject3D::Add(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	Object3D* self = (Object3D*)v8::Local<v8::External>::Cast(holder->GetInternalField(0))->Value();

	v8::Local<v8::Object> holder_object = info[0].As<v8::Object>();
	Object3D* object = (Object3D*)v8::Local<v8::External>::Cast(holder_object->GetInternalField(0))->Value();

	self->add(object);

	if (holder_object->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "_parent").ToLocalChecked()).ToChecked())
	{
		v8::Local<v8::Object> holder_parent = holder_object->Get(context, v8::String::NewFromUtf8(isolate, "_parent").ToLocalChecked()).ToLocalChecked().As<v8::Object>();
		v8::Local<v8::Function> remove = holder_parent->Get(context, v8::String::NewFromUtf8(isolate, "remove").ToLocalChecked()).ToLocalChecked().As<v8::Function>();
		v8::Local<v8::Value> args[1];
		args[0] = holder_object;
		remove->Call(context, holder_parent, 1, args);
	}

	holder_object->Set(context, v8::String::NewFromUtf8(isolate, "_parent").ToLocalChecked(), holder);
	v8::Local<v8::Array> children = holder->Get(context, v8::String::NewFromUtf8(isolate, "children").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
	children->Set(context, children->Length(), holder_object);

	info.GetReturnValue().Set(holder);
}

void WrapperObject3D::Remove(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	Object3D* self = (Object3D*)v8::Local<v8::External>::Cast(holder->GetInternalField(0))->Value();

	v8::Local<v8::Object> holder_object = info[0].As<v8::Object>();
	Object3D* object = (Object3D*)v8::Local<v8::External>::Cast(holder_object->GetInternalField(0))->Value();

	self->remove(object);

	v8::Local<v8::Array> children = holder->Get(context, v8::String::NewFromUtf8(isolate, "children").ToLocalChecked()).ToLocalChecked().As<v8::Array>();	

	for (unsigned i = 0; i < children->Length(); i++)
	{
		v8::Local<v8::Object> obj_i = children->Get(context, i).ToLocalChecked().As<v8::Object>();
		if (obj_i == holder_object)
		{
			holder_object->Delete(context, v8::String::NewFromUtf8(isolate, "_parent").ToLocalChecked());
			for (unsigned j = i; j < children->Length() - 1; j++)
			{
				children->Set(context, j, children->Get(context, j+1).ToLocalChecked());
			}			
			children->Delete(context, children->Length()-1);
			children->Set(context, v8::String::NewFromUtf8(isolate, "length").ToLocalChecked(), v8::Number::New(isolate, (double)(children->Length() - 1)));
			break;
		}
	}

	info.GetReturnValue().Set(holder);
}

void WrapperObject3D::RemoveFromParent(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	Object3D* self = (Object3D*)v8::Local<v8::External>::Cast(holder->GetInternalField(0))->Value();
	self->removeFromParent();
	if (holder->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "_parent").ToLocalChecked()).ToChecked())
	{
		v8::Local<v8::Object> holder_parent = holder->Get(context, v8::String::NewFromUtf8(isolate, "_parent").ToLocalChecked()).ToLocalChecked().As<v8::Object>();
		v8::Local<v8::Function> remove = holder_parent->Get(context, v8::String::NewFromUtf8(isolate, "remove").ToLocalChecked()).ToLocalChecked().As<v8::Function>();
		v8::Local<v8::Value> args[1];
		args[0] = holder;
		remove->Call(context, holder_parent, 1, args);
	}
	info.GetReturnValue().Set(holder);
}

void WrapperObject3D::Clear(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();
	Object3D* self = (Object3D*)v8::Local<v8::External>::Cast(holder->GetInternalField(0))->Value();
	self->clear();

	v8::Local<v8::Array> children = holder->Get(context, v8::String::NewFromUtf8(isolate, "children").ToLocalChecked()).ToLocalChecked().As<v8::Array>();

	for (unsigned i = 0; i < children->Length(); i++)
	{
		v8::Local<v8::Object> obj_i = children->Get(context, i).ToLocalChecked().As<v8::Object>();		
		obj_i->Delete(context, v8::String::NewFromUtf8(isolate, "_parent").ToLocalChecked());		
		children->Delete(context, i);		
	}
	children->Set(context, v8::String::NewFromUtf8(isolate, "length").ToLocalChecked(), v8::Number::New(isolate, 0.0));

	info.GetReturnValue().Set(holder);

}

void WrapperObject3D::GetObjectByName(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();

	std::string name;
	{
		v8::String::Utf8Value _name(isolate, info[0]);
		name = *_name;
	}

	std::string name2;
	{
		v8::String::Utf8Value _name(isolate, holder->Get(context, v8::String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked());
		name2 = *_name;
	}

	if (name2 == name)
	{
		info.GetReturnValue().Set(holder);
		return;
	}

	v8::Local<v8::Array> children = holder->Get(context, v8::String::NewFromUtf8(isolate, "children").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
	for (unsigned i = 0; i < children->Length(); i++)
	{
		v8::Local<v8::Object> obj_i = children->Get(context, i).ToLocalChecked().As<v8::Object>();

		v8::Local<v8::Function> getObjectByName = obj_i->Get(context, v8::String::NewFromUtf8(isolate, "getObjectByName").ToLocalChecked()).ToLocalChecked().As<v8::Function>();
		v8::Local<v8::Value> args[1];
		args[0] = info[0];

		v8::Local<v8::Value> ret = getObjectByName->Call(context, obj_i, 1, args).ToLocalChecked();
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
	v8::Isolate* isolate = info.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> holder = info.Holder();

	v8::Local<v8::Function> callback = info[0].As<v8::Function>();
	v8::Local<v8::Value> args[1];
	args[0] = holder;
	callback->Call(context, context->Global(), 1, args);

	v8::Local<v8::Array> children = holder->Get(context, v8::String::NewFromUtf8(isolate, "children").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
	for (unsigned i = 0; i < children->Length(); i++)
	{
		v8::Local<v8::Object> obj_i = children->Get(context, i).ToLocalChecked().As<v8::Object>();

		v8::Local<v8::Function> traverse = obj_i->Get(context, v8::String::NewFromUtf8(isolate, "traverse").ToLocalChecked()).ToLocalChecked().As<v8::Function>();
		v8::Local<v8::Value> args[1];
		args[0] = callback;
		traverse->Call(context, obj_i, 1, args).ToLocalChecked();
	}
}

