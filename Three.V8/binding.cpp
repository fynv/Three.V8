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


#include "utils/Image.hpp"

#include "core/Object3D.hpp"

#include "cameras/Camera.hpp"
#include "cameras/PerspectiveCamera.hpp"

#include "backgrounds/Background.hpp"
#include "lights/DirectionalLight.hpp"
#include "lights/EnvironmentMap.hpp"
#include "lights/EnvironmentMapCreator.hpp"
#include "lights/AmbientLight.hpp"
#include "lights/HemisphereLight.hpp"

#include "models/SimpleModel.hpp"
#include "models/GLTFModel.hpp"

#include "scenes/Scene.hpp"

#include "renderers/GLRenderer.hpp"
#include "renderers/GLRenderTarget.hpp"
#include "renderers/CubeRenderTarget.hpp"

#include "core/BoundingVolumeHierarchy.hpp"

#include "GamePlayer.hpp"
#include "network/HttpClient.hpp"
#include "loaders/FileLoader.hpp"
#include "loaders/ImageLoader.hpp"
#include "loaders/GLTFLoader.hpp"
#include "savers/ImageSaver.hpp"

#include "gui/UIManager.hpp"
#include "gui/UIArea.hpp"
#include "gui/UIBlock.hpp"
#include "gui/UIPanel.hpp"
#include "gui/UIButton.hpp"
#include "gui/UIScrollViewer.hpp"
#include "gui/UILineEdit.hpp"
#include "gui/UIText.hpp"
#include "gui/UITextBlock.hpp"
#include "gui/UIImage.hpp"
#include "gui/UI3DViewer.hpp"


GlobalDefinitions GameContext::s_globals =
{
	{
		{"print", GameContext::Print},
		{"setCallback", GameContext::SetCallback},
		{"now", GameContext::Now},
		{"getGLError", GameContext::GetGLError}
	},
	{
		{ "Object3D", WrapperObject3D::New,  WrapperObject3D::create_template },
		{ "Camera", WrapperCamera::New,  WrapperCamera::create_template },
		{ "PerspectiveCamera", WrapperPerspectiveCamera::New,  WrapperPerspectiveCamera::create_template },
		{ "ColorBackground", WrapperColorBackground::New,  WrapperColorBackground::create_template },
		{ "CubeBackground", WrapperCubeBackground::New,  WrapperCubeBackground::create_template },
		{ "HemisphereBackground", WrapperHemisphereBackground::New,  WrapperHemisphereBackground::create_template },
		{ "Scene", WrapperScene::New,  WrapperScene::create_template },
		{ "GLRenderer", WrapperGLRenderer::New,  WrapperGLRenderer::create_template },
		{ "GLRenderTarget", WrapperGLRenderTarget::New,  WrapperGLRenderTarget::create_template },
		{ "CubeRenderTarget", WrapperCubeRenderTarget::New,  WrapperCubeRenderTarget::create_template },
		{ "SimpleModel", WrapperSimpleModel::New,  WrapperSimpleModel::create_template },
		{ "GLTFModel", WrapperGLTFModel::New,  WrapperGLTFModel::create_template },
		{ "Image", WrapperImage::New,  WrapperImage::create_template },
		{ "CubeImage", WrapperCubeImage::New,  WrapperCubeImage::create_template },
		{ "DirectionalLight", WrapperDirectionalLight::New, WrapperDirectionalLight::create_template },
		{ "EnvironmentMap", WrapperEnvironmentMap::New, WrapperEnvironmentMap::create_template },
		{ "EnvironmentMapCreator", WrapperEnvironmentMapCreator::New, WrapperEnvironmentMapCreator::create_template},
		{ "AmbientLight", WrapperAmbientLight::New, WrapperAmbientLight::create_template},
		{ "HemisphereLight", WrapperHemisphereLight::New, WrapperHemisphereLight::create_template},
		{ "BoundingVolumeHierarchy", WrappeBoundingVolumeHierarchy::New, WrappeBoundingVolumeHierarchy::create_template},

		{ "UIArea", WrapperUIArea::New, WrapperUIArea::create_template},
		{ "UIBlock", WrapperUIBlock::New, WrapperUIBlock::create_template},
		{ "UIPanel", WrapperUIPanel::New, WrapperUIPanel::create_template},
		{ "UIButton", WrapperUIButton::New, WrapperUIButton::create_template},
		{ "UIScrollViewer", WrapperUIScrollViewer::New, WrapperUIScrollViewer::create_template},
		{ "UILineEdit", WrapperUILineEdit::New, WrapperUILineEdit::create_template},
		{ "UIText", WrapperUIText::New, WrapperUIText::create_template},
		{ "UITextBlock", WrapperUITextBlock::New, WrapperUITextBlock::create_template},
		{ "UIImage", WrapperUIImage::New, WrapperUIImage::create_template},
		{ "UI3DViewer", WrapperUI3DViewer::New, WrapperUI3DViewer::create_template},
	},
	{
		{ "fileLoader", WrapperFileLoader::create_template },
		{ "imageLoader", WrapperImageLoader::create_template},
		{ "gltfLoader", WrapperGLTFLoader::create_template},	
		{ "imageSaver", WrapperImageSaver::create_template},
	}
};

