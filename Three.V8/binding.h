#pragma once

#include <v8.h>
#include <libplatform/libplatform.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

class V8VM
{
public:
	V8VM(const char* exec_path);
	~V8VM();

	v8::Isolate* m_isolate;
	void RunVM(void (*callback)(void*), void* data);
   
private:
	std::unique_ptr<v8::Platform> m_platform;
	v8::ArrayBuffer::Allocator* m_array_buffer_allocator;
};

struct FunctionDefinition
{
	std::string name;
	v8::FunctionCallback func;
};

struct ClassDefinition
{
	std::string name;
	v8::FunctionCallback constructor;
	v8::Local<v8::FunctionTemplate>(*creator)(v8::Isolate* isolate, v8::FunctionCallback constructor);
};

struct GlobalDefinitions
{
	std::vector<FunctionDefinition> funcs;
	std::vector<ClassDefinition> classes;
};

class GamePlayer;
class GameContext
{
public:
	GameContext(V8VM* vm, GamePlayer* gamePlayer, const char* filename);
	~GameContext();
	
	V8VM* m_vm;

	typedef v8::Persistent<v8::Context, v8::CopyablePersistentTraits<v8::Context>> ContextT;
	ContextT m_context;

	v8::Function* GetCallback(const char* name);
	v8::MaybeLocal<v8::Value> InvokeCallback(v8::Function* callback, const std::vector<v8::Local<v8::Value>>& args);

private:
	GamePlayer* m_gamePlayer;
	static GlobalDefinitions s_globals;
	void _create_context();

	void _report_exception(v8::TryCatch* handler);
	bool _execute_string(v8::Local<v8::String> source, v8::Local<v8::Value> name, bool print_result, bool report_exceptions);
	void _run_script(const char* filename);

	typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> CallbackT;
	std::unordered_map<std::string, CallbackT> m_callbacks;

	static void Print(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void SetCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void Now(const v8::FunctionCallbackInfo<v8::Value>& args);
};