#include <assert.h>
#include <cstdio>
#include <utils/Logging.h>
#include "binding.h"
#include "WrapperUtils.hpp"

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
	m_isolate->Enter();
}

V8VM::~V8VM()
{	
	m_isolate->Exit();
	m_isolate->Dispose();
	v8::V8::Dispose();
	v8::V8::DisposePlatform();
	delete m_array_buffer_allocator;
}

GameContext::GameContext(V8VM* vm, const WorldDefinition& world_definition, const char* filename): m_vm(vm)
{
	_create_context(world_definition);
	v8::HandleScope handle_scope(m_vm->m_isolate);
	v8::Context::Scope context_scope(m_context.Get(m_vm->m_isolate));
	_run_script(filename);
}

GameContext::~GameContext()
{
	auto iter = m_objects.begin();
	while (iter != m_objects.end())
	{
		Dtor dtor = iter->second.second;
		dtor(iter->first, this);
		iter->second.first.Reset();
		iter++;
	}
}

void GameContext::GeneralNew(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	int idx_ctor_dtor;
	lctx.jnum_to_num(info.Data(), idx_ctor_dtor);	
	CtorDtor ctor_dtor = lctx.ctx()->m_cls_ctor_dtor[idx_ctor_dtor];
	void* ptr = ctor_dtor.ctor(info);	
	info.This()->SetAlignedPointerInInternalField(0, ptr);
	lctx.ctx()->regiter_object(info.This(), ctor_dtor.dtor);
}

void GameContext::GeneralDispose(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GameContext* ctx = lctx.ctx();
	void* self = lctx.get_self();
	ctx->remove_object(self);
}

