#pragma once

#include <v8.h>
#include <glm.hpp>
#include <gtx/quaternion.hpp>

template<class T, class InfoType>
inline T* get_self(const InfoType& info)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Local<v8::Object> holder = info.Holder();
	v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(holder->GetInternalField(0));
	T* self = (T*)wrap->Value();
	return self;
}

inline void vec3_to_jvec3(v8::Isolate* isolate, const glm::vec3& vec, v8::Local<v8::Object> jvec)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	jvec->Set(context, v8::String::NewFromUtf8(isolate, "x").ToLocalChecked(), v8::Number::New(isolate, (double)vec.x));
	jvec->Set(context, v8::String::NewFromUtf8(isolate, "y").ToLocalChecked(), v8::Number::New(isolate, (double)vec.y));
	jvec->Set(context, v8::String::NewFromUtf8(isolate, "z").ToLocalChecked(), v8::Number::New(isolate, (double)vec.z));
}

inline void jvec3_to_vec3(v8::Isolate* isolate, v8::Local<v8::Object> jvec, glm::vec3& vec)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	vec.x = (float)jvec->Get(context, v8::String::NewFromUtf8(isolate, "x").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	vec.y = (float)jvec->Get(context, v8::String::NewFromUtf8(isolate, "y").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	vec.z = (float)jvec->Get(context, v8::String::NewFromUtf8(isolate, "z").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
}

inline void quat_to_jquat(v8::Isolate* isolate, const glm::quat& quat, v8::Local<v8::Object> jquat)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	jquat->Set(context, v8::String::NewFromUtf8(isolate, "x").ToLocalChecked(), v8::Number::New(isolate, (double)quat.x));
	jquat->Set(context, v8::String::NewFromUtf8(isolate, "y").ToLocalChecked(), v8::Number::New(isolate, (double)quat.y));
	jquat->Set(context, v8::String::NewFromUtf8(isolate, "z").ToLocalChecked(), v8::Number::New(isolate, (double)quat.z));
	jquat->Set(context, v8::String::NewFromUtf8(isolate, "w").ToLocalChecked(), v8::Number::New(isolate, (double)quat.w));
}

inline void jquat_to_quat(v8::Isolate* isolate, v8::Local<v8::Object> jquat, glm::quat& quat)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	quat.x = (float)jquat->Get(context, v8::String::NewFromUtf8(isolate, "x").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	quat.y = (float)jquat->Get(context, v8::String::NewFromUtf8(isolate, "y").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	quat.z = (float)jquat->Get(context, v8::String::NewFromUtf8(isolate, "z").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
	quat.w = (float)jquat->Get(context, v8::String::NewFromUtf8(isolate, "w").ToLocalChecked()).ToLocalChecked().As<v8::Number>()->Value();
}

inline void mat4_to_jmat4(v8::Isolate* isolate, const glm::mat4& matrix, v8::Local<v8::Object> jmatrix)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Array> elements;
	if (jmatrix->HasOwnProperty(context, v8::String::NewFromUtf8(isolate, "elements").ToLocalChecked()).ToChecked())
	{
		elements = jmatrix->Get(context, v8::String::NewFromUtf8(isolate, "elements").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
	}
	else
	{
		elements = v8::Array::New(isolate, 16);
		jmatrix->Set(context, v8::String::NewFromUtf8(isolate, "elements").ToLocalChecked(), elements);
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			elements->Set(context, j + i * 4, v8::Number::New(isolate, (double)matrix[i][j]));
		}
	}
}

inline void jmat4_to_mat4(v8::Isolate* isolate, v8::Local<v8::Object> jmatrix, glm::mat4& matrix)
{
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Array> elements = jmatrix->Get(context, v8::String::NewFromUtf8(isolate, "elements").ToLocalChecked()).ToLocalChecked().As<v8::Array>();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			matrix[i][j] = (float)elements->Get(context, j + i * 4).ToLocalChecked().As<v8::Number>()->Value();
		}
	}
}