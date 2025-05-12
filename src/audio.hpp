// danielsinkin97@gmail.com
#pragma once

#include <SDL_mixer.h>
#include <stdexcept>
#include <string>

#include "constants.hpp"
#include "log.hpp"

namespace Audio {

inline bool initialized = false;

void init() {
    if (initialized) {
        LOG_WARN("Audio::init() called more than once — ignoring");
        return;
    }

    LOG_INFO("Initializing SDL_mixer...");

    int mix_flags = MIX_INIT_MP3;
    int initted = Mix_Init(mix_flags);
    if ((initted & mix_flags) != mix_flags) {
        throw std::runtime_error("Mix_Init failed: " + std::string(Mix_GetError()));
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        throw std::runtime_error("Mix_OpenAudio failed: " + std::string(Mix_GetError()));
    }

    initialized = true;
    LOG_INFO("Audio system initialized");
}

void shutdown() {
    if (!initialized) return;
    Mix_CloseAudio();
    Mix_Quit();
    initialized = false;
    LOG_INFO("Audio system shut down");
}

void playBeep() {
    static Mix_Chunk *beep = [] {
        Mix_Chunk *c = Mix_LoadWAV(Constants::fp_sound_beep);
        if (!c) throw std::runtime_error("Failed to load beep: " + std::string(Mix_GetError()));
        return c;
    }();

    static constexpr int beep_channel = 0;
    static bool channel_reserved = [] {
        Mix_AllocateChannels(1); // Ensure we have at least one channel
        return true;
    }();

    // Immediately halt current sound on the beep channel (if any)
    Mix_HaltChannel(beep_channel);

    if (Mix_PlayChannel(beep_channel, beep, 0) == -1) {
        LOG_WARN("Mix_PlayChannel failed: " + std::string(Mix_GetError()));
    }
}

void updateBeep(bool should_beep) {
    static Mix_Chunk *beep = [] {
        Mix_Chunk *c = Mix_LoadWAV(Constants::fp_sound_beep);
        if (!c) throw std::runtime_error("Failed to load beep: " + std::string(Mix_GetError()));
        return c;
    }();

    static constexpr int beep_channel = 0;
    static bool is_playing = false;

    if (should_beep && !is_playing) {
        if (Mix_PlayChannel(beep_channel, beep, -1) == -1) {
            LOG_WARN("Mix_PlayChannel failed: " + std::string(Mix_GetError()));
        } else {
            is_playing = true;
        }
    } else if (!should_beep && is_playing) {
        Mix_HaltChannel(beep_channel);
        is_playing = false;
    }
}
} // namespace Audio