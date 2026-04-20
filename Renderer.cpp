#include "Renderer.h"

// Renderer.cpp - add these definitions at the top
SDL_Window* Renderer::window = nullptr;
SDL_Renderer* Renderer::renderer = nullptr;
std::string Renderer::game_title = "";
int Renderer::x_resolution = 640;
int Renderer::y_resolution = 360;
TTF_Font* Renderer::font = nullptr;
std::vector<TextDrawRequest> Renderer::text_requests;
std::unordered_map<std::string, SDL_Texture*> Renderer::texture_cache;

int Renderer::clear_r = 255;
int Renderer::clear_g = 255;
int Renderer::clear_b = 255;

std::vector<ImageRequest> Renderer::scene_requests;
std::vector<ImageRequest> Renderer::ui_requests;
std::vector<PixelRequest> Renderer::pixel_requests;

// Map: FontName -> (FontSize -> TTF_Font*)
std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> Renderer::font_cache;
void Renderer::create_window(const rapidjson::Document &game_config, const rapidjson::Document &rendering_config) {

    if (game_config.HasMember("game_title")){
        game_title = game_config["game_title"].GetString();
    }
    if (rendering_config.IsObject()){
        if (rendering_config.HasMember("x_resolution")){
            x_resolution = rendering_config["x_resolution"].GetInt();
        }
        if (rendering_config.HasMember("y_resolution")){
            y_resolution = rendering_config["y_resolution"].GetInt();
        }
    }
    
    window = Helper::SDL_CreateWindow(game_title.c_str(), 0, 0, x_resolution, y_resolution, 0);
}

void Renderer::init_renderer(const rapidjson::Document& rendering_config){
    renderer = Helper::SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    
    if (rendering_config.IsObject()){
        if (rendering_config.HasMember("clear_color_r")){
            clear_r = rendering_config["clear_color_r"].GetInt();
        }
        if (rendering_config.HasMember("clear_color_g")){
            clear_g = rendering_config["clear_color_g"].GetInt();
        }
        if (rendering_config.HasMember("clear_color_b")){
            clear_b = rendering_config["clear_color_b"].GetInt();
        }        
    }
    
    SDL_SetRenderDrawColor(renderer, clear_r, clear_g, clear_b, 255);

    SDL_RenderClear(renderer);
}

void Renderer::init_font(const rapidjson::Document& game_config){
    if (game_config.HasMember("font") && game_config["font"].IsString()){
        std::string font_name = game_config["font"].GetString(); 
        std::filesystem::path font_path = "resources/fonts/" + font_name + ".ttf";

        if (!std::filesystem::exists(font_path)) {
                std::cout << "error: font " << font_name << " missing";
                exit(0);
            }
        std::string path_str = font_path.string();
        font = TTF_OpenFont(path_str.c_str(), 16);
    } else {
        std::cout << "error: text render failed. No font configured";
        exit(0);
    }
}

