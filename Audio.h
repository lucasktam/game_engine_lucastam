#ifndef AUDIO_H
#define AUDIO_H

#include "AudioHelper.h"
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/filereadstream.h"
#include <string>
#include <vector>
#include <filesystem>
class Audio {
public:
    static void init(const rapidjson::Document &game_config);
    
    // Lua API
    static void Play(int channel, std::string clip_name, bool does_loop);
    static void Halt(int channel);
    static void SetVolume(int channel, float volume);

private:
    static inline std::unordered_map<std::string, Mix_Chunk*> audio_cache;
};
#endif