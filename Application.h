#include <string>
#include <chrono>
#include <thread>
#include "Helper.h"
#include "Lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

// Application.h
class Application {
public:
    static void Quit();
    static void Sleep(int milliseconds);
    static int GetFrame();
    static void OpenURL(const std::string& url);
};