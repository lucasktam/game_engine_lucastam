#include "engine.h"

using namespace std;
namespace fs = filesystem;


rapidjson::Document game_config = rapidjson::Document();
rapidjson::Document rendering_config = rapidjson::Document();
rapidjson::Document initial_scene_document = rapidjson::Document();  


void load_files(){ 
    // Load resources/ fodler
    const fs::path resources_path = "resources";
    if (!fs::is_directory(resources_path)) {
        cout << "error: resources/ missing";
        exit(0);
    }
    // Load game.config
    const fs::path game_config_path = "resources/game.config";
    if (!fs::exists(game_config_path)) {
        cout << "error: resources/game.config missing";
        exit(0);
    }
    eu.ReadJsonFile(game_config_path.string(), game_config);    
    // Load rendering.config
    const fs::path rendering_config_path = "resources/rendering.config";
    if (fs::exists(rendering_config_path)) {
        eu.ReadJsonFile(rendering_config_path.string(), rendering_config);
    }
    // Load map 

    if (!game_config.HasMember("initial_scene")) {
        cout << "error: initial_scene unspecified";
        exit(0);
    }

    fs::path initial_scene_path = "resources/scenes/" + std::string(game_config["initial_scene"].GetString()) + ".scene";
    if (!fs::exists(initial_scene_path)) {
        cout << "error: scene " << std::string(game_config["initial_scene"].GetString()) << " is missing";
        exit(0);
    }
    eu.ReadJsonFile(initial_scene_path.string(), initial_scene_document);
}
int main(int argc, char* argv[]) {
     // Create a new Lua state
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    luabridge::getGlobalNamespace(L)
        .beginNamespace("Debug")
        .addFunction("Log", ComponentDB::CppLog)
        .addFunction("LogError", ComponentDB::CppLogError)
        .endNamespace();
 
    luabridge::getGlobalNamespace(L)
        .beginClass<Actor>("Actor")
        .addFunction("GetName",           &Actor::GetName)
        .addFunction("GetID",             &Actor::GetID)
        .addFunction("GetComponentByKey", &Actor::GetComponentByKey)
        .addFunction("GetComponent",      &Actor::GetComponent)
        .addFunction("GetComponents",     &Actor::GetComponents)
        .addFunction("AddComponent",      &Actor::AddComponent)
        .addFunction("RemoveComponent",   &Actor::RemoveComponent)
        .addStaticFunction("Find",    &Game::Find)
        .addStaticFunction("FindAll", &Game::FindAll)
        .addStaticFunction("Instantiate", &Game::Instantiate)
        .addStaticFunction("Destroy", &Game::Destroy)
        .endClass();

    luabridge::getGlobalNamespace(L)
        .beginNamespace("Application")
        .addFunction("Quit",    &Application::Quit)
        .addFunction("Sleep",   &Application::Sleep)
        .addFunction("GetFrame", &Application::GetFrame)
        .addFunction("OpenURL", &Application::OpenURL)
        .endNamespace();

    luabridge::getGlobalNamespace(L)
        .beginClass<glm::vec2>("vec2")
        .addProperty("x", &glm::vec2::x)
        .addProperty("y", &glm::vec2::y)
        .endClass();

    luabridge::getGlobalNamespace(L)
        .beginNamespace("Input")
        .addFunction("GetKey", &Input::GetKey)
        .addFunction("GetKeyDown", &Input::GetKeyDown)
        .addFunction("GetKeyUp", &Input::GetKeyUp)
        .addFunction("GetMousePosition", &Input::GetMousePosition)
        .addFunction("GetMouseButton", &Input::GetMouseButton)
        .addFunction("GetMouseButtonDown", &Input::GetMouseButtonDown)
        .addFunction("GetMouseButtonUp", &Input::GetMouseButtonUp)
        .addFunction("GetMouseScrollDelta", &Input::GetMouseScrollDelta)
        .addFunction("HideCursor", &Input::HideCursor)
        .addFunction("ShowCursor", &Input::ShowCursor)
        .endNamespace();


    luabridge::getGlobalNamespace(L)
        // Image Namespace
        .beginClass<Renderer>("Image")
            .addStaticFunction("Draw", &Renderer::Draw)
            .addStaticFunction("DrawEx", &Renderer::DrawEx)
            .addStaticFunction("DrawUI", &Renderer::DrawUI)
            .addStaticFunction("DrawUIEx", &Renderer::DrawUIEx)
            .addStaticFunction("DrawPixel", &Renderer::DrawPixel)
        .endClass()
        
        // Text Namespace
        .beginClass<Renderer>("Text")
            .addStaticFunction("Draw", &Renderer::DrawText)
        .endClass()

        // Audio Namespace
        .beginClass<Audio>("Audio")
            .addStaticFunction("Play", &Audio::Play)
            .addStaticFunction("Halt", &Audio::Halt)
            .addStaticFunction("SetVolume", &Audio::SetVolume)
        .endClass();

    luabridge::getGlobalNamespace(L)
        .beginClass<Camera>("Camera")
        .addStaticFunction("SetPosition", &Camera::SetPosition)
        .addStaticFunction("GetPositionX", &Camera::GetPositionX)
        .addStaticFunction("GetPositionY", &Camera::GetPositionY)
        .addStaticFunction("SetZoom", &Camera::SetZoom)
        .addStaticFunction("GetZoom", &Camera::GetZoom)
        .endClass();

    luabridge::getGlobalNamespace(L)
        .beginNamespace("Scene")
            .addFunction("Load",        Game::SceneLoad)
            .addFunction("GetCurrent",  Game::SceneGetCurrent)
            .addFunction("DontDestroy", Game::SceneDontDestroy)
        .endNamespace();

    luabridge::getGlobalNamespace(L)
    .beginClass<b2Vec2>("Vector2")
        .addConstructor<void(*)(float, float)>()
        .addProperty("x", &b2Vec2::x)
        .addProperty("y", &b2Vec2::y)
        .addFunction("Normalize", &b2Vec2::Normalize)
        .addFunction("Length", &b2Vec2::Length)
        .addFunction("__add", &b2Vec2::operator_add)
        .addFunction("__sub", &b2Vec2::operator_sub)
        .addFunction("__mul", &b2Vec2::operator_mul)
        .addStaticFunction("Distance", &b2Distance)
        .addStaticFunction("Dot", static_cast<float(*)(const b2Vec2&, const b2Vec2&)>(&b2Dot))
    .endClass()
    .beginClass<RigidBody>("Rigidbody")
        .addProperty("x", &RigidBody::x)
        .addProperty("y", &RigidBody::y)
        .addProperty("body_type", &RigidBody::body_type)
        .addProperty("rotation", &RigidBody::rotation)
        .addProperty("gravity_scale", &RigidBody::gravity_scale)
        .addProperty("density", &RigidBody::density)
        .addProperty("angular_friction", &RigidBody::angular_friction)
        .addProperty("precise", &RigidBody::precise)
        .addProperty("has_collider", &RigidBody::has_collider)
        .addProperty("has_trigger", &RigidBody::has_trigger)
        .addFunction("GetPosition", &RigidBody::GetPosition)
        .addFunction("GetRotation", &RigidBody::GetRotation)
        .addFunction("AddForce", &RigidBody::AddForce)
        .addFunction("SetVelocity", &RigidBody::SetVelocity)
        .addFunction("SetPosition", &RigidBody::SetPosition)
        .addFunction("SetRotation", &RigidBody::SetRotation)
        .addFunction("SetAngularVelocity", &RigidBody::SetAngularVelocity)
        .addFunction("SetGravityScale", &RigidBody::SetGravityScale)
        .addFunction("SetUpDirection", &RigidBody::SetUpDirection)
        .addFunction("SetRightDirection", &RigidBody::SetRightDirection)
        .addFunction("GetVelocity", &RigidBody::GetVelocity)
        .addFunction("GetAngularVelocity", &RigidBody::GetAngularVelocity)
        .addFunction("GetGravityScale", &RigidBody::GetGravityScale)
        .addFunction("GetUpDirection", &RigidBody::GetUpDirection)
        .addFunction("GetRightDirection", &RigidBody::GetRightDirection)
    .endClass()
    .beginClass<Collision>("Collision")
        .addData("other",             &Collision::other)
        .addData("point",             &Collision::point)
        .addData("relative_velocity", &Collision::relative_velocity)
        .addData("normal",            &Collision::normal)
    .endClass();

    luabridge::getGlobalNamespace(L)
    .beginClass<HitResult>("HitResult")
        .addData("actor",      &HitResult::actor)
        .addData("point",      &HitResult::point)
        .addData("normal",     &HitResult::normal)
        .addData("is_trigger", &HitResult::is_trigger)
    .endClass();

    // Physics.Raycast
    luabridge::getGlobalNamespace(L)
    .beginNamespace("Physics")
        .addFunction("Raycast",    &Physics::Raycast)
        .addFunction("RaycastAll", &Physics::RaycastAll)
    .endNamespace();

    luabridge::getGlobalNamespace(L)
    .beginNamespace("Event")
        .addFunction("Publish",     &EventBus::Publish)
        .addFunction("Subscribe",   &EventBus::Subscribe)
        .addFunction("Unsubscribe", &EventBus::Unsubscribe)
    .endNamespace();

    luabridge::getGlobalNamespace(L)
    .beginClass<ParticleSystem>("ParticleSystem")
        .addProperty("enabled",               &ParticleSystem::enabled)
        .addProperty("x",                     &ParticleSystem::x)
        .addProperty("y",                     &ParticleSystem::y)
        .addProperty("emit_angle_min",        &ParticleSystem::emit_angle_min)
        .addProperty("emit_angle_max",        &ParticleSystem::emit_angle_max)
        .addProperty("emit_radius_min",       &ParticleSystem::emit_radius_min)
        .addProperty("emit_radius_max",       &ParticleSystem::emit_radius_max)
        .addProperty("frames_between_bursts", &ParticleSystem::frames_between_bursts)
        .addProperty("burst_quantity",        &ParticleSystem::burst_quantity)
        .addProperty("start_scale_min",       &ParticleSystem::start_scale_min)
        .addProperty("start_scale_max",       &ParticleSystem::start_scale_max)
        .addProperty("rotation_min",          &ParticleSystem::rotation_min)
        .addProperty("rotation_max",          &ParticleSystem::rotation_max)
        .addProperty("start_color_r",         &ParticleSystem::start_color_r)
        .addProperty("start_color_g",         &ParticleSystem::start_color_g)
        .addProperty("start_color_b",         &ParticleSystem::start_color_b)
        .addProperty("start_color_a",         &ParticleSystem::start_color_a)
        .addProperty("duration_frames",       &ParticleSystem::duration_frames)
        .addProperty("image",                 &ParticleSystem::image)
        .addProperty("sorting_order",         &ParticleSystem::sorting_order)
        .addProperty("start_speed_min",       &ParticleSystem::start_speed_min)
        .addProperty("start_speed_max",       &ParticleSystem::start_speed_max)
        .addProperty("rotation_speed_min",    &ParticleSystem::rotation_speed_min)
        .addProperty("rotation_speed_max",    &ParticleSystem::rotation_speed_max)
        .addProperty("gravity_scale_x",       &ParticleSystem::gravity_scale_x)
        .addProperty("gravity_scale_y",       &ParticleSystem::gravity_scale_y)
        .addProperty("drag_factor",           &ParticleSystem::drag_factor)
        .addProperty("angular_drag_factor",   &ParticleSystem::angular_drag_factor)
        .addProperty("end_scale",             &ParticleSystem::end_scale)
        .addProperty("end_color_r",           &ParticleSystem::end_color_r)
        .addProperty("end_color_g",           &ParticleSystem::end_color_g)
        .addProperty("end_color_b",           &ParticleSystem::end_color_b)
        .addProperty("end_color_a",           &ParticleSystem::end_color_a)
        .addFunction("Stop",  &ParticleSystem::Stop)
        .addFunction("Play",  &ParticleSystem::Play)
        .addFunction("Burst", &ParticleSystem::Burst)
    .endClass();

    load_files();

    

    bool is_running = true;

    TTF_Init();

    bool intro_bgm_playing = false;
    Renderer::create_window(game_config, rendering_config);
    Renderer::init_renderer(rendering_config);
    Audio::init(game_config);
 
    Game::init(L, game_config, initial_scene_document);

    while (is_running){    
        Renderer::Reset();
        SDL_RenderClear(Renderer::get_renderer());
        Game::ProcessSceneLoad();
        SDL_Event e; 
        while (Helper::SDL_PollEvent(&e)) 
        {
            if (e.type == SDL_QUIT)
            {
                is_running = false; 
            }
            
            Input::ProcessEvent(e);
            
        }
        Game::ProcessPendingActorChanges();
        Game::update_all_actors();
        Game::late_update_all_actors();
        Input::LateUpdate();
        
        EventBus::ProcessPendingSubscriptions();
        if (Game::physics_world) {
            Game::physics_world->Step(1.0f / 60.0f, 8, 3);
        }

        Renderer::RenderAll();
        Helper::SDL_RenderPresent(Renderer::get_renderer());

        
    }

    return 0;
    Renderer::Deinit();
    lua_close(L);

}

