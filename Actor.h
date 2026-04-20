#ifndef ACTOR_H
#define ACTOR_H

#include <string>
#include <memory>
#include <map>
#include "Lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <iostream>
#include "Helper.h"
#include "RigidBody.h"
#include "Collider.h"
#include "Game.h"
#include "ParticleSystem.h"
class Actor {
private:
    int uid;
    std::string name;
    static int nAddComponent;
    

public:
    std::map<std::string, std::shared_ptr<luabridge::LuaRef>> components;
    std::map<std::string, std::shared_ptr<RigidBody>> rigidbodies;

    std::map<std::string, std::shared_ptr<RigidBody>> rigidbodies_to_add;
    std::vector<std::string> rigidbodies_to_start;

    std::map<std::string, std::shared_ptr<ParticleSystem>> particle_systems;
    std::vector<std::string> particle_systems_to_start;

    std::vector<std::string> components_to_remove;
    Actor(int uid, std::string name) : uid(uid), name(name) {}
    std::string GetName() { return name; }
    int GetID() { return uid; }

    void InjectConvenienceReferences(std::shared_ptr<luabridge::LuaRef> component_ref);

    luabridge::LuaRef GetComponentByKey(const std::string& key);
    luabridge::LuaRef GetComponent(const std::string& type_name);
    luabridge::LuaRef GetComponents(const std::string& type_name);

    lua_State* L_state = nullptr;

    luabridge::LuaRef AddComponent(const std::string& type_name);

    void RemoveComponent(luabridge::LuaRef component_ref);
    void InitComponents();
    std::vector<std::shared_ptr<luabridge::LuaRef>> components_to_add; 
    std::vector<std::shared_ptr<luabridge::LuaRef>> components_to_start;
    bool dont_destroy = false;
};

#endif