GameContext::GameContext(V8VM* vm, GamePlayer* gamePlayer, const char* filename)
	: m_vm(vm)
	, m_gamePlayer(gamePlayer)
	, m_http(new HttpClient)
	, m_ui_manager(new UIManager)
{
	_create_context();
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

void GameContext::_create_context()
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
	global_obj->SetAlignedPointerInInternalField(0, this);

	// gamePlayer
	{
		v8::Local<v8::ObjectTemplate> templ = WrapperGamePlayer::create_template(isolate);
		v8::Local<v8::Object> obj = templ->NewInstance(context).ToLocalChecked();
		obj->SetAlignedPointerInInternalField(0, m_gamePlayer);
		global_obj->Set(context, v8::String::NewFromUtf8(isolate, "gamePlayer").ToLocalChecked(), obj);
	}

	// http
	{
		v8::Local<v8::ObjectTemplate> templ = WrapperHttpClient::create_template(isolate);
		v8::Local<v8::Object> obj = templ->NewInstance(context).ToLocalChecked();
		obj->SetAlignedPointerInInternalField(0, m_http.get());
		global_obj->Set(context, v8::String::NewFromUtf8(isolate, "http").ToLocalChecked(), obj);
	}

	// ui-manager
	{
		v8::Local<v8::ObjectTemplate> templ = WrapperUIManager::create_template(isolate);
		v8::Local<v8::Object> obj = templ->NewInstance(context).ToLocalChecked();
		obj->SetAlignedPointerInInternalField(0, m_ui_manager.get());
		global_obj->Set(context, v8::String::NewFromUtf8(isolate, "UIManager").ToLocalChecked(), obj);
	}
	
	for (size_t i = 0; i < s_globals.objects.size(); i++)
	{
		const ObjectDefinition& d_obj = s_globals.objects[i];
		v8::Local<v8::ObjectTemplate> templ = d_obj.creator(isolate);
		v8::Local<v8::Object> obj = templ->NewInstance(context).ToLocalChecked();
		global_obj->Set(context, v8::String::NewFromUtf8(isolate, d_obj.name.c_str()).ToLocalChecked(), obj);
	}

	//context->AllowCodeGenerationFromStrings(false);

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
	GameContext* self = get_context(args);
	v8::Isolate* isolate = args.GetIsolate();
	v8::HandleScope handle_scope(isolate);
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

#include <GL/glew.h>

void GameContext::GetGLError(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Isolate* isolate = args.GetIsolate();
	v8::HandleScope handle_scope(isolate);
	unsigned err = glGetError();
	args.GetReturnValue().Set(v8::Number::New(isolate, (double)err));
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

void GameContext::CheckPendings()
{
	m_http->CheckPendings();
}