#pragma once

#include <SDL.h>
#include <cstdint>

class Platform {
public:
    // Constructor
    Platform(const char* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight);

    // Destructor
    ~Platform();

    // Update the texture and render it
    void Update(const void* buffer, int pitch);

    // Process input and update key states
    bool ProcessInput(uint8_t* keys);

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
};