#pragma once

#include <v8.h>
#include <libplatform/libplatform.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <utility>

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

struct ObjectDefinition
{
	std::string name;
	v8::Local<v8::ObjectTemplate>(*creator)(v8::Isolate* isolate);
};

struct GlobalDefinitions
{
	std::vector<FunctionDefinition> funcs;
	std::vector<ClassDefinition> classes;
	std::vector<ObjectDefinition> objects;
};

class GamePlayer;
class HttpClient;
class WSClient;
class OpusRecorder;
class AVCRecorder;
class UIManager;
class GameContext
{
public:
	GameContext(V8VM* vm, GamePlayer* gamePlayer, const char* filename);
	~GameContext();
	
	V8VM* m_vm;

	typedef v8::Persistent<v8::Context, v8::CopyablePersistentTraits<v8::Context>> ContextT;
	ContextT m_context;

	GamePlayer* GetGamePlayer() const { return m_gamePlayer; }
	HttpClient* GetHttpClient() const { return m_http.get(); }
	UIManager* GetUIManager() const { return m_ui_manager.get(); }

	v8::Function* GetCallback(const char* name);
	v8::MaybeLocal<v8::Value> InvokeCallback(v8::Function* callback, const std::vector<v8::Local<v8::Value>>& args);

	typedef void (*Dtor)(void* ptr, GameContext* ctx);
	void regiter_object(v8::Local<v8::Object> obj, Dtor dtor);
	void remove_object(void* ptr);

	void add_ws_client(WSClient* client);
	void remove_ws_client(WSClient* client);

	void add_opus_recorder(OpusRecorder* rec);
	void remove_opus_recorder(OpusRecorder* rec);

	void add_avc_recorder(AVCRecorder* rec);
	void remove_avc_recorder(AVCRecorder* rec);

	void CheckPendings();

private:
	GamePlayer* m_gamePlayer;
	std::unique_ptr<HttpClient> m_http;
	std::unordered_set<WSClient*> m_ws_clients;
	std::unordered_set<OpusRecorder*> m_opus_recorders;
	std::unordered_set<AVCRecorder*> m_avc_recorders;
	std::unique_ptr<UIManager> m_ui_manager;
	static GlobalDefinitions s_globals;
	void _create_context();

	void _report_exception(v8::TryCatch* handler);
	bool _execute_string(v8::Local<v8::String> source, v8::Local<v8::Value> name, bool print_result, bool report_exceptions);
	void _run_script(const char* filename);

	typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> CallbackT;
	std::unordered_map<std::string, CallbackT> m_callbacks;

	typedef v8::Global<v8::Object> ObjectT;
	std::unordered_map<void*, std::pair<ObjectT, Dtor>> m_objects;

	static void Print(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void SetCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void Now(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void GetGLError(const v8::FunctionCallbackInfo<v8::Value>& args);

#if THREE_MM
	static void GetListOfCameras(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void GetListOfAudioDevices(const v8::FunctionCallbackInfo<v8::Value>& args);
#endif

	static void GeneralCall(const v8::FunctionCallbackInfo<v8::Value>& args);

	static void WeakCallback(v8::WeakCallbackInfo<GameContext> const& data);
};