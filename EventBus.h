#ifndef EVENTBUS_H
#define EVENTBUS_H

#include <string>
#include <vector>
#include <unordered_map>
#include "Lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <algorithm>
class EventBus {
public:
    using Subscriber = std::pair<luabridge::LuaRef, luabridge::LuaRef>; // {component, function}

    static void Publish(const std::string& event_type, luabridge::LuaRef event_object);
    static void Subscribe(const std::string& event_type, luabridge::LuaRef component, luabridge::LuaRef function);
    static void Unsubscribe(const std::string& event_type, luabridge::LuaRef component, luabridge::LuaRef function);

    // Call at end of frame to apply pending sub/unsub
    static void ProcessPendingSubscriptions();

private:
    static std::unordered_map<std::string, std::vector<Subscriber>> subscribers;

    static std::vector<std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>> pending_subscribes;
    static std::vector<std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>> pending_unsubscribes;
};

#endif