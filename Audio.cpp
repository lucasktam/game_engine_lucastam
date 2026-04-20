#include "Audio.h"

void Audio::init(const rapidjson::Document &game_config) {
    // Initialize SDL_mixer with 50 channels as requested
    AudioHelper::Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    AudioHelper::Mix_AllocateChannels(50);
}

void Audio::Play(int channel, std::string clip_name, bool does_loop) {
    Mix_Chunk* chunk = nullptr;

    // 1. Check Cache
    if (audio_cache.count(clip_name)) {
        chunk = audio_cache[clip_name];
    } else {
        // 2. Try to load .wav then .ogg
        std::string base_path = "resources/audio/" + clip_name;
        std::string wav_path = base_path + ".wav";
        std::string ogg_path = base_path + ".ogg";

        if (std::filesystem::exists(wav_path)) {
            chunk = AudioHelper::Mix_LoadWAV(wav_path.c_str());
        } else if (std::filesystem::exists(ogg_path)) {
            chunk = AudioHelper::Mix_LoadWAV(ogg_path.c_str());
        }

        if (!chunk) {
            std::cout << "error: failed to play audio clip " << clip_name << std::endl;
            return; // Fail gracefully or exit(0) per your project style
        }
        audio_cache[clip_name] = chunk;
    }

    // 3. Play on channel. loop: true -> -1, false -> 0
    int loops = does_loop ? -1 : 0;
    AudioHelper::Mix_PlayChannel(channel, chunk, loops);
}

void Audio::Halt(int channel) {
    AudioHelper::Mix_HaltChannel(channel);
}

void Audio::SetVolume(int channel, float volume) {
    // Downcast float to int (0-128)
    int vol = static_cast<int>(volume);
    AudioHelper::Mix_Volume(channel, vol);
}

