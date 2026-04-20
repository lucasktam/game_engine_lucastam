#include "Game.h"
#include "Actor.h"
namespace fs = std::filesystem;

int Game::x_resolution = Renderer::get_x_resolution(); 
int Game::y_resolution = Renderer::get_y_resolution();
bool Game::is_scene_stale = false; 
std::deque<Actor*> Game::actors; 

std::unordered_map<std::string, rapidjson::Document> Game::template_cache;
lua_State* Game::L;
std::vector<Actor*> Game::actors_to_add;
std::vector<Actor*> Game::actors_to_destroy;

std::string Game::current_scene_name = "";
std::string Game::pending_scene_name = "";
bool Game::scene_load_pending = false;

b2World* Game::physics_world = nullptr;
bool Game::rigidbody_ever_existed = false;
bool Game::particle_system_ever_existed = false;

std::unique_ptr<Collider> Game::contact_listener = nullptr;

void ReportError(const std::string & actor_name, const luabridge::LuaException & e){
    std::string error_message = e.what();

    std::replace(error_message.begin(), error_message.end(), '\\', '/');

    std::cout << "\033[31m" << actor_name << " : " << error_message << "\033[0m" << std::endl;
}

void Game::init(lua_State* L_, const rapidjson::Document &game_config, const rapidjson::Document &initial_scene_document){
    L = L_;
    current_scene_name = game_config["initial_scene"].GetString();
    load_actors(initial_scene_document);
    ProcessPendingActorChanges(); 
}


