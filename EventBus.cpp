#include "EventBus.h"

std::unordered_map<std::string, std::vector<EventBus::Subscriber>> EventBus::subscribers;
std::vector<std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>> EventBus::pending_subscribes;
std::vector<std::tuple<std::string, luabridge::LuaRef, luabridge::LuaRef>> EventBus::pending_unsubscribes;

void EventBus::Publish(const std::string& event_type, luabridge::LuaRef event_object) {
    auto it = subscribers.find(event_type);
    if (it == subscribers.end()) return;

    // Copy the list so mid-publish subscribe/unsubscribe doesn't break iteration
    std::vector<Subscriber> subs_copy = it->second;
    for (auto& [component, function] : subs_copy) {
        try { function(component, event_object); }
        catch (const luabridge::LuaException& e) {
            std::string msg = e.what();
            std::replace(msg.begin(), msg.end(), '\\', '/');
            std::cout << "\033[31m" << msg << "\033[0m" << std::endl;
        }
    }
}

void EventBus::Subscribe(const std::string& event_type, luabridge::LuaRef component, luabridge::LuaRef function) {
    pending_subscribes.emplace_back(event_type, component, function);
}

void EventBus::Unsubscribe(const std::string& event_type, luabridge::LuaRef component, luabridge::LuaRef function) {
    pending_unsubscribes.emplace_back(event_type, component, function);
}

void EventBus::ProcessPendingSubscriptions() {
    for (auto& [event_type, component, function] : pending_subscribes) {
        subscribers[event_type].emplace_back(component, function);
    }
    pending_subscribes.clear();

    for (auto& [event_type, component, function] : pending_unsubscribes) {
        auto it = subscribers.find(event_type);
        if (it == subscribers.end()) continue;

        luabridge::LuaRef comp = component;
        luabridge::LuaRef func = function;

        auto& subs = it->second;
        subs.erase(std::remove_if(subs.begin(), subs.end(), [comp, func](const Subscriber& s) {
            return s.first == comp && s.second == func;
        }), subs.end());
    }
    pending_unsubscribes.clear();
}