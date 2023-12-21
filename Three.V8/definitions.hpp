#pragma once

#include <v8.h>
#include <string>
#include <vector>

class GameContext;

struct FunctionDefinition
{
	std::string name;
	v8::FunctionCallback func;
};

struct AccessorDefinition
{
	std::string name;
	v8::AccessorGetterCallback get = nullptr;
	v8::AccessorSetterCallback set = nullptr;
};

typedef void* (*Constructor)(const v8::FunctionCallbackInfo<v8::Value>& info);
typedef void* (*ObjConstructor)();
typedef void (*Destructor)(void* ptr, GameContext* ctx);


struct ObjectDefinition
{
	std::string name;
	ObjConstructor ctor = nullptr;
	Destructor dtor = nullptr;
	std::vector<FunctionDefinition> methods;
	std::vector<AccessorDefinition> properties;
};

struct ClassDefinition
{
	std::string name;
	Constructor ctor;
	Destructor dtor;
	std::vector<FunctionDefinition> methods;
	std::vector<AccessorDefinition> properties;
};

typedef void (*ObjectDefiner)(ObjectDefinition&);
typedef void (*ClassDefiner)(ClassDefinition&);

struct ModuleDefinition
{
	std::string name;
	std::vector<FunctionDefinition> functions;
	std::vector<ObjectDefiner> objects;
	std::vector<ClassDefiner> classes;
};

struct WorldDefinition
{
	ModuleDefinition default_module;
	std::vector<ModuleDefinition> modules;
};


