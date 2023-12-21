#pragma once

#include <v8.h>
#include <libplatform/libplatform.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <utility>

#include "definitions.hpp"

class V8VM
{
public:
	V8VM(const char* exec_path);
	~V8VM();

	std::unique_ptr<v8::Platform> m_platform;
	v8::Isolate* m_isolate;	

private:	
	v8::ArrayBuffer::Allocator* m_array_buffer_allocator;
};

class GameContext
{
public:
	GameContext(V8VM* vm, const WorldDefinition& world_definition, const char* filename);
	~GameContext();
	
	V8VM* m_vm;

	typedef v8::Persistent<v8::Context, v8::CopyablePersistentTraits<v8::Context>> ContextT;
	ContextT m_context;

	static void SetCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
	v8::Function* GetCallback(const char* name);
	v8::MaybeLocal<v8::Value> InvokeCallback(v8::Function* callback, const std::vector<v8::Local<v8::Value>>& args);

	typedef void (*Dtor)(void* ptr, GameContext* ctx);
	void regiter_object(v8::Local<v8::Object> obj, Dtor dtor);
	void remove_object(void* ptr);

private:
	void _create_context(const WorldDefinition& world_definition);

	void _report_exception(v8::TryCatch* handler);
	bool _execute_string(v8::Local<v8::String> source, v8::Local<v8::Value> name, bool print_result, bool report_exceptions);
	void _run_script(const char* filename);

	typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> CallbackT;
	std::unordered_map<std::string, CallbackT> m_callbacks;

	typedef v8::Global<v8::Object> ObjectT;
	std::unordered_map<void*, std::pair<ObjectT, Dtor>> m_objects;

	static void WeakCallback(v8::WeakCallbackInfo<GameContext> const& data);

	struct CtorDtor
	{
		Constructor ctor;
		Destructor dtor;
	};
	std::vector<CtorDtor> m_cls_ctor_dtor;

	static void GeneralNew(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void GeneralDispose(const v8::FunctionCallbackInfo<v8::Value>& info);

};