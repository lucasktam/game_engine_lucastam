#include "ParticleSystem.h"
#include "Renderer.h"
#include "Actor.h"

void ParticleSystem::OnStart() {
    emit_angle_distribution  = RandomEngine(emit_angle_min,  emit_angle_max,  298);
    emit_radius_distribution = RandomEngine(emit_radius_min, emit_radius_max, 404);

    rotation_distribution = RandomEngine(rotation_min,      rotation_max,      440);
    scale_distribution    = RandomEngine(start_scale_min,   start_scale_max,   494);

    speed_distribution          = RandomEngine(start_speed_min,    start_speed_max,    498);
    rotation_speed_distribution = RandomEngine(rotation_speed_min, rotation_speed_max, 305);

    // Clamp to minimums
    if (frames_between_bursts < 1) frames_between_bursts = 1;
    if (burst_quantity < 1)        burst_quantity = 1;
    if (duration_frames < 1)       duration_frames = 1;

    // Ensure default texture exists
    if (particle_default_texture.empty()) {
        particle_default_texture = "particle_default_texture";
        Renderer::CreateDefaultParticleTextureWithName(particle_default_texture);
    }
}

void ParticleSystem::OnUpdate() {
    // 1. Burst if it's time
    if (is_playing && local_frame_number % frames_between_bursts == 0) {
        Burst();
    }

    // 2. Process and render all particle slots
    RenderParticles();

    // 3. Increment local frame counter at the very end
    local_frame_number++;
}

void ParticleSystem::Burst() {
    for (int i = 0; i < burst_quantity; i++) {
        SpawnParticle();
    }
}

void ParticleSystem::SpawnParticle() {
    /* Emission Shape */
    float angle_radians = glm::radians(emit_angle_distribution.Sample());
    float radius        = emit_radius_distribution.Sample();

    float cos_angle = glm::cos(angle_radians);
    float sin_angle = glm::sin(angle_radians);

    float starting_x_pos = x + cos_angle * radius;
    float starting_y_pos = y + sin_angle * radius;

    /* Emission Velocity */
    float speed = speed_distribution.Sample();

    float starting_x_vel = cos_angle * speed;
    float starting_y_vel = sin_angle * speed;
    

    int slot;
    if (!free_list.empty()) {
        slot = free_list.front();
        free_list.pop();
    } else {
        slot = number_of_particle_slots;
        particles.emplace_back();
        number_of_particle_slots++;
    }

    Particle& p      = particles[slot];
    p.x              = starting_x_pos;
    p.y              = starting_y_pos;
    p.vel_x          = starting_x_vel;
    p.vel_y          = starting_y_vel;
    p.rotation       = rotation_distribution.Sample();
    p.rotation_speed = rotation_speed_distribution.Sample();
    p.scale          = scale_distribution.Sample();
    p.start_scale    = p.scale;
    p.r              = static_cast<float>(start_color_r);
    p.g              = static_cast<float>(start_color_g);
    p.b              = static_cast<float>(start_color_b);
    p.a              = static_cast<float>(start_color_a);
    p.start_r        = p.r;
    p.start_g        = p.g;
    p.start_b        = p.b;
    p.start_a        = p.a;
    p.start_frame    = local_frame_number;
    p.active         = true;
    number_of_particles++;
}

void ParticleSystem::RenderParticles() {
    const std::string& tex = (!image.empty()) ? image : particle_default_texture;

    for (int i = 0; i < number_of_particle_slots; i++) {
        /* Inactive particles are not processed or rendered */
        if (particles[i].active == false)
            continue;

        /* Check if the particle's lifetime has expired */
        int frames_particle_has_been_alive = local_frame_number - particles[i].start_frame;
        if (frames_particle_has_been_alive >= duration_frames) {
            particles[i].active = false;
            free_list.push(i);
            number_of_particles--;
            continue;
        }

        Particle& p = particles[i];

        // 1. Apply gravity to velocity
        p.vel_x += gravity_scale_x;
        p.vel_y += gravity_scale_y;

        // 2. Apply drag to velocity, angular drag to angular velocity
        p.vel_x          *= drag_factor;
        p.vel_y          *= drag_factor;
        p.rotation_speed *= angular_drag_factor;

        // 3. Apply velocities to position and rotation
        p.x        += p.vel_x;
        p.y        += p.vel_y;
        p.rotation += p.rotation_speed;

        // 4. Process scale
        float render_scale = p.scale;
        if (end_scale_set) {
            float lifetime_progress = static_cast<float>(frames_particle_has_been_alive)
                                    / static_cast<float>(duration_frames);
            render_scale = glm::mix(p.start_scale, end_scale, lifetime_progress);
        }

        // 4. Process color
        float render_r = p.r;
        float render_g = p.g;
        float render_b = p.b;
        float render_a = p.a;
        if (end_color_r_set || end_color_g_set || end_color_b_set || end_color_a_set) {
            float lifetime_progress = static_cast<float>(frames_particle_has_been_alive)
                                    / static_cast<float>(duration_frames);
            if (end_color_r_set) render_r = glm::mix(p.start_r, static_cast<float>(end_color_r), lifetime_progress);
            if (end_color_g_set) render_g = glm::mix(p.start_g, static_cast<float>(end_color_g), lifetime_progress);
            if (end_color_b_set) render_b = glm::mix(p.start_b, static_cast<float>(end_color_b), lifetime_progress);
            if (end_color_a_set) render_a = glm::mix(p.start_a, static_cast<float>(end_color_a), lifetime_progress);
        }

        Renderer::DrawEx(
            tex,
            p.x, p.y,
            p.rotation,
            render_scale, render_scale,
            0.5f, 0.5f,
            render_r, render_g, render_b, render_a,
            sorting_order
        );
    }
}

void ParticleSystem::Stop() {
    is_playing = false;
}

void ParticleSystem::Play() {
    is_playing = true;
}