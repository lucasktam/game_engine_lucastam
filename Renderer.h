#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <vector>
#include <filesystem>
#include "glm/glm/glm.hpp"
#include "Helper.h"
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/filereadstream.h"
#include "Camera.h"

struct TextDrawRequest {
    std::string content;
    int x, y;
    std::string font_name;
    int font_size;
    SDL_Color color;
};

struct ImageRequest {
    std::string name;
    float x, y;
    int rotation;
    float scale_x, scale_y;
    float pivot_x, pivot_y;
    int r, g, b, a;
    int sorting_order;
};

struct PixelRequest {
    int x, y;
    int r, g, b, a;
};

// Renderer.h
class Renderer {
public:
    static void create_window(const rapidjson::Document& game_config, const rapidjson::Document &rendering_config);
    static void init_renderer(const rapidjson::Document &rendering_config);
    static void init_font(const rapidjson::Document& game_config);
    static SDL_Renderer* get_renderer() { return renderer; }
    static int get_x_resolution() { return x_resolution; }
    static int get_y_resolution() { return y_resolution; }
    static TTF_Font* get_font() { return font; }

    
    static void DrawText(std::string content, float x, float y, std::string font_name, float font_size, float r, float g, float b, float a);
    static void RenderAllText();

    // Lua API - Images
    static void Draw(std::string name, float x, float y);
    static void DrawEx(std::string name, float x, float y, float rot, float sx, float sy, float px, float py, float r, float g, float b, float a, float sort);
    
    // Lua API - UI
    static void DrawUI(std::string name, float x, float y);
    static void DrawUIEx(std::string name, float x, float y, float r, float g, float b, float a, float sort);

    // Lua API - Pixels
    static void DrawPixel(float x, float y, float r, float g, float b, float a);

    static void RenderAll();
    static void Deinit();

    static void Reset() {SDL_SetRenderDrawColor(renderer, clear_r, clear_g, clear_b, 255);}

    static void CreateDefaultParticleTextureWithName(const std::string & name);
private:
    static SDL_Window* window;
    static SDL_Renderer* renderer;
    static std::string game_title;
    static int x_resolution;
    static int y_resolution;
    static TTF_Font* font;
    
    static int clear_r;
    static int clear_g;
    static int clear_b;

    // Map: FontName -> (FontSize -> TTF_Font*)
    static std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> font_cache;

    // test case 9 stuff
    static std::vector<ImageRequest> scene_requests;
    static std::vector<ImageRequest> ui_requests;
    static std::vector<PixelRequest> pixel_requests;
    static std::vector<TextDrawRequest> text_requests;

    static std::unordered_map<std::string, SDL_Texture*> texture_cache;
    static SDL_Texture* GetTexture(std::string name);
    
};
#endif 