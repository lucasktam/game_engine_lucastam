#include "RigidBody.h"
#include "Actor.h"

void RigidBody::Initialize(b2World* world, Actor *a) {
    b2BodyDef def;

    if      (body_type == "dynamic")   def.type = b2_dynamicBody;
    else if (body_type == "static")    def.type = b2_staticBody;
    else if (body_type == "kinematic") def.type = b2_kinematicBody;

    def.position.Set(x, y);
    def.angle        = rotation * (b2_pi / 180.0f); 
    def.bullet       = precise;
    def.angularDamping = angular_friction;
    def.gravityScale = gravity_scale;

    body = world->CreateBody(&def);

    

    // Collider fixture
    if (has_collider) {
        b2FixtureDef fixture_def;
        b2PolygonShape polygon_shape;
        b2CircleShape circle_shape;
        fixture_def.density     = density;
        fixture_def.friction    = friction;
        fixture_def.restitution = bounciness;
        fixture_def.isSensor    = false;

        // Only collide with other colliders
        fixture_def.filter.categoryBits = CATEGORY_COLLIDER;
        fixture_def.filter.maskBits     = CATEGORY_COLLIDER;

        if (collider_type == "box") {
            polygon_shape.SetAsBox(0.5f * width, 0.5f * height);
            fixture_def.shape = &polygon_shape;
        } else {
            circle_shape.m_radius = radius;
            fixture_def.shape = &circle_shape;
        }
        b2Fixture* fixture = body->CreateFixture(&fixture_def);
        fixture->GetUserData().pointer = reinterpret_cast<uintptr_t>(a);
    }

    // Trigger fixture
    if (has_trigger) {
        b2FixtureDef fixture_def;
        b2PolygonShape polygon_shape;
        b2CircleShape circle_shape;
        fixture_def.density  = density;
        fixture_def.isSensor = true;

        // Only collide with other triggers
        fixture_def.filter.categoryBits = CATEGORY_TRIGGER;
        fixture_def.filter.maskBits     = CATEGORY_TRIGGER;

        if (trigger_type == "box") {
            polygon_shape.SetAsBox(0.5f * trigger_width, 0.5f * trigger_height);
            fixture_def.shape = &polygon_shape;
        } else {
            circle_shape.m_radius = trigger_radius;
            fixture_def.shape = &circle_shape;
        }
        b2Fixture* fixture = body->CreateFixture(&fixture_def);
        fixture->GetUserData().pointer = reinterpret_cast<uintptr_t>(a);
    }

    // Phantom (no collider, no trigger) — keep as-is but also give it a category
    if (!has_collider && !has_trigger) {
        b2PolygonShape phantom_shape;
        phantom_shape.SetAsBox(0.5f * width, 0.5f * height);
        b2FixtureDef phantom_fixture_def;
        phantom_fixture_def.shape    = &phantom_shape;
        phantom_fixture_def.density  = density;
        phantom_fixture_def.isSensor = true;
        // Phantom doesn't interact with anything
        phantom_fixture_def.filter.categoryBits = 0x0000;
        phantom_fixture_def.filter.maskBits     = 0x0000;
        b2Fixture* fixture = body->CreateFixture(&phantom_fixture_def);
        fixture->GetUserData().pointer = reinterpret_cast<uintptr_t>(a);
    }
}

b2Vec2 RigidBody::GetPosition() {
    if (!body) return {x, y};

        b2Vec2 pos = body->GetPosition();
        return {pos.x, pos.y};
}

float RigidBody::GetRotation(){
    if (!body) return rotation;

        float radians = body->GetAngle();
        return RadiansToDegrees(radians);
}

void RigidBody::SetPosition(b2Vec2 pos) {
    if (body == nullptr) {
        x = pos.x;
        y = pos.y;
    } else {
        float current_rotation = body->GetAngle();
        body->SetTransform(pos, current_rotation);
    }
}

void RigidBody::SetUpDirection(b2Vec2 direction) {
    b2Vec2 dir(direction.x, direction.y);
    dir.Normalize();
    
    float angle = glm::atan(dir.x, -dir.y);
    
    body->SetTransform(body->GetPosition(), angle);
}

void RigidBody::SetRightDirection(b2Vec2 direction) {
    b2Vec2 dir(direction.x, direction.y);
    dir.Normalize();
    
    float angle = glm::atan(dir.x, -dir.y);
    
    body->SetTransform(body->GetPosition(), angle - b2_pi / 2.0f);
}

b2Vec2 RigidBody::GetUpDirection(){
    float angle = body->GetAngle();
    b2Vec2 result = b2Vec2(glm::sin(angle), -glm::cos(angle));
    result.Normalize();
    return result;
}

b2Vec2 RigidBody::GetRightDirection(){
    float angle = body->GetAngle();
    b2Vec2 result = b2Vec2(glm::cos(angle), glm::sin(angle));
    result.Normalize();
    return result;
}

void RigidBody::OnDestroy(b2World* world) {
    if (body && world) {
        world->DestroyBody(body);
        body = nullptr;
        
    }
}