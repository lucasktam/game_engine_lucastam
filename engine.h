// In your binding file or a Physics.h

#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include "Helper.h"
#include "glm/glm/glm.hpp"
#include "Renderer.h"
#include "EngineUtils.h"
#include "Audio.h"
#include "Game.h"
#include "Input.h"
#include "Application.h"
#include <algorithm>
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/filereadstream.h"
#include "ComponentDB.h"
#include "box2d/include/box2d/box2d.h"
#include "Lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include "RaycastCallback.h"
#include "EventBus.h"


struct Physics {
    static luabridge::LuaRef Raycast(b2Vec2 pos, b2Vec2 dir, float dist) {
        if (dist <= 0 || !Game::physics_world) return luabridge::LuaRef(Game::get_lua_state());

        dir.Normalize();
        b2Vec2 end = pos + dist * dir;

        RaycastCallback cb;
        Game::physics_world->RayCast(&cb, pos, end);

        if (cb.hits.empty()) return luabridge::LuaRef(Game::get_lua_state());

        HitResult* closest = &cb.hits[0];
        float min_dist = (cb.hits[0].point - pos).Length();
        for (auto& h : cb.hits) {
            float d = (h.point - pos).Length();
            if (d < min_dist) { min_dist = d; closest = &h; }
        }

        return luabridge::LuaRef(Game::get_lua_state(), *closest);
    }

    static luabridge::LuaRef RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist) {
        luabridge::LuaRef table = luabridge::newTable(Game::get_lua_state());
        if (dist <= 0 || !Game::physics_world) return table;

        dir.Normalize();
        b2Vec2 end = pos + dist * dir;

        RaycastCallback cb;
        Game::physics_world->RayCast(&cb, pos, end);

        std::sort(cb.hits.begin(), cb.hits.end(), [&](const HitResult& a, const HitResult& b) {
            return (a.point - pos).LengthSquared() < (b.point - pos).LengthSquared();
        });

        for (int i = 0; i < (int)cb.hits.size(); i++) {
            table[i + 1] = cb.hits[i];
        }
        return table;
    }
};