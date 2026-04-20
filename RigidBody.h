#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include <string>
#include <iostream>
#include <cmath>
#include "box2d/include/box2d/box2d.h"
#include "glm/glm/glm.hpp"

class Actor;

static const uint16_t CATEGORY_COLLIDER = 0x0001;
static const uint16_t CATEGORY_TRIGGER  = 0x0002;

class RigidBody {
public:
    RigidBody()
        : x(0.0f), y(0.0f), body_type("dynamic"), precise(true),
          gravity_scale(1.0f), density(1.0f), angular_friction(0.3f),
          rotation(0.0f), has_collider(true), has_trigger(true),
          body(nullptr), collider_type("box"), width(1.0f), height(1.0f), 
          radius(0.5f), friction(0.3f), bounciness(0.3f), trigger_type("box"),
          trigger_width(1.0f), trigger_height(1.0f), trigger_radius(0.5f) {}

    float x;
    float y;
    std::string body_type;
    bool precise;
    float gravity_scale;
    float density;
    float angular_friction;
    float rotation;
    bool has_collider;
    bool has_trigger;

    void Initialize(b2World* world, Actor *a);

    b2Vec2 GetPosition();

    float GetRotation();

    void AddForce(b2Vec2 vec2) {body->ApplyForceToCenter(vec2, true);}
    void SetVelocity(b2Vec2 vec2) {body->SetLinearVelocity(vec2);}
    void SetPosition(b2Vec2 vec2);
    void SetRotation(float degrees_clockwise) { body->SetTransform(GetPosition(), DegreesToRadians(degrees_clockwise)); }
    void SetAngularVelocity(float degrees_clockwise){body->SetAngularVelocity(DegreesToRadians(degrees_clockwise)); }
    void SetGravityScale(float scale){body->SetGravityScale(scale);}
    void SetUpDirection(b2Vec2 direction);
    void SetRightDirection(b2Vec2 direction);

    b2Vec2 GetVelocity() { return body->GetLinearVelocity(); }
    float GetAngularVelocity() {return RadiansToDegrees(body->GetAngularVelocity()); }
    float GetGravityScale() { return body->GetGravityScale(); }
    b2Vec2 GetUpDirection();
    b2Vec2 GetRightDirection();

    std::string collider_type;
    float width;
    float height;
    float radius;
    float friction;
    float bounciness; 

    std::string trigger_type;
    float trigger_width;
    float trigger_height;
    float trigger_radius;

    void OnDestroy(b2World* world);

    std::string key;
private:
    b2Body* body;

    float DegreesToRadians(float degrees) {
        return degrees * (b2_pi / 180.0f);
    }

    float RadiansToDegrees(float radians) {
        return radians * (180.0f / b2_pi);
    }


};

#endif