void Game::load_lua_file(const std::map<std::string, std::map<std::string, luabridge::LuaRef>> components, Actor*a){
    // Pass 1: instantiate all components and store them
    a->L_state = L;
    for (auto &p : components){
        
        std::string type = p.second.at("type").cast<std::string>();

        if (type == "Rigidbody") {
            rigidbody_ever_existed = true;

            // Lazy-init the b2World on first Rigidbody seen
            if (!physics_world) {
                b2Vec2 gravity(0.0f, 9.8f);
                physics_world = new b2World(gravity);
                contact_listener = std::make_unique<Collider>();
                physics_world->SetContactListener(contact_listener.get());
            }

            auto rb = std::make_shared<RigidBody>();

            // Override defaults from scene/template properties
            for (auto& [key, val] : p.second) {
                if      (key == "x")               rb->x               = val.cast<float>();
                else if (key == "y")               rb->y               = val.cast<float>();
                else if (key == "body_type")       rb->body_type       = val.cast<std::string>();
                else if (key == "precise")         rb->precise         = val.cast<bool>();
                else if (key == "gravity_scale")   rb->gravity_scale   = val.cast<float>();
                else if (key == "density")         rb->density         = val.cast<float>();
                else if (key == "angular_friction") rb->angular_friction = val.cast<float>();
                else if (key == "rotation")        rb->rotation        = val.cast<float>();
                else if (key == "has_collider")    rb->has_collider    = val.cast<bool>();
                else if (key == "has_trigger")     rb->has_trigger     = val.cast<bool>();

                else if (key == "collider_type")     rb->collider_type    = val.cast<std::string>();
                else if (key == "width")     rb->width    = val.cast<float>();
                else if (key == "height")     rb->height     = val.cast<float>();
                else if (key == "radius")     rb->radius     = val.cast<float>();
                else if (key == "friction")     rb->friction     = val.cast<float>();
                else if (key == "bounciness")     rb->bounciness     = val.cast<float>();

                else if (key == "trigger_type") rb->trigger_type    = val.cast<std::string>();
                else if (key == "trigger_width")     rb->trigger_width    = val.cast<float>();
                else if (key == "trigger_height")     rb->trigger_height     = val.cast<float>();
                else if (key == "trigger_radius")     rb->trigger_radius     = val.cast<float>();
            }

            // Store on actor — we'll call OnStart() in pass 2
            a->rigidbodies[p.first] = rb;
            continue; // Skip Lua loading for this component
        }
        
        if (type == "ParticleSystem") {
            particle_system_ever_existed = true;

            auto ps = std::make_shared<ParticleSystem>();

            for (auto& [key, val] : p.second) {
                if      (key == "x")                  ps->x                  = val.cast<float>();
                else if (key == "y")                  ps->y                  = val.cast<float>();
                else if (key == "emit_angle_min")     ps->emit_angle_min     = val.cast<float>();
                else if (key == "emit_angle_max")     ps->emit_angle_max     = val.cast<float>();
                else if (key == "emit_radius_min")    ps->emit_radius_min    = val.cast<float>();
                else if (key == "emit_radius_max")    ps->emit_radius_max    = val.cast<float>();
                else if (key == "frames_between_bursts") ps->frames_between_bursts = val.cast<int>();
                else if (key == "burst_quantity")     ps->burst_quantity     = val.cast<int>();
                else if (key == "start_scale_min")    ps->start_scale_min    = val.cast<float>();
                else if (key == "start_scale_max")    ps->start_scale_max    = val.cast<float>();
                else if (key == "rotation_min")       ps->rotation_min       = val.cast<float>();
                else if (key == "rotation_max")       ps->rotation_max       = val.cast<float>();
                else if (key == "start_color_r")      ps->start_color_r      = val.cast<int>();
                else if (key == "start_color_g")      ps->start_color_g      = val.cast<int>();
                else if (key == "start_color_b")      ps->start_color_b      = val.cast<int>();
                else if (key == "start_color_a")      ps->start_color_a      = val.cast<int>();
                else if (key == "image")              ps->image              = val.cast<std::string>();
                else if (key == "sorting_order")      ps->sorting_order      = val.cast<int>();
                else if (key == "duration_frames")    ps->duration_frames    = val.cast<int>();

                else if (key == "start_speed_min")    ps->start_speed_min    = val.cast<float>();
                else if (key == "start_speed_max")    ps->start_speed_max    = val.cast<float>();
                else if (key == "rotation_speed_min") ps->rotation_speed_min = val.cast<float>();
                else if (key == "rotation_speed_max") ps->rotation_speed_max = val.cast<float>();

                else if (key == "gravity_scale_x")    ps->gravity_scale_x    = val.cast<float>();
                else if (key == "gravity_scale_y")    ps->gravity_scale_y    = val.cast<float>();
                else if (key == "drag_factor")        ps->drag_factor        = val.cast<float>();
                else if (key == "angular_drag_factor") ps->angular_drag_factor = val.cast<float>();
                else if (key == "end_scale") {
                    ps->end_scale     = val.cast<float>();
                    ps->end_scale_set = true;
                }
                else if (key == "end_color_r") { ps->end_color_r = val.cast<int>(); ps->end_color_r_set = true; }
                else if (key == "end_color_g") { ps->end_color_g = val.cast<int>(); ps->end_color_g_set = true; }
                else if (key == "end_color_b") { ps->end_color_b = val.cast<int>(); ps->end_color_b_set = true; }
                else if (key == "end_color_a") { ps->end_color_a = val.cast<int>(); ps->end_color_a_set = true; }
            }

            ps->actor = a;
            ps->key   = p.first;

            a->particle_systems[p.first] = ps;
            a->particle_systems_to_start.push_back(p.first);
            continue;
        }

        fs::path lua_path = std::string("resources/component_types/") + type + ".lua";
        if (!fs::exists(lua_path)) {
            std::cout << "error: failed to locate component " << type;
            exit(0);
        }
        if (luaL_dofile(L, lua_path.string().c_str()) != LUA_OK){
            std::cout << std::string("problem with lua file ") + type;
            exit(0);
        }

        luabridge::LuaRef base = luabridge::getGlobal(L, type.c_str());
        luabridge::LuaRef instance = luabridge::newTable(L);
        instance["key"] = p.first;
        instance["enabled"] = true;
        luabridge::LuaRef metatable = luabridge::newTable(L);
        metatable["__index"] = base;

        for (auto& [key, val] : p.second) {
            instance[key] = val;
        }

        instance.push(L);
        metatable.push(L);
        lua_setmetatable(L, -2);
        lua_pop(L, 1);

        auto component_ref = std::make_shared<luabridge::LuaRef>(instance);
        
        a->InjectConvenienceReferences(component_ref);

        // // Store the component on the actor by key
        // a->components[p.first] = component_ref;
    }
}

luabridge::LuaRef Game::extractValue(const rapidjson::Value& val) {
    if (val.IsString()) return luabridge::LuaRef(L, val.GetString());
    if (val.IsInt())    return luabridge::LuaRef(L, val.GetInt());
    if (val.IsDouble()) return luabridge::LuaRef(L, val.GetDouble());
    if (val.IsBool())   return luabridge::LuaRef(L, val.GetBool());
    return luabridge::LuaRef(L);
}

