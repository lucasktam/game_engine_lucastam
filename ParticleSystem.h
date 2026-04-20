#pragma once

#include <vector>
#include <queue>
#include <string>
#include "glm/glm/glm.hpp"
#include "Helper.h"

class Actor;

struct Particle {
    float x         = 0.0f;
    float y         = 0.0f;
    float scale     = 1.0f;
    float rotation  = 0.0f;
    float r         = 255.0f;
    float g         = 255.0f;
    float b         = 255.0f;
    float a         = 255.0f;
    bool  active    = false;
    int   start_frame = 0;  
    float vel_x          = 0.0f;
    float vel_y          = 0.0f;
    float rotation_speed = 0.0f;
    float start_scale = 1.0f;

    float start_r = 255.0f;
    float start_g = 255.0f;
    float start_b = 255.0f;
    float start_a = 255.0f;
};

class ParticleSystem {
public:
    void OnStart();
    void OnUpdate();
    void Stop();
    void Play();
    void Burst();
    // Owner / bookkeeping
    Actor*      actor   = nullptr;
    std::string key     = "???";
    bool        enabled = true;

    // Position offset
    float x = 0.0f;
    float y = 0.0f;

    // Emission shape
    float emit_angle_min  = 0.0f;
    float emit_angle_max  = 360.0f;
    float emit_radius_min = 0.0f;
    float emit_radius_max = 0.5f;

    // Burst control
    int frames_between_bursts = 1;
    int burst_quantity        = 1;

    // Scale
    float start_scale_min = 1.0f;
    float start_scale_max = 1.0f;

    // Rotation
    float rotation_min = 0.0f;
    float rotation_max = 0.0f;

    // Color
    int start_color_r = 255;
    int start_color_g = 255;
    int start_color_b = 255;
    int start_color_a = 255;

    // Lifetime
    int duration_frames = 300;

    // Image / sorting
    std::string image         = "";
    int         sorting_order = 9999;

    std::string particle_default_texture = "";

        // Suite #2 properties
    float start_speed_min    = 0.0f;
    float start_speed_max    = 0.0f;
    float rotation_speed_min = 0.0f;
    float rotation_speed_max = 0.0f;   

    // Suite #3 properties
    float gravity_scale_x   = 0.0f;
    float gravity_scale_y   = 0.0f;
    float drag_factor        = 1.0f;
    float angular_drag_factor = 1.0f;
    float end_scale          = -1.0f;  // sentinel: -1 means "not configured"
    bool  end_scale_set      = false;  // track whether designer configured it

    int  end_color_r     = 255;
    int  end_color_g     = 255;
    int  end_color_b     = 255;
    int  end_color_a     = 255;
    bool end_color_r_set = false;
    bool end_color_g_set = false;
    bool end_color_b_set = false;
    bool end_color_a_set = false;
private:
    // Slot-based particle storage
    std::vector<Particle> particles;          // fixed-size slots
    std::queue<int>       free_list;          // indices available for reuse
    int number_of_particle_slots = 0;
    int number_of_particles      = 0;

    int local_frame_number = 0;

    // Distributions — seeded once in OnStart()
    RandomEngine emit_angle_distribution {0.0f, 360.0f, 298};
    RandomEngine emit_radius_distribution{0.0f, 0.5f,   404};
    RandomEngine rotation_distribution   {0.0f, 0.0f,   440};
    RandomEngine scale_distribution      {1.0f, 1.0f,   494};
    RandomEngine speed_distribution         {0.0f, 0.0f, 498};
    RandomEngine rotation_speed_distribution{0.0f, 0.0f, 305};
    
    bool is_playing = true;
    void SpawnParticle();
    void RenderParticles();
};