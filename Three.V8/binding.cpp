#include <assert.h>
#include <utils/Utils.h>
#include "binding.h"

V8VM::V8VM(const char* exec_path)
{
	v8::V8::InitializeICUDefaultLocation(exec_path);
	v8::V8::InitializeExternalStartupData(exec_path);
	m_platform = v8::platform::NewDefaultPlatform();
	v8::V8::InitializePlatform(m_platform.get());
	v8::V8::Initialize();
	int argc = 1;
	char* argv = (char*)exec_path;
	v8::V8::SetFlagsFromCommandLine(&argc, &argv, true);
	v8::Isolate::CreateParams create_params;
	m_array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	create_params.array_buffer_allocator = m_array_buffer_allocator;
	m_isolate = v8::Isolate::New(create_params);	
}

V8VM::~V8VM()
{	
	m_isolate->Dispose();
	v8::V8::Dispose();
	v8::V8::ShutdownPlatform();
	delete m_array_buffer_allocator;
}

void V8VM::RunVM(void (*callback)(void*), void* data)
{
	v8::Isolate::Scope isolate_scope(m_isolate);
	v8::HandleScope handle_scope(m_isolate);
	callback(data);
}


#include "core/Object3D.hpp"
#include "cameras/Camera.hpp"
#include "cameras/PerspectiveCamera.hpp"
#include "backgrounds/Background.hpp"
#include "scenes/Scene.hpp"
#include "renderers/GLRenderer.hpp"
#include "models/SimpleModel.hpp"
#include "utils/Image.hpp"


GlobalDefinitions GameContext::s_globals =
{
	{
		{"print", GameContext::Print},
		{"setCallback", GameContext::SetCallback},
		{"now", GameContext::Now},
	},
	{
		{ "Object3D", WrapperObject3D::New,  WrapperObject3D::create_template },
		{ "Camera", WrapperCamera::New,  WrapperCamera::create_template },
		{ "PerspectiveCamera", WrapperPerspectiveCamera::New,  WrapperPerspectiveCamera::create_template },
		{ "ColorBackground", WrapperColorBackground::New,  WrapperColorBackground::create_template },
		{ "Scene", WrapperScene::New,  WrapperScene::create_template },
		{ "GLRenderer", WrapperGLRenderer::New,  WrapperGLRenderer::create_template },
		{ "SimpleModel", WrapperSimpleModel::New,  WrapperSimpleModel::create_template },
		{ "Image", WrapperImage::New,  WrapperImage::create_template }
	}
};

GameContext::GameContext(V8VM* vm, const std::vector<GlobalObjectWrapper>& global_objs, const char* filename): m_vm(vm)
{
	_create_context(global_objs);
	v8::Context::Scope context_scope(m_context.Get(m_vm->m_isolate));
	_run_script(filename);
}

GameContext::~GameContext()
{

}

void GameContext::_create_context(const std::vector<GlobalObjectWrapper>& global_objs)
{
	v8::Isolate* isolate = m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);

	v8::Local<v8::ObjectTemplate> global_tmpl = v8::ObjectTemplate::New(isolate);
	global_tmpl->SetInternalFieldCount(1);	

	for (size_t i = 0; i < s_globals.funcs.size(); i++)
	{
		const FunctionDefinition& func = s_globals.funcs[i];
		global_tmpl->Set(isolate, func.name.c_str(), v8::FunctionTemplate::New(isolate, func.func));
	}

	for (size_t i = 0; i < s_globals.classes.size(); i++)
	{
		const ClassDefinition& cls = s_globals.classes[i];
		global_tmpl->Set(isolate, cls.name.c_str(), cls.creator(isolate, cls.constructor));
	}
	
	v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global_tmpl);
	v8::Local<v8::Object> global_obj = context->Global();
	global_obj->SetInternalField(0, v8::External::New(isolate, this));

	for (size_t i = 0; i < global_objs.size(); i++)
	{
		const GlobalObjectWrapper& wobj = global_objs[i];
		v8::Local<v8::ObjectTemplate> templ = wobj.creator(isolate);
		v8::Local<v8::Object> obj = templ->NewInstance(context).ToLocalChecked();
		obj->SetInternalField(0, v8::External::New(isolate, wobj.ptr));
		global_obj->Set(context, v8::String::NewFromUtf8(isolate, wobj.name.c_str()).ToLocalChecked(), obj);
	}

	m_context = ContextT(isolate, context);
}

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
	return *value ? *value : "<string conversion failed>";
}