void Game::load_actors(const rapidjson::Document &initial_scene_document, int onstart_from_index){

    const rapidjson::Value& actors_json = initial_scene_document["actors"];

    for (rapidjson::SizeType i = 0; i < actors_json.Size(); i++){
        const rapidjson::Value& actor = actors_json[i];
        std::string name = "";
        std::map<std::string, std::map<std::string, luabridge::LuaRef>> components; 
        if (actor.HasMember("template")){

            std::string template_name = actor["template"].GetString();
            
            // Check cache first
            if (template_cache.find(template_name) == template_cache.end()){
                // Load template only if not cached
                fs::path template_path = "resources/actor_templates/" + template_name + ".template";
                
                if (!fs::exists(template_path)) {
                    std::cout << "error: template " << template_name << " is missing";
                    exit(0);
                }
                
                //Place into template cache 
                template_cache[template_name] = rapidjson::Document();
                eu.ReadJsonFile(template_path.string(), template_cache[template_name]);
            }
            
            const rapidjson::Document& template_document = template_cache[template_name];

            if (template_document.HasMember("name")) { name = template_document["name"].GetString(); }
            if (template_document.HasMember("components")) {    
                for (auto it = template_document["components"].MemberBegin(); it != template_document["components"].MemberEnd(); ++it) {
                    const std::string compName = it->name.GetString();
                    
                    // Get or create the inner map
                    auto& inner = components[compName]; // outer map is fine, std::map is the value
                    
                    for (auto jt = it->value.MemberBegin(); jt != it->value.MemberEnd(); ++jt) {
                        inner.erase(jt->name.GetString());
                        inner.emplace(jt->name.GetString(), extractValue(jt->value));
                    }
                }
            }
        }

        if (actor.HasMember("name")) { name = actor["name"].GetString(); }
        if (actor.HasMember("components")) {    
            for (auto it = actor["components"].MemberBegin(); it != actor["components"].MemberEnd(); ++it) {
                const std::string compName = it->name.GetString();
                // Get or create the inner map
                auto& inner = components[compName]; // outer map is fine, std::map is the value
                
                for (auto jt = it->value.MemberBegin(); jt != it->value.MemberEnd(); ++jt) {
                    inner.erase(jt->name.GetString());
                    inner.emplace(jt->name.GetString(), extractValue(jt->value));
                }
            }
        }

        Actor* a = new Actor(static_cast<int>(i), name);
        actors.push_back(a);
        load_lua_file(components, actors.back());
    }
    // Pass 2: now actors is fully populated, safe to call OnStart
    for (int i = onstart_from_index; i < actors.size(); i++) {
        Actor* a = actors[i];

        // OnStart for C++ Rigidbodies
        for (auto& [key, rb] : a->rigidbodies) {
            rb->Initialize(physics_world, a); // Creates b2Body, fixture, etc.
        }

        // OnStart for Lua components (existing logic)
        for (auto& [key, component_ref] : a->components) {
            luabridge::LuaRef onStart = (*component_ref)["OnStart"];
            if (onStart.isFunction() && (*component_ref)["enabled"]) {
                try { onStart(*component_ref); }
                catch (const luabridge::LuaException& e) { ReportError(a->GetName(), e); }
            }
        }
    }
}

void Game::SceneLoad(const std::string& scene_name) {
    // Last call in a frame wins
    pending_scene_name = scene_name;
    scene_load_pending = true;
}

std::string Game::SceneGetCurrent() {
    return current_scene_name;
}

void Game::SceneDontDestroy(Actor* actor) {
    if (actor) {
        actor->dont_destroy = true;
    }
}

