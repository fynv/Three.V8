#pragma once

#include "definitions.hpp"

class GamePlayer;
class WrapperGamePlayer
{
public:
	static GamePlayer* s_game_player;
	static void define(ObjectDefinition& object);

private:
	static void* ctor();
	static void dtor(void* ptr, GameContext* ctx);

	static void GetWidth(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void GetHeight(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);

	static void Message(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void HasFont(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void CreateFontFromFile(const v8::FunctionCallbackInfo<v8::Value>& info);
	static void CreateFontFromMemory(const v8::FunctionCallbackInfo<v8::Value>& info);

	static void GetPicking(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
	static void SetPicking(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info);

	static void PickObject(const v8::FunctionCallbackInfo<v8::Value>& info);
};

