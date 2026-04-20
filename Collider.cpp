#include "Collider.h"
#include "Actor.h"
void Collider::FireCollisionEvent(Actor* self, const Collision& col, const std::string& func_name) {
    if (!self) return;
    for (auto& [key, component_ref] : self->components) {
        luabridge::LuaRef fn = (*component_ref)[func_name];
        if (fn.isFunction() && (*component_ref)["enabled"]) {
            try { fn(*component_ref, col); }
            catch (const luabridge::LuaException& e) {
                std::string msg = e.what();
                std::replace(msg.begin(), msg.end(), '\\', '/');
                std::cout << "\033[31m" << self->GetName() << " : " << msg << "\033[0m" << std::endl;
            }
        }
    }
}

void Collider::BeginContact(b2Contact* contact) {
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    Actor* actorA = reinterpret_cast<Actor*>(fixtureA->GetUserData().pointer);
    Actor* actorB = reinterpret_cast<Actor*>(fixtureB->GetUserData().pointer);
    if (!actorA || !actorB) return;

    bool isTrigger = fixtureA->IsSensor() && fixtureB->IsSensor();
    std::string func = isTrigger ? "OnTriggerEnter" : "OnCollisionEnter";

    b2Vec2 sentinel(-999.0f, -999.0f);

    if (isTrigger) {
        // Triggers: all four fields — point and normal are sentinel
        b2Vec2 velA = fixtureA->GetBody()->GetLinearVelocity();
        b2Vec2 velB = fixtureB->GetBody()->GetLinearVelocity();
        b2Vec2 relative_velocity = velA - velB;

        Collision colForA { actorB, sentinel, relative_velocity, sentinel };
        FireCollisionEvent(actorA, colForA, func);

        Collision colForB { actorA, sentinel, relative_velocity, sentinel };
        FireCollisionEvent(actorB, colForB, func);
    } else {
        b2WorldManifold worldManifold;
        contact->GetWorldManifold(&worldManifold);

        b2Vec2 point  = worldManifold.points[0];
        b2Vec2 normal = worldManifold.normal;
        b2Vec2 velA   = fixtureA->GetBody()->GetLinearVelocity();
        b2Vec2 velB   = fixtureB->GetBody()->GetLinearVelocity();
        b2Vec2 relative_velocity = velA - velB;

        Collision colForA { actorB, point, relative_velocity, normal };
        FireCollisionEvent(actorA, colForA, func);

        Collision colForB { actorA, point, relative_velocity, normal };
        FireCollisionEvent(actorB, colForB, func);
    }
}

void Collider::EndContact(b2Contact* contact) {
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    Actor* actorA = reinterpret_cast<Actor*>(fixtureA->GetUserData().pointer);
    Actor* actorB = reinterpret_cast<Actor*>(fixtureB->GetUserData().pointer);
    if (!actorA || !actorB) return;

    bool isTrigger = fixtureA->IsSensor() && fixtureB->IsSensor();
    std::string func = isTrigger ? "OnTriggerExit" : "OnCollisionExit";

    b2Vec2 sentinel(-999.0f, -999.0f);
    b2Vec2 velA = fixtureA->GetBody()->GetLinearVelocity();
    b2Vec2 velB = fixtureB->GetBody()->GetLinearVelocity();
    b2Vec2 relative_velocity = velA - velB;

    // Both enter and exit for triggers use sentinel for point/normal
    Collision colForA { actorB, sentinel, relative_velocity, sentinel };
    FireCollisionEvent(actorA, colForA, func);

    Collision colForB { actorA, sentinel, relative_velocity, sentinel };
    FireCollisionEvent(actorB, colForB, func);
}