void GameContext::_report_exception(v8::TryCatch* try_catch)
{
	v8::HandleScope handle_scope(m_vm->m_isolate);
	v8::String::Utf8Value exception(m_vm->m_isolate, try_catch->Exception());
	const char* exception_string = ToCString(exception);
	v8::Local<v8::Message> message = try_catch->Message();
	if (message.IsEmpty()) {
		// V8 didn't provide any extra information about this error; just
		// print the exception.
		fprintf(stderr, "%s\n", exception_string);
	}
	else {
		// Print (filename):(line number): (message).
		v8::String::Utf8Value filename(m_vm->m_isolate,
			message->GetScriptOrigin().ResourceName());
		v8::Local<v8::Context> context(m_vm->m_isolate->GetCurrentContext());
		const char* filename_string = ToCString(filename);
		int linenum = message->GetLineNumber(context).FromJust();
		fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
		// Print line of source code.
		v8::String::Utf8Value sourceline(
			m_vm->m_isolate, message->GetSourceLine(context).ToLocalChecked());
		const char* sourceline_string = ToCString(sourceline);
		fprintf(stderr, "%s\n", sourceline_string);
		// Print wavy underline (GetUnderline is deprecated).
		int start = message->GetStartColumn(context).FromJust();
		for (int i = 0; i < start; i++) {
			fprintf(stderr, " ");
		}
		int end = message->GetEndColumn(context).FromJust();
		for (int i = start; i < end; i++) {
			fprintf(stderr, "^");
		}
		fprintf(stderr, "\n");
		v8::Local<v8::Value> stack_trace_string;
		if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) &&
			stack_trace_string->IsString() &&
			stack_trace_string.As<v8::String>()->Length() > 0) {
			v8::String::Utf8Value stack_trace(m_vm->m_isolate, stack_trace_string);
			const char* err = ToCString(stack_trace);
			fprintf(stderr, "%s\n", err);
		}
	}
}

bool GameContext::_execute_string(v8::Local<v8::String> source, v8::Local<v8::Value> name, bool print_result, bool report_exceptions)
{
	v8::HandleScope handle_scope(m_vm->m_isolate);
	v8::TryCatch try_catch(m_vm->m_isolate);
	v8::ScriptOrigin origin(m_vm->m_isolate, name);
	v8::ScriptCompiler::Source script_source(source, origin);

	v8::Local<v8::Context> context(m_vm->m_isolate->GetCurrentContext());
	v8::Local<v8::Script> script;

	if (!v8::ScriptCompiler::Compile(context, &script_source).ToLocal(&script)) {
		// Print errors that happened during compilation.
		if (report_exceptions)
			_report_exception(&try_catch);
		return false;
	}
	else {
		v8::Local<v8::Value> result;
		if (!script->Run(context).ToLocal(&result)) {
			assert(try_catch.HasCaught());
			// Print errors that happened during execution.
			if (report_exceptions)
				_report_exception(&try_catch);
			return false;
		}
		else {
			assert(!try_catch.HasCaught());
			if (print_result && !result->IsUndefined()) {
				// If all went well and the result wasn't undefined then print
				// the returned value.
				v8::String::Utf8Value str(m_vm->m_isolate, result);
				const char* cstr = ToCString(str);
				printf("%s\n", cstr);
			}
			return true;
		}
	}
}

void GameContext::_run_script(const char* filename)
{
	FILE* fp = fopen(filename, "rb");
	fseek(fp, 0, SEEK_END);
	long length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	std::vector<char> str_buf((size_t)length + 1, 0);
	fread(str_buf.data(), 1, length, fp);
	fclose(fp);

	v8::HandleScope handle_scope(m_vm->m_isolate);
	v8::Local<v8::String> source = v8::String::NewFromUtf8(m_vm->m_isolate, str_buf.data()).ToLocalChecked();
	v8::Local<v8::String> file_name = v8::String::NewFromUtf8(m_vm->m_isolate, filename).ToLocalChecked();

	_execute_string(source, file_name, false, true);
}


void GameContext::Print(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	bool first = true;
	for (int i = 0; i < args.Length(); i++) {
		v8::HandleScope handle_scope(args.GetIsolate());
		if (first) {
			first = false;
		}
		else {
			printf(" ");
		}
		v8::String::Utf8Value str(args.GetIsolate(), args[i]);
		const char* cstr = ToCString(str);
		printf("%s", cstr);
	}
	printf("\n");
	fflush(stdout);
}

void GameContext::SetCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Isolate* isolate = args.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> global = context->Global();
	v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(global->GetInternalField(0));
	GameContext* self = (GameContext*)wrap->Value();

	v8::String::Utf8Value name(isolate, args[0]);
	v8::Local<v8::Function> callback = args[1].As<v8::Function>();
	self->m_callbacks[*name] = CallbackT(isolate, callback);
}


v8::Function* GameContext::GetCallback(const char* name)
{
	auto iter = m_callbacks.find(name);
	if (iter == m_callbacks.end()) return nullptr;
	return *iter->second.Get(m_vm->m_isolate);
}

v8::MaybeLocal<v8::Value> GameContext::InvokeCallback(v8::Function* callback, const std::vector<v8::Local<v8::Value>>& args)
{
	v8::Isolate* isolate = m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> global = context->Global();	
	return callback->Call(context, global, (int)args.size(), (v8::Local<v8::Value>*)args.data());
}

void GameContext::Now(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Isolate* isolate = args.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	double now = time_sec() * 1000.0;
	args.GetReturnValue().Set(v8::Number::New(isolate, now));
}