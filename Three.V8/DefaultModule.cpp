#include <GL/glew.h>
#include <utils/Utils.h>
#include <utils/Logging.h>
#include "DefaultModule.h"
#include "binding.h"
#include "WrapperUtils.hpp"

inline const char* ToCString(const v8::String::Utf8Value& value) {
	return *value ? *value : "<string conversion failed>";
}


static void Print(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string line = "";
	bool first = true;
	for (int i = 0; i < args.Length(); i++) {
		v8::HandleScope handle_scope(args.GetIsolate());
		if (first) {
			first = false;
		}
		else
		{
			line += " ";
		}
		v8::String::Utf8Value str(args.GetIsolate(), args[i]);
		const char* cstr = ToCString(str);
		line += cstr;
	}
	
	Logging::print_std(line.c_str());

}


static void Now(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	LocalContext lctx(args);
	double now = time_sec() * 1000.0;
	args.GetReturnValue().Set(lctx.num_to_jnum(now));
}


static void GetGLError(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	LocalContext lctx(args);
	unsigned err = glGetError();
	args.GetReturnValue().Set(lctx.num_to_jnum(err));
}


#if THREE_MM
#include <MMCamera.h>
#include <MMPlayer.h>

static void GetListOfCameras(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	LocalContext lctx(args);
	const std::vector<std::string>& lst = MMCamera::s_get_list_devices();

	v8::Local<v8::Array> ret = v8::Array::New(lctx.isolate);
	for (size_t i = 0; i < lst.size(); i++)
	{
		v8::Local<v8::String> name = lctx.str_to_jstr(lst[i].c_str());
		ret->Set(lctx.context, (unsigned)i, name);
	}
	args.GetReturnValue().Set(ret);
}

static void GetListOfAudioDevices(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	LocalContext lctx(args);
	const std::vector<std::string>& lst = GetNamesAudioDevices(false);

	v8::Local<v8::Array> ret = v8::Array::New(lctx.isolate);
	for (size_t i = 0; i < lst.size(); i++)
	{
		v8::Local<v8::String> name = lctx.str_to_jstr(lst[i].c_str());
		ret->Set(lctx.context, (unsigned)i, name);
	}
	args.GetReturnValue().Set(ret);
}
#endif


#ifdef _WIN32
#include <Windows.h>
#include <commdlg.h>
#endif

static void GeneralCall(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	LocalContext lctx(args);
	v8::Local<v8::String> ret;
	std::string cmd = lctx.jstr_to_str(args[0]);

#ifdef _WIN32
	if (cmd == "OpenFile")
	{
		v8::String::Value filter_name(lctx.isolate, args[1]);
		v8::String::Value filter_ext(lctx.isolate, args[2]);

		wchar_t filter[1024];
		wsprintf(filter, L"%s", (wchar_t*)(*filter_name));

		int pos = lstrlenW(filter);
		wsprintf(filter + pos + 1, L"%s", (wchar_t*)(*filter_ext));

		int pos2 = lstrlenW(filter + pos + 1);
		filter[pos + 1 + pos2 + 1] = 0;


		wchar_t buffer[1024];
		OPENFILENAMEW ofn{ 0 };
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = buffer;
		ofn.lpstrFile[0] = 0;
		ofn.nMaxFile = 1024;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		::GetOpenFileName(&ofn);

		ret = v8::String::NewFromTwoByte(lctx.isolate, (uint16_t*)buffer).ToLocalChecked();
	}
#endif

	args.GetReturnValue().Set(ret);
}

#include "WrapperGamePlayer.h"
#include "gui/WrapperUIManager.h"
#include "network/WrapperHttpClient.h"
#include "loaders/WrapperFileLoader.h"
#include "loaders/WrapperImageLoader.h"
#include "loaders/WrapperHDRImageLoader.h"
#include "loaders/WrapperDDSImageLoader.h"
#include "loaders/WrapperGLTFLoader.h"
#include "loaders/WrapperProbeGridLoader.h"
#include "loaders/WrapperLODProbeGridLoader.h"
#include "savers/WrapperFileSaver.h"
#include "savers/WrapperImageSaver.h"
#include "savers/WrapperHDRImageSaver.h"
#include "savers/WrapperProbeGridSaver.h"
#include "savers/WrapperLODProbeGridSaver.h"
#include "volume/WrapperVolumeDataLoader.h"
#include "utils/WrapperText.h"