void GameContext::_create_context(const WorldDefinition& world_definition)
{
	v8::Isolate* isolate = m_vm->m_isolate;
	v8::HandleScope handle_scope(isolate);

	v8::Local<v8::ObjectTemplate> global_tmpl = v8::ObjectTemplate::New(isolate);
	global_tmpl->SetInternalFieldCount(1);	

	const ModuleDefinition& default_module = world_definition.default_module;
	
	// functions
	const std::vector<FunctionDefinition>& functions = default_module.functions;
	for (size_t i = 0; i < functions.size(); i++)
	{
		const FunctionDefinition& func = functions[i];
		global_tmpl->Set(isolate, func.name.c_str(), v8::FunctionTemplate::New(isolate, func.func));
	}

	// classes
	const std::vector<ClassDefiner>& classes = default_module.classes;
	for (size_t i = 0; i < classes.size(); i++)
	{
		const ClassDefiner& cls_define = classes[i];
		ClassDefinition def;
		cls_define(def);

		int idx_ctor_dtor = (int)m_cls_ctor_dtor.size();
		m_cls_ctor_dtor.push_back({ def.ctor, def.dtor });
		v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, GeneralNew, v8::Number::New(isolate, idx_ctor_dtor));
		templ->InstanceTemplate()->SetInternalFieldCount(2);
		templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));

		for (size_t j = 0; j < def.properties.size(); j++)
		{
			const AccessorDefinition& property = def.properties[j];
			templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, property.name.c_str()).ToLocalChecked(), property.get, property.set);
		}

		for (size_t j = 0; j < def.methods.size(); j++)
		{
			const FunctionDefinition& method = def.methods[j];
			templ->InstanceTemplate()->Set(isolate, method.name.c_str(), v8::FunctionTemplate::New(isolate, method.func));
		}

		global_tmpl->Set(isolate, def.name.c_str(), templ);	
	}

	v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global_tmpl);
	v8::Local<v8::Object> global_obj = context->Global();
	global_obj->SetAlignedPointerInInternalField(0, this);

	// global objects
	const std::vector<ObjectDefiner>& objects = default_module.objects;
	for (size_t i = 0; i < objects.size(); i++)
	{
		const ObjectDefiner& obj_define = objects[i];
		ObjectDefinition def;
		obj_define(def);

		v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
		if (def.ctor != nullptr)
		{
			templ->SetInternalFieldCount(2);
		}

		for (size_t j = 0; j < def.properties.size(); j++)
		{
			const AccessorDefinition& property = def.properties[j];
			templ->SetAccessor(v8::String::NewFromUtf8(isolate, property.name.c_str()).ToLocalChecked(), property.get, property.set);
		}

		for (size_t j = 0; j < def.methods.size(); j++)
		{
			const FunctionDefinition& method = def.methods[j];
			templ->Set(isolate, method.name.c_str(), v8::FunctionTemplate::New(isolate, method.func));
		}

		v8::Local<v8::Object> obj = templ->NewInstance(context).ToLocalChecked();
		if (def.ctor != nullptr)
		{
			obj->SetAlignedPointerInInternalField(0, def.ctor());
			regiter_object(obj, def.dtor);
		}				
		global_obj->Set(context, v8::String::NewFromUtf8(isolate, def.name.c_str()).ToLocalChecked(), obj);
	}

	// additional modules
	const std::vector<ModuleDefinition>& modules = world_definition.modules;
	for (size_t m = 0; m < modules.size(); m++)
	{
		const ModuleDefinition& mod = modules[m];
		
		v8::Local<v8::ObjectTemplate> mod_tmpl = v8::ObjectTemplate::New(isolate);

		// functions
		const std::vector<FunctionDefinition>& functions = mod.functions;
		for (size_t i = 0; i < functions.size(); i++)
		{
			const FunctionDefinition& func = functions[i];
			mod_tmpl->Set(isolate, func.name.c_str(), v8::FunctionTemplate::New(isolate, func.func));
		}

		// classes
		const std::vector<ClassDefiner>& classes = mod.classes;
		for (size_t i = 0; i < classes.size(); i++)
		{
			const ClassDefiner& cls_define = classes[i];
			ClassDefinition def;
			cls_define(def);

			int idx_ctor_dtor = (int)m_cls_ctor_dtor.size();
			m_cls_ctor_dtor.push_back({ def.ctor, def.dtor });
			v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, GeneralNew, v8::Number::New(isolate, idx_ctor_dtor));
			templ->InstanceTemplate()->SetInternalFieldCount(2);
			templ->InstanceTemplate()->Set(isolate, "dispose", v8::FunctionTemplate::New(isolate, GeneralDispose));

			for (size_t j = 0; j < def.properties.size(); j++)
			{
				const AccessorDefinition& property = def.properties[j];
				templ->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, property.name.c_str()).ToLocalChecked(), property.get, property.set);
			}

			for (size_t j = 0; j < def.methods.size(); j++)
			{
				const FunctionDefinition& method = def.methods[j];
				templ->InstanceTemplate()->Set(isolate, method.name.c_str(), v8::FunctionTemplate::New(isolate, method.func));
			}

			mod_tmpl->Set(isolate, def.name.c_str(), templ);
		}

		v8::Local<v8::Object> obj_mod = mod_tmpl->NewInstance(context).ToLocalChecked();

		// objects
		const std::vector<ObjectDefiner>& objects = mod.objects;
		for (size_t i = 0; i < objects.size(); i++)
		{
			const ObjectDefiner& obj_define = objects[i];
			ObjectDefinition def;
			obj_define(def);

			v8::Local<v8::ObjectTemplate> templ = v8::ObjectTemplate::New(isolate);
			if (def.ctor != nullptr)
			{
				templ->SetInternalFieldCount(2);
			}

			for (size_t j = 0; j < def.properties.size(); j++)
			{
				const AccessorDefinition& property = def.properties[j];
				templ->SetAccessor(v8::String::NewFromUtf8(isolate, property.name.c_str()).ToLocalChecked(), property.get, property.set);
			}

			for (size_t j = 0; j < def.methods.size(); j++)
			{
				const FunctionDefinition& method = def.methods[j];
				templ->Set(isolate, method.name.c_str(), v8::FunctionTemplate::New(isolate, method.func));
			}

			v8::Local<v8::Object> obj = templ->NewInstance(context).ToLocalChecked();
			if (def.ctor != nullptr)
			{
				obj->SetAlignedPointerInInternalField(0, def.ctor());
				regiter_object(obj, def.dtor);
			}
			obj_mod->Set(context, v8::String::NewFromUtf8(isolate, def.name.c_str()).ToLocalChecked(), obj);
		}

		global_obj->Set(context, v8::String::NewFromUtf8(isolate, mod.name.c_str()).ToLocalChecked(), obj_mod);
	}

	//context->AllowCodeGenerationFromStrings(false);

	m_context = ContextT(isolate, context);
}