void Game::ProcessSceneLoad() {
    if (!scene_load_pending) return;
    scene_load_pending = false;
 
    const std::string scene_name = pending_scene_name;
    pending_scene_name = "";
 
    // 1. Collect persistent actors
    std::vector<Actor*> persistent_actors;
    for (Actor* a : actors) {
        if (a->dont_destroy) {
            persistent_actors.push_back(a);
        }
    }
 
    // 2. Delete non-persistent actors
    for (Actor* a : actors) {
        if (!a->dont_destroy) {
            for (auto& [key, component_ref] : a->components) {
                luabridge::LuaRef onDestroy = (*component_ref)["OnDestroy"];
                if (onDestroy.isFunction()) {
                    try { onDestroy(*component_ref); }
                    catch (const luabridge::LuaException& e) { ReportError(a->GetName(), e); }
                }
            }
            for (auto& [key, rb] : a->rigidbodies) {
                rb->OnDestroy(physics_world);
            }
            delete a;
        }
    }
    actors.clear();
 
    for (Actor* a : actors_to_add) {
        if (!a->dont_destroy) delete a;
    }
    actors_to_add.clear();
 
    for (Actor* a : actors_to_destroy) {
        if (!a->dont_destroy) delete a;
    }
    actors_to_destroy.clear();
 
    // 3. Restore persistent actors
    for (Actor* a : persistent_actors) {
        actors.push_back(a);
    }
 
    // 4. Load the new scene file
    fs::path scene_path = "resources/scenes/" + scene_name + ".scene";
    if (!fs::exists(scene_path)) {
        std::cout << "error: scene " << scene_name << " is missing";
        exit(0);
    }
 
    rapidjson::Document scene_document;
    eu.ReadJsonFile(scene_path.string(), scene_document);
 
    current_scene_name = scene_name;
 
    int new_actors_start = static_cast<int>(actors.size()); // snapshot after persistent actors restored
    load_actors(scene_document, new_actors_start);
}

Actor* Game::Find(std::string name) {
    for (Actor* a : actors){
        if (a->GetName() == name){
            for (Actor* a : actors_to_destroy){
                if (a->GetName() == name){
                    return nullptr;
                }
            }
            return a;
        }
    }
    for (Actor* a : actors_to_add){
        if (a->GetName() == name){
            return a;
        }
    }
    return nullptr; 
}

luabridge::LuaRef Game::FindAll(std::string name){
    luabridge::LuaRef table = luabridge::newTable(L);
    int idx = 1;
    for (Actor* a : actors){
        if (a->GetName() == name){
            for (Actor* a : actors_to_destroy){
                if (a->GetName() == name){
                    continue;
                }
            }
            table[idx++] = a;
        }
    }
    for (Actor* a : actors_to_add){
        if (a->GetName() == name){
            table[idx++] = a;
        }
    }
    return table;
}

void Game::update_all_actors(){
    // ── Rigidbody init ────────────────────────────────────────────────────
    for (auto a : actors) {
        for (const std::string& key : a->rigidbodies_to_start) {
            a->rigidbodies[key]->Initialize(physics_world, a);
        }
        a->rigidbodies_to_start.clear();
    }

    // ── ParticleSystem OnStart 
    for (auto a : actors) {
        for (const std::string& key : a->particle_systems_to_start) {
            a->particle_systems[key]->OnStart();
        }
        a->particle_systems_to_start.clear();
    }

    // ── Lua component OnStart ─────────────────────────────────────────────
    for (auto a: actors){
        for (auto c : a->components_to_start){
            luabridge::LuaRef onStart = (*c)["OnStart"];
            if (onStart.isFunction() && (*c)["enabled"]){
                try {
                    onStart(*c);
                }
                catch (const luabridge::LuaException& e) {
                    ReportError(a->GetName(), e);
                }
            }
        }
    }
    for (auto a: actors){
        a->components_to_start.clear();
    }

    // ── Lua component OnUpdate ────────────────────────────────────────────
    for (auto a: actors){
        for (auto& [key, component_ref] : a->components){
            luabridge::LuaRef onUpdate = (*component_ref)["OnUpdate"];
            if (onUpdate.isFunction() && (*component_ref)["enabled"]){
                try {
                    onUpdate(*component_ref);
                }
                catch (const luabridge::LuaException& e) {
                    ReportError(a->GetName(), e);
                }
            }
        }
    }

    // ── ParticleSystem OnUpdate ← ADD THIS ───────────────────────────────
    for (auto a : actors) {
        for (auto& [key, ps] : a->particle_systems) {
            if (ps && ps->enabled)
                ps->OnUpdate();
        }
    }
}

