#ifndef GAME_H
#define GAME_H

#include "EngineUtils.h"
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/filereadstream.h"
#include "Input.h" 
#include <filesystem>
#include "glm/glm/glm.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include "Helper.h"
#include "Renderer.h"
#include <optional>

#include "Lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "RigidBody.h"
#include "box2d/include/box2d/box2d.h"
#include "Collider.h"

class Actor;

class Game {
public:
    static void init(lua_State* L, const rapidjson::Document &game_config, const rapidjson::Document &initial_scene_document);
    static void load_actors(const rapidjson::Document &initial_scene_document, int onstart_from_index=0); 

    static void update_all_actors();
    static void late_update_all_actors();
    static void update_actor_grid_position(Actor& a, glm::vec2 old_pos, glm::vec2 new_pos);
    static void move_npc(Actor& a);
    static void ProcessPendingActorChanges();
    static Actor* Find(std::string name);
    static luabridge::LuaRef FindAll(std::string name);
    static Actor* Instantiate(std::string actor_template_name);
    static void Destroy(Actor* a);

    static lua_State* get_lua_state() { return L;}
    

    // methods
    static void        SceneLoad(const std::string& scene_name);
    static std::string SceneGetCurrent();
    static void        SceneDontDestroy(Actor* actor);
    static void        ProcessSceneLoad();
    static b2World* physics_world;
    static std::unique_ptr<Collider> contact_listener;
    static bool rigidbody_ever_existed;
    static bool particle_system_ever_existed;
private: 
    // statics
    static std::string current_scene_name;
    static std::string pending_scene_name;
    static bool scene_load_pending;
    
    static int x_resolution;
    static int y_resolution;
    static std::deque<Actor*> actors;
    static bool is_scene_stale;

    static std::unordered_map<std::string, rapidjson::Document> template_cache;
    static lua_State* L;

    static void load_lua_file(const std::map<std::string, std::map<std::string, luabridge::LuaRef>> components, Actor*a);
    static luabridge::LuaRef extractValue(const rapidjson::Value& val);

    static std::vector<Actor*> actors_to_add;
    static std::vector<Actor*> actors_to_destroy;
    

    
    
};

#endif