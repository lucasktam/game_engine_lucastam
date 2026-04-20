#include "Input.h"


/*
    static inline std::unordered_map<SDL_Scancode, INPUT_STATE> keyboard_states;
	static inline std::vector<SDL_Scancode> just_became_down_scancodes;
	static inline std::vector<SDL_Scancode> just_became_up_scancodes;
    INPUT_STATE { INPUT_STATE_UP, INPUT_STATE_JUST_BECAME_DOWN, INPUT_STATE_DOWN, INPUT_STATE_JUST_BECAME_UP };
*/

// Call before main loop begins.
void Input::Init(){
    
    keyboard_states.clear();
    just_became_down_scancodes.clear();
    just_became_up_scancodes.clear();

    mouse_button_states.clear();
    just_became_down_buttons.clear();
    just_became_up_buttons.clear();
    mouse_scroll_this_frame = 0;
}

// Call every frame at start of event loop.
void Input::ProcessEvent(const SDL_Event & e){
    if (e.type == SDL_KEYDOWN)
    {
        SDL_Scancode key = e.key.keysym.scancode;
        keyboard_states[key] = INPUT_STATE_JUST_BECAME_DOWN;
        just_became_down_scancodes.push_back(key);
    }
    else if (e.type == SDL_KEYUP)
    {
        SDL_Scancode key = e.key.keysym.scancode;
        keyboard_states[key] = INPUT_STATE_JUST_BECAME_UP;
        just_became_up_scancodes.push_back(key);
    }
    else if (e.type == SDL_MOUSEBUTTONDOWN)
    {
        int button = e.button.button;
        mouse_button_states[button] = INPUT_STATE_JUST_BECAME_DOWN;
        just_became_down_buttons.push_back(button);
    }
    else if (e.type == SDL_MOUSEBUTTONUP)
    {
        int button = e.button.button;
        mouse_button_states[button] = INPUT_STATE_JUST_BECAME_UP;
        just_became_up_buttons.push_back(button);
    }
    else if (e.type == SDL_MOUSEMOTION)
    {
        mouse_position.x = e.motion.x;
        mouse_position.y = e.motion.y;
    }
    else if (e.type == SDL_MOUSEWHEEL)
    {
        mouse_scroll_this_frame = e.wheel.preciseY;
    }
}

void Input::LateUpdate(){
    for (SDL_Scancode key : just_became_down_scancodes)
        keyboard_states[key] = INPUT_STATE_DOWN;
    for (SDL_Scancode key : just_became_up_scancodes)
        keyboard_states[key] = INPUT_STATE_UP;
    just_became_down_scancodes.clear();
    just_became_up_scancodes.clear();

    for (int button : just_became_down_buttons)
        mouse_button_states[button] = INPUT_STATE_DOWN;
    for (int button : just_became_up_buttons)
        mouse_button_states[button] = INPUT_STATE_UP;
    just_became_down_buttons.clear();
    just_became_up_buttons.clear();

    mouse_scroll_this_frame = 0;
}

bool Input::GetKey(const std::string& k)
{   
    auto map_it = __keycode_to_scancode.find(k);
    if (map_it == __keycode_to_scancode.end()) return false;

    auto it = keyboard_states.find(map_it->second);
    if (it == keyboard_states.end()) return false;

    return it->second == INPUT_STATE_DOWN ||
           it->second == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetKeyDown(const std::string& k)
{
    auto map_it = __keycode_to_scancode.find(k);
    if (map_it == __keycode_to_scancode.end()) return false;

    auto it = keyboard_states.find(map_it->second);
    if (it == keyboard_states.end()) return false;

    return it->second == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetKeyUp(const std::string& k)
{
    auto map_it = __keycode_to_scancode.find(k);
    if (map_it == __keycode_to_scancode.end()) return false;

    auto it = keyboard_states.find(map_it->second);
    if (it == keyboard_states.end()) return false;

    return it->second == INPUT_STATE_JUST_BECAME_UP;
}

// --- Mouse ---

glm::vec2 Input::GetMousePosition()
{
    return mouse_position;
}

bool Input::GetMouseButton(int button)
{
    auto it = mouse_button_states.find(button);
    if (it == mouse_button_states.end()) return false;
    return it->second == INPUT_STATE_DOWN || it->second == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetMouseButtonDown(int button)
{
    auto it = mouse_button_states.find(button);
    if (it == mouse_button_states.end()) return false;
    return it->second == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetMouseButtonUp(int button)
{
    auto it = mouse_button_states.find(button);
    if (it == mouse_button_states.end()) return false;
    return it->second == INPUT_STATE_JUST_BECAME_UP;
}

float Input::GetMouseScrollDelta()
{
    return mouse_scroll_this_frame;
}

void Input::HideCursor(){
    int result = SDL_ShowCursor(SDL_DISABLE);
}

void Input::ShowCursor(){
    int result = SDL_ShowCursor(SDL_ENABLE);
}