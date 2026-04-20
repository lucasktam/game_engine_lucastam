#include "Application.h"

// Application.cpp
void Application::Quit() { exit(0); }

void Application::Sleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int Application::GetFrame() { return Helper::GetFrameNumber(); }

void Application::OpenURL(const std::string& url) {
#ifdef _WIN32
    std::system(("start " + url).c_str());
#elif __APPLE__
    std::system(("open " + url).c_str());
#else
    std::system(("xdg-open " + url).c_str());
#endif
}