#include "core/WrapperObject3D.h"
#include "cameras/WrapperCamera.h"
#include "cameras/WrapperPerspectiveCamera.h"
#include "cameras/WrapperReflector.h"
#include "backgrounds/WrapperBackground.h"
#include "backgrounds/WrapperBackgroundScene.h"
#include "scenes/WrapperScene.h"
#include "scenes/WrapperFog.h"
#include "renderers/WrapperGLRenderer.h"
#include "renderers/WrapperGLRenderTarget.h"
#include "renderers/WrapperCubeRenderTarget.h"
#include "models/WrapperSimpleModel.h"
#include "models/WrapperGLTFModel.h"
#include "models/WrapperAnimationMixer.h"
#include "utils/WrapperImage.h"
#include "utils/WrapperHDRImage.h"
#include "utils/WrapperDDSImage.h"
#include "lights/WrapperDirectionalLight.h"
#include "lights/WrapperEnvironmentMap.h"
#include "lights/WrapperProbeGrid.h"
#include "lights/WrapperLODProbeGrid.h"
#include "lights/WrapperEnvironmentMapCreator.h"
#include "lights/WrapperAmbientLight.h"
#include "lights/WrapperHemisphereLight.h"
#include "lights/WrapperProbeGridWidget.h"
#include "lights/WrapperLODProbeGridWidget.h"
#include "core/WrapperBoundingVolumeHierarchy.h"
#include "gui/WrapperUIArea.h"
#include "gui/WrapperUIBlock.h"
#include "gui/WrapperUIPanel.h"
#include "gui/WrapperUIButton.h"
#include "gui/WrapperUIScrollViewer.h"
#include "gui/WrapperUILineEdit.h"
#include "gui/WrapperUIText.h"
#include "gui/WrapperUITextBlock.h"
#include "gui/WrapperUIImage.h"
#include "gui/WrapperUI3DViewer.h"
#include "gui/WrapperUIDraggable.h"

#if THREE_MM
#include "multimedia/WrapperMMCamera.h"
#include "multimedia/WrapperMMLazyVideo.h"
#include "multimedia/WrapperMMAudio.h"
#include "multimedia/WrapperMMVideo.h"
#include "multimedia/WrapperOpusRecorder.h"
#include "multimedia/WrapperOpusPlayer.h"
#include "multimedia/WrapperAVCRecorder.h"
#include "multimedia/WrapperAVCPlayer.h"
#endif

#include "volume/WrapperVolumeData.h"
#include "volume/WrapperVolumeIsosurfaceModel.h"
#include "network/WrapperWSClient.h"
#include "models/WrapperHeightField.h"

void GetDefaultModule(ModuleDefinition& module)
{
	module.functions = {
		{ "print", Print},
		{ "setCallback", GameContext::SetCallback},
		{ "now", Now},
		{"getGLError", GetGLError},
#if THREE_MM
		{"getListOfCameras", GetListOfCameras},
		{"getListOfAudioDevices", GetListOfAudioDevices},
#endif
		{"generalCall", GeneralCall},
	};

	module.objects = {
		WrapperGamePlayer::define,
		WrapperUIManager::define,
		WrapperHttpClient::define,
		WrapperFileLoader::define,
		WrapperImageLoader::define,
		WrapperHDRImageLoader::define,
		WrapperDDSImageLoader::define,
		WrapperGLTFLoader::define,
		WrapperProbeGridLoader::define,
		WrapperLODProbeGridLoader::define,
		WrapperFileSaver::define,
		WrapperImageSaver::define,
		WrapperHDRImageSaver::define,
		WrapperProbeGridSaver::define,
		WrapperLODProbeGridSaver::define,
		WrapperVolumeDataLoader::define,
		WrapperText::define
	};

	module.classes = {
		WrapperObject3D::define,
		WrapperCamera::define,
		WrapperPerspectiveCamera::define,
		WrapperReflector::define,
		WrapperColorBackground::define,
		WrapperCubeBackground::define,
		WrapperHemisphereBackground::define,
		WrapperBackgroundScene::define,
		WrapperScene::define,
		WrapperFog::define,
		WrapperGLRenderer::define,
		WrapperGLRenderTarget::define,
		WrapperCubeRenderTarget::define,
		WrapperSimpleModel::define,
		WrapperGLTFModel::define,
		WrapperAnimationMixer::define,
		WrapperImage::define,
		WrapperCubeImage::define,
		WrapperHDRImage::define,
		WrapperHDRCubeImage::define,
		WrapperDDSImage::define,
		WrapperDirectionalLight::define,
		WrapperEnvironmentMap::define,
		WrapperProbeGrid::define,
		WrapperLODProbeGrid::define,
		WrapperEnvironmentMapCreator::define,
		WrapperAmbientLight::define,
		WrapperHemisphereLight::define,
		WrapperProbeGridWidget::define,
		WrapperLODProbeGridWidget::define,
		WrapperBoundingVolumeHierarchy::define,
		WrapperUIArea::define,
		WrapperUIBlock::define,
		WrapperUIPanel::define,
		WrapperUIButton::define,
		WrapperUIScrollViewer::define,
		WrapperUILineEdit::define,
		WrapperUIText::define,
		WrapperUITextBlock::define,
		WrapperUIImage::define,
		WrapperUI3DViewer::define,
		WrapperUIDraggable::define,
#if THREE_MM
		WrapperMMCamera::define,
		WrapperMMLazyVideo::define,
		WrapperMMAudio::define,
		WrapperMMVideo::define,
		WrapperOpusRecorder::define,
		WrapperOpusPlayer::define,
		WrapperAVCRecorder::define,
		WrapperAVCPlayer::define,
#endif
		WrapperVolumeData::define,
		WrapperVolumeIsosurfaceModel::define,
		WrapperWSClient::define,
		WrapperHeightField::define

	};
}