// Extracts a C string from a V8 Utf8Value.
inline const char* ToCString(const v8::String::Utf8Value& value) {
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
		Logging::print_err(exception_string);
	}
	else {
		// Print (filename):(line number): (message).
		v8::String::Utf8Value filename(m_vm->m_isolate,
			message->GetScriptOrigin().ResourceName());
		v8::Local<v8::Context> context(m_vm->m_isolate->GetCurrentContext());
		const char* filename_string = ToCString(filename);
		int linenum = message->GetLineNumber(context).FromJust();
		{
			char line[1024];
			sprintf(line, "%s:%i: %s", filename_string, linenum, exception_string);
			Logging::print_err(line);
		}
		// Print line of source code.
		v8::String::Utf8Value sourceline(
			m_vm->m_isolate, message->GetSourceLine(context).ToLocalChecked());
		const char* sourceline_string = ToCString(sourceline);
		Logging::print_err(sourceline_string);

		{
			std::string line = "";
			// Print wavy underline (GetUnderline is deprecated).
			int start = message->GetStartColumn(context).FromJust();
			for (int i = 0; i < start; i++) 
			{
				line += " ";
			}
			int end = message->GetEndColumn(context).FromJust();
			for (int i = start; i < end; i++) 
			{
				line += "^";
			}
			Logging::print_err(line.c_str());
		}
		v8::Local<v8::Value> stack_trace_string;
		if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) &&
			stack_trace_string->IsString() &&
			stack_trace_string.As<v8::String>()->Length() > 0) {
			v8::String::Utf8Value stack_trace(m_vm->m_isolate, stack_trace_string);
			const char* err = ToCString(stack_trace);
			Logging::print_err(err);
		}
	}
}

bool GameContext::_execute_string(v8::Local<v8::String> source, v8::Local<v8::Value> name, bool print_result, bool report_exceptions)
{
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
				Logging::print_std(cstr);
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

void GameContext::SetCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{	
	LocalContext lctx(args);
	GameContext* self = lctx.ctx();
	std::string name = lctx.jstr_to_str(args[0]);
	v8::Local<v8::Function> callback = args[1].As<v8::Function>();	
	self->m_callbacks[name] = CallbackT(lctx.isolate, callback);
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
	v8::TryCatch try_catch(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::Object> global = context->Global();	
	v8::Local<v8::Value> ret;
	if (!callback->Call(context, global, (int)args.size(), (v8::Local<v8::Value>*)args.data()).ToLocal(&ret))	
	{		
		_report_exception(&try_catch);		
	}
	return ret;
}

void GameContext::WeakCallback(v8::WeakCallbackInfo<GameContext> const& data)
{
	void* self = data.GetInternalField(0);	
	GameContext* ctx = (GameContext *)data.GetInternalField(1);	
	ctx->remove_object(self);
}

void GameContext::regiter_object(v8::Local<v8::Object> obj, Dtor dtor)
{
	obj->SetAlignedPointerInInternalField(1, this);	
	void* self = obj->GetAlignedPointerFromInternalField(0);
	ObjectT pptr(m_vm->m_isolate, obj);
	pptr.SetWeak(this, WeakCallback, v8::WeakCallbackType::kInternalFields);
	m_objects.emplace(self, std::pair<ObjectT, Dtor>(std::move(pptr), dtor));
}

void GameContext::remove_object(void* ptr)
{
	Dtor dtor = m_objects[ptr].second;
	dtor(ptr, this);

	m_objects[ptr].first.Reset();
	m_objects.erase(ptr);
}