void Game::late_update_all_actors(){
    for (auto a: actors){
        for (auto& [key, component_ref] : a->components){
            luabridge::LuaRef onLateUpdate = (*component_ref)["OnLateUpdate"];
            if (onLateUpdate.isFunction() && (*component_ref)["enabled"]){
                try {
                    onLateUpdate(*component_ref);
                }
                catch (const luabridge::LuaException& e) {
                    ReportError(a->GetName(), e);
                }
            }
        }
    }
    for (auto a : actors) {
        a->InitComponents();

        // Call OnDestroy on components being removed
        for (const std::string& key : a->components_to_remove) {
            // Check Lua components
            auto it = a->components.find(key);
            if (it != a->components.end()) {
                luabridge::LuaRef onDestroy = (*it->second)["OnDestroy"];
                if (onDestroy.isFunction()) {
                    try { onDestroy(*it->second); }
                    catch (const luabridge::LuaException& e) { ReportError(a->GetName(), e); }
                }
                a->components.erase(it);
            }
            // Check rigidbodies
            auto rb_it = a->rigidbodies.find(key);
            if (rb_it != a->rigidbodies.end()) {
                rb_it->second->OnDestroy(physics_world);
                a->rigidbodies.erase(rb_it);
            }
        } 
        a->components_to_remove.clear();
    }
}

void Game::ProcessPendingActorChanges() {
    // 1. Handle Deletions
    for (Actor* a : actors_to_destroy) {
        // Remove from main actors list
        auto it = std::find(actors.begin(), actors.end(), a);
        if (it != actors.end()) {
            actors.erase(it);
        }
        auto jt = std::find(actors_to_add.begin(), actors_to_add.end(), a);
        if (jt != actors_to_add.end()) {
            actors_to_add.erase(jt);
        }
        // Free memory
        delete a;
    }
    actors_to_destroy.clear();

    // 2. Clear Addition list 
    // (They were already pushed to 'actors' in Instantiate to be discoverable)
    for (Actor * a: actors_to_add){
        actors.push_back(a);
    }
    actors_to_add.clear();
}

Actor* Game::Instantiate(std::string template_name) {
    // 1. Check/Load Template Cache (Same logic as load_actors)
    if (template_cache.find(template_name) == template_cache.end()) {
        fs::path template_path = "resources/actor_templates/" + template_name + ".template";
        if (!fs::exists(template_path)) {
            // Handle error or return nullptr
            return nullptr; 
        }
        template_cache[template_name] = rapidjson::Document();
        // Assuming 'eu' is your EngineUtils instance
        eu.ReadJsonFile(template_path.string(), template_cache[template_name]);
    }

    const rapidjson::Document& doc = template_cache[template_name];
    
    // 2. Extract Components from Template
    std::map<std::string, std::map<std::string, luabridge::LuaRef>> components;
    std::string name = doc.HasMember("name") ? doc["name"].GetString() : "";

    if (doc.HasMember("components")) {
        for (auto it = doc["components"].MemberBegin(); it != doc["components"].MemberEnd(); ++it) {
            const std::string compName = it->name.GetString();
            auto& inner = components[compName];
            for (auto jt = it->value.MemberBegin(); jt != it->value.MemberEnd(); ++jt) {
                inner.emplace(jt->name.GetString(), extractValue(jt->value));
            }
        }
    }

    // 3. Create Actor
    // Use a unique ID (perhaps based on current total + pending)
    Actor* new_actor = new Actor(static_cast<int>(actors.size() + actors_to_add.size()), name);
    
    // 4. Load Lua Components (populates new_actor->components)
    load_lua_file(components, new_actor);

    for (auto& [key, rb] : new_actor->rigidbodies) {
        new_actor->rigidbodies_to_start.push_back(key);
    }

    for (auto& [key, component_ref] : new_actor->components) {
        // Check if the component has an OnStart function
        if ((*component_ref)["OnStart"].isFunction()) {
            // Store the component (the Lua table), not just the function
            new_actor->components_to_start.push_back(component_ref); 
        }
    }

    actors_to_add.push_back(new_actor);

    return new_actor;
}

void Game::Destroy(Actor* a) {
    if (!a) return;

    for (auto& [key, component_ref] : a->components) {
        (*component_ref)["enabled"] = false;
        luabridge::LuaRef onDestroy = (*component_ref)["OnDestroy"];
        if (onDestroy.isFunction()) {
            try { onDestroy(*component_ref); }
            catch (const luabridge::LuaException& e) { ReportError(a->GetName(), e); }
        }
    }
    for (auto& [key, rb] : a->rigidbodies) {
        rb->OnDestroy(physics_world);
    }   
    for (auto& [key, ps] : a->particle_systems) {
        ps->enabled = false;
    }
    actors_to_destroy.push_back(a);
}
