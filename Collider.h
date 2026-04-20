#ifndef COLLIDER_H
#define COLLIDER_H

#include <string>
#include <iostream>
#include <cmath>
#include "box2d/include/box2d/box2d.h"
#include "glm/glm/glm.hpp"
#include "Lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

class Actor; 

struct Collision {
    Actor* other;
    b2Vec2 point;
    b2Vec2 relative_velocity;
    b2Vec2 normal;
};

class Collider : public b2ContactListener {
public:
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;
private:
    void FireCollisionEvent(Actor* self, const Collision& col, const std::string& func_name);
};

#endif