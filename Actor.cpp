#include "Actor.h"

int Actor::nAddComponent = 0;
namespace fs = std::filesystem;

void Actor::InjectConvenienceReferences(std::shared_ptr<luabridge::LuaRef> component_ref) {
    (*component_ref)["actor"] = this;

    luabridge::LuaRef keyVal = (*component_ref)["key"];
    if (keyVal.isString()) {
        std::string key = keyVal.cast<std::string>();
        components[key] = component_ref;
    }
}

luabridge::LuaRef Actor::GetComponentByKey(const std::string& key) {
    auto it = components.find(key);
    if (it != components.end())
        return *(it->second);

    auto rb_it = rigidbodies.find(key);
    if (rb_it != rigidbodies.end())
        return luabridge::LuaRef(L_state, rb_it->second.get());

    auto ps_it = particle_systems.find(key);
    if (ps_it != particle_systems.end())
        return luabridge::LuaRef(L_state, ps_it->second.get());

    return luabridge::LuaRef(L_state);
}

luabridge::LuaRef Actor::GetComponent(const std::string& type_name) {
    if (type_name == "Rigidbody") {
        for (auto& [key, rb] : rigidbodies) {
            bool pending = false;
            for (const std::string& c : components_to_remove)
                if (c == key) { pending = true; break; }
            if (!pending)
                return luabridge::LuaRef(L_state, rb.get());
        }
        return luabridge::LuaRef(L_state);
    }

    if (type_name == "ParticleSystem") {
        for (auto& [key, ps] : particle_systems) {
            bool pending = false;
            for (const std::string& c : components_to_remove)
                if (c == key) { pending = true; break; }
            if (!pending)
                return luabridge::LuaRef(L_state, ps.get());
        }
        return luabridge::LuaRef(L_state);
    }

    for (auto& [key, ref] : components) {
        luabridge::LuaRef typeVal = (*ref)["type"];
        if (!typeVal.isString() || typeVal.cast<std::string>() != type_name)
            continue;

        bool pending = false;
        for (const std::string& c : components_to_remove)
            if (c == key) { pending = true; break; }
        if (!pending)
            return *ref;
    }
    return luabridge::LuaRef(L_state);
}

luabridge::LuaRef Actor::GetComponents(const std::string& type_name) {
    luabridge::LuaRef table = luabridge::newTable(L_state);
    int idx = 1;

    if (type_name == "Rigidbody") {
        for (auto& [key, rb] : rigidbodies) {
            bool pending = false;
            for (const std::string& c : components_to_remove)
                if (c == key) { pending = true; break; }
            if (!pending)
                table[idx++] = luabridge::LuaRef(L_state, rb.get());
        }
        return table;
    }

    if (type_name == "ParticleSystem") {
        for (auto& [key, ps] : particle_systems) {
            bool pending = false;
            for (const std::string& c : components_to_remove)
                if (c == key) { pending = true; break; }
            if (!pending)
                table[idx++] = luabridge::LuaRef(L_state, ps.get());
        }
        return table;
    }

    for (auto& [key, ref] : components) {
        luabridge::LuaRef typeVal = (*ref)["type"];
        if (!typeVal.isString() || typeVal.cast<std::string>() != type_name)
            continue;

        bool pending = false;
        for (const std::string& c : components_to_remove)
            if (c == key) { pending = true; break; }
        if (!pending)
            table[idx++] = *ref;
    }
    return table;
}

luabridge::LuaRef Actor::AddComponent(const std::string& type_name) {
    if (type_name == "Rigidbody") {
        Game::rigidbody_ever_existed = true;
        if (!Game::physics_world) {
            b2Vec2 gravity(0.0f, 9.8f);
            Game::physics_world = new b2World(gravity);
            Game::contact_listener = std::make_unique<Collider>();
            Game::physics_world->SetContactListener(Game::contact_listener.get());
        }

        auto rb = std::make_shared<RigidBody>();
        std::string key = "r" + std::to_string(nAddComponent++);
        rb->key = key;
        rigidbodies_to_add[key] = rb;
        return luabridge::LuaRef(L_state, rb.get());
    }

    if (type_name == "ParticleSystem") {
        Game::particle_system_ever_existed = true;

        auto ps = std::make_shared<ParticleSystem>();
        std::string key = "r" + std::to_string(nAddComponent++);
        ps->key   = key;
        ps->actor = this;

        particle_systems[key] = ps;
        particle_systems_to_start.push_back(key);
        return luabridge::LuaRef(L_state, ps.get());
    }

    fs::path lua_path = std::string("resources/component_types/") + type_name + ".lua";
    if (!fs::exists(lua_path)) {
        std::cout << "error: failed to locate component " << type_name;
        exit(0);
    }
    if (luaL_dofile(L_state, lua_path.string().c_str()) != LUA_OK) {
        std::cout << std::string("problem with lua file ") + type_name;
        exit(0);
    }

    luabridge::LuaRef base = luabridge::getGlobal(L_state, type_name.c_str());
    luabridge::LuaRef instance = luabridge::newTable(L_state);

    std::string key = "r" + std::to_string(nAddComponent++);
    instance["key"] = key;
    instance["enabled"] = true;
    instance["type"] = type_name;
    luabridge::LuaRef metatable = luabridge::newTable(L_state);
    metatable["__index"] = base;

    instance.push(L_state);
    metatable.push(L_state);
    lua_setmetatable(L_state, -2);
    lua_pop(L_state, 1);

    auto component_ref = std::make_shared<luabridge::LuaRef>(instance);
    components_to_add.push_back(component_ref);
    return instance;
}

void Actor::RemoveComponent(luabridge::LuaRef component_ref) {
    if (component_ref.isUserdata()) {
        RigidBody* rb = component_ref.cast<RigidBody*>();
        if (rb) {
            for (auto& [key, rbptr] : rigidbodies) {
                if (rbptr.get() == rb) {
                    components_to_remove.push_back(key);
                    return;
                }
            }
            std::cout << "rigidbody not found in map!" << std::endl;
            return;
        }
        
        ParticleSystem* ps = component_ref.cast<ParticleSystem*>();
        if (ps) {
            for (auto& [key, psptr] : particle_systems) {
                if (psptr.get() == ps) {
                    psptr->enabled = false;
                    components_to_remove.push_back(key);
                    return;
                }
            }
            std::cout << "particlesystem not found in map!" << std::endl;
            return;
        }
    }

    if (!component_ref.isTable()) {
        std::cout << "RemoveComponent: component_ref is not a table or userdata" << std::endl;
        return;
    }

    luabridge::LuaRef keyVal = component_ref["key"];
    if (!keyVal.isString()) {
        std::cout << "RemoveComponent: component has no valid key field" << std::endl;
        return;
    }

    component_ref["enabled"] = false;
    std::string key = keyVal.cast<std::string>();
    components_to_remove.push_back(key);
}

void Actor::InitComponents() {
    for (auto& [key, rb] : rigidbodies_to_add) {
        rigidbodies[key] = rb;
        rigidbodies_to_start.push_back(key);
    }
    rigidbodies_to_add.clear();

    if (components_to_add.size() > 0) {
        for (auto c : components_to_add) {
            InjectConvenienceReferences(c);
            components_to_start.push_back(c);
        }
    }
    components_to_add.clear();
}