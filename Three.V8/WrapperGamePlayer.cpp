
#include "WrapperUtils.hpp"
#include "GamePlayer.h"
#include "models/SimpleModel.h"
#include "models/GLTFModel.h"

#include "WrapperGamePlayer.h"

GamePlayer* WrapperGamePlayer::s_game_player = nullptr;

void WrapperGamePlayer::define(ObjectDefinition& object)
{
	object.name = "gamePlayer";
	object.ctor = ctor;
	object.dtor = dtor;
	object.properties = {
		{ "width", GetWidth },
		{ "height", GetHeight },
		{ "picking", GetPicking, SetPicking }
	};

	object.methods = {
		{ "message", Message },
		{ "hasFont", HasFont },
		{ "createFontFromFile", CreateFontFromFile },
		{ "createFontFromMemory", CreateFontFromMemory },
		{ "pickObject", PickObject },
	};

}

void* WrapperGamePlayer::ctor()
{
	return s_game_player;
}

void WrapperGamePlayer::dtor(void* ptr, GameContext* ctx)
{
	
}


void WrapperGamePlayer::GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->width()));
}

void WrapperGamePlayer::GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	info.GetReturnValue().Set(lctx.num_to_jnum(self->height()));
}

void WrapperGamePlayer::Message(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	std::string name = lctx.jstr_to_str(info[0]);
	std::string msg = lctx.jstr_to_str(info[1]);
	std::string res = self->UserMessage(name.c_str(), msg.c_str());
	info.GetReturnValue().Set(lctx.str_to_jstr(res.c_str()));
}

void WrapperGamePlayer::HasFont(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	std::string name = lctx.jstr_to_str(info[0]);
	bool has_font = self->UIRenderer().HasFont(name.c_str());
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, has_font));
}

void WrapperGamePlayer::CreateFontFromFile(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	std::string name = lctx.jstr_to_str(info[0]);
	std::string filename = lctx.jstr_to_str(info[1]);
	self->UIRenderer().CreateFont(name.c_str(), filename.c_str());
}

void WrapperGamePlayer::CreateFontFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	std::string name = lctx.jstr_to_str(info[0]);
	v8::Local<v8::ArrayBuffer> data = info[1].As<v8::ArrayBuffer>();
	self->UIRenderer().CreateFont(name.c_str(), (unsigned char*)data->GetBackingStore()->Data(), data->ByteLength());
}

void WrapperGamePlayer::GetPicking(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	bool picking = self->Picking();
	info.GetReturnValue().Set(v8::Boolean::New(lctx.isolate, picking));
}

void WrapperGamePlayer::SetPicking(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	bool picking = value.As<v8::Boolean>()->Value();
	self->SetPicking(picking);
}

void WrapperGamePlayer::PickObject(const v8::FunctionCallbackInfo<v8::Value>& info)
{
	LocalContext lctx(info);
	GamePlayer* self = lctx.self<GamePlayer>();
	GLPickingTarget* target = self->pickingTarget();

	int x, y;
	lctx.jnum_to_num(info[0], x);
	lctx.jnum_to_num(info[1], y);

	const GLPickingTarget::IdxInfo& idxInfo = target->pick_obj(x, y);
	if (idxInfo.obj != nullptr)
	{
		v8::Local<v8::Object> ret = v8::Object::New(lctx.isolate);
		v8::Local<v8::String> name = lctx.str_to_jstr(idxInfo.obj->name.c_str());
		v8::Local<v8::String> uuid = lctx.str_to_jstr(idxInfo.obj->uuid.c_str());
		lctx.set_property(ret, "name", name);
		lctx.set_property(ret, "uuid", uuid);

		if (idxInfo.primitive_idx == 0)
		{
			lctx.set_property(ret, "mesh_id", lctx.num_to_jnum(0));
			lctx.set_property(ret, "prim_id", lctx.num_to_jnum(0));
		}
		else
		{
			GLTFModel* model = (GLTFModel*)idxInfo.obj;
			int prim_idx = 0;
			for (size_t i = 0; i < model->m_meshs.size(); i++)
			{
				Mesh& mesh = model->m_meshs[i];
				for (size_t j = 0; j < mesh.primitives.size(); j++, prim_idx++)
				{
					if (prim_idx == idxInfo.primitive_idx)
					{
						lctx.set_property(ret, "mesh_id", lctx.num_to_jnum(i));
						lctx.set_property(ret, "prim_id", lctx.num_to_jnum(j));
						break;
					}
				}
				if (prim_idx == idxInfo.primitive_idx) break;
			}
		}

		info.GetReturnValue().Set(ret);
	}
	else
	{
		info.GetReturnValue().SetNull();
	}

}

