#ifndef RAYCASTCALLBACK_H
#define RAYCASTCALLBACK_H

#include "box2d/include/box2d/box2d.h"
#include "Actor.h"
#include <vector>

struct HitResult {
    Actor* actor;
    b2Vec2 point;
    b2Vec2 normal;
    bool is_trigger;
};

class RaycastCallback : public b2RayCastCallback {
public:
    std::vector<HitResult> hits;

    float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
                        const b2Vec2& normal, float fraction) override {

        // Skip phantom fixtures (category 0x0000)
        if (fixture->GetFilterData().categoryBits == 0x0000) return -1;

        Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);
        if (!actor) return -1;

        HitResult hit;
        hit.actor      = actor;
        hit.point      = point;
        hit.normal     = normal;
        hit.is_trigger = fixture->IsSensor();
        hits.push_back(hit);

        return 1; // don't clip the ray — collect all hits
    }
};

#endif