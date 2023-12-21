#pragma once

#include "definitions.hpp"

class GameContext;

class WrapperObject3D
{
public:
	static void define(ClassDefinition& cls);	
	
private:
	static void* ctor(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void dtor(void* ptr, GameContext* ctx);
	
	static void GetName(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetName(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetUuid(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetUuid(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetIsBuilding(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetIsBuilding(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void GetMoved(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetMoved(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

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