void Renderer::DrawText(std::string content, float x, float y, std::string font_name, float font_size, float r, float g, float b, float a) {
    TextDrawRequest req;
    req.content = content;
    req.x = static_cast<int>(x);
    req.y = static_cast<int>(y);
    req.font_name = font_name;
    req.font_size = static_cast<int>(font_size);
    req.color = { (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a };

    text_requests.push_back(req);
}

void Renderer::RenderAllText() {
    for (const auto& req : text_requests) {
        // 1. Check/Load Font from cache
        if (font_cache[req.font_name].find(req.font_size) == font_cache[req.font_name].end()) {
            std::string path = "resources/fonts/" + req.font_name + ".ttf";
            if (!std::filesystem::exists(path)) {
                std::cout << "error: font " << req.font_name << " missing";
                exit(0);
            }
            font_cache[req.font_name][req.font_size] = TTF_OpenFont(path.c_str(), req.font_size);
        }

        TTF_Font* target_font = font_cache[req.font_name][req.font_size];

        // 2. Create Surface and Texture
        SDL_Surface* surface = TTF_RenderText_Solid(target_font, req.content.c_str(), req.color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

        float width, height;
        Helper::SDL_QueryTexture(texture, &width, &height);
        SDL_FRect dest_rect = { static_cast<float>(req.x), static_cast<float>(req.y), width, height };

        // 3. Draw to Screen
        Helper::SDL_RenderCopy(renderer, texture, NULL, &dest_rect);

        // 4. Cleanup temporary resources
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }
    // Clear requests for the next frame
    text_requests.clear();
}

void Renderer::Draw(std::string name, float x, float y) {
    DrawEx(name, x, y, 0, 1, 1, 0.5f, 0.5f, 255, 255, 255, 255, 0);
}

void Renderer::DrawEx(std::string name, float x, float y, float rot, float sx, float sy, float px, float py, float r, float g, float b, float a, float sort) {
    scene_requests.push_back({name, x, y, (int)rot, sx, sy, px, py, (int)r, (int)g, (int)b, (int)a, (int)sort});
}

void Renderer::DrawUI(std::string name, float x, float y) {
    DrawUIEx(name, x, y, 255, 255, 255, 255, 0);
}

void Renderer::DrawUIEx(std::string name, float x, float y, float r, float g, float b, float a, float sort) {
    ui_requests.push_back({name, x, y, 0, 1, 1, 0.0f, 0.0f, (int)r, (int)g, (int)b, (int)a, (int)sort});
}

void Renderer::DrawPixel(float x, float y, float r, float g, float b, float a) {
    pixel_requests.push_back({(int)x, (int)y, (int)r, (int)g, (int)b, (int)a});
}

void Renderer::RenderAll() {
    // 1. Sort Layers by sorting_order
    auto sort_func = [](const ImageRequest& a, const ImageRequest& b) {
        return a.sorting_order < b.sorting_order;
    };
    std::stable_sort(scene_requests.begin(), scene_requests.end(), sort_func);
    std::stable_sort(ui_requests.begin(), ui_requests.end(), sort_func);

    // 2. Scene-space images
    for (auto& req : scene_requests) {
        SDL_Texture* tex = GetTexture(req.name);
        float w, h;
        Helper::SDL_QueryTexture(tex, &w, &h);

        float abs_sx = std::abs(req.scale_x);
        float abs_sy = std::abs(req.scale_y);

        SDL_FPoint pivot;
        pivot.x = req.pivot_x * w * abs_sx;
        pivot.y = req.pivot_y * h * abs_sy;

        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (req.scale_x < 0) flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
        if (req.scale_y < 0) flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);

        float zoom = Camera::GetZoom();
        SDL_FRect dest = {
            ((req.x * 100.0f - pivot.x) + (x_resolution / 2.0f) - Camera::GetPositionX() * 100.0f) * zoom + (x_resolution / 2.0f) * (1.0f - zoom),
            ((req.y * 100.0f - pivot.y) + (y_resolution / 2.0f) - Camera::GetPositionY() * 100.0f) * zoom + (y_resolution / 2.0f) * (1.0f - zoom),
            (w * abs_sx * zoom),
            (h * abs_sy * zoom)
        };
        SDL_SetTextureColorMod(tex, req.r, req.g, req.b);
        SDL_SetTextureAlphaMod(tex, req.a);
        Helper::SDL_RenderCopyEx(0, "", renderer, tex, NULL, &dest, req.rotation, &pivot, flip);
        SDL_SetTextureColorMod(tex, 255, 255, 255); // Reset
        SDL_SetTextureAlphaMod(tex, 255);
    }

    // 3. UI Images (Screen Space)
    for (auto& req : ui_requests) {
        SDL_Texture* tex = GetTexture(req.name);
        float w, h;
        Helper::SDL_QueryTexture(tex, &w, &h);
        SDL_FRect dest = { req.x, req.y, w, h };
        
        SDL_SetTextureColorMod(tex, req.r, req.g, req.b);
        SDL_SetTextureAlphaMod(tex, req.a);
        Helper::SDL_RenderCopy(renderer, tex, NULL, &dest);
        SDL_SetTextureColorMod(tex, 255, 255, 255);
        SDL_SetTextureAlphaMod(tex, 255);
    }

    // 4. Text (Previously implemented RenderAllText logic)
    RenderAllText();

    // 5. Pixels (Top-most)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (auto& p : pixel_requests) {
        SDL_SetRenderDrawColor(renderer, p.r, p.g, p.b, p.a);
        SDL_RenderDrawPoint(renderer, p.x, p.y);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Clear all for next frame
    scene_requests.clear();
    ui_requests.clear();
    pixel_requests.clear();
    text_requests.clear();
}

// Initialize the static member at the top of Renderer.cpp


SDL_Texture* Renderer::GetTexture(std::string name) {
    // 1. Check if it's already in the cache
    auto it = texture_cache.find(name);
    if (it != texture_cache.end()) {
        return it->second;
    }

    // 2. Not in cache? Construct the path and load it
    // Assuming standard .png extension for your project
    std::filesystem::path image_path = "resources/images/" + name + ".png";
    
    if (!std::filesystem::exists(image_path)) {
        std::cout << "error: missing image " << name;
        exit(0);
    }

    // Use your existing Helper wrapper for SDL_image
    SDL_Texture* texture = IMG_LoadTexture(renderer, image_path.string().c_str());
    
    if (!texture) {
        std::cout << "error: failed to create texture from " << name;
        exit(0);
    }

    // 3. Store and return
    texture_cache[name] = texture;
    return texture;
}

void Renderer::CreateDefaultParticleTextureWithName(const std::string & name){
    if (texture_cache.find(name) != texture_cache.end()){
        return;
    }

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 8, 8, 32, SDL_PIXELFORMAT_RGBA8888);

    Uint32 white_color = SDL_MapRGBA(surface->format, 255, 255, 255, 255);
    SDL_FillRect(surface, NULL, white_color);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(Renderer::get_renderer(), surface);

    SDL_FreeSurface(surface);
    texture_cache[name] = texture;
}

void Renderer::Deinit() {
    // Free all textures
    for (auto const& [name, tex] : texture_cache) {
        SDL_DestroyTexture(tex);
    }
    texture_cache.clear();

    // Free all fonts
    for (auto& [font_name, size_map] : font_cache) {
        for (auto& [size, font_ptr] : size_map) {
            TTF_CloseFont(font_ptr);
        }
    }
    font_cache.clear();
}