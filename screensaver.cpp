#include <SDL2/SDL.h>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <ctime>

struct Particle {
    float x, y;
    float vx, vy;
};

int main(int argc, char* argv[]) {
    const int NUM_PARTICLES = 10000;
    const float G = 500.0f;
    const float DAMP = 0.99f;
    const int SCREEN_W = 1920;
    const int SCREEN_H = 1080;
    const float MARGIN = 50.0f;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Gravity Particles",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_W, SCREEN_H, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Initialize particles
    std::vector<Particle> particles(NUM_PARTICLES);
    for(auto &p : particles) {
        p.x = rand() % SCREEN_W;
        p.y = rand() % SCREEN_H;
        p.vx = ((rand() / (float)RAND_MAX) * 2 - 1);
        p.vy = ((rand() / (float)RAND_MAX) * 2 - 1);
    }

    // Gravity point
    float cx = SCREEN_W/2.0f, cy = SCREEN_H/2.0f;
    float cvx = ((rand() / (float)RAND_MAX) * 6 - 3);
    float cvy = ((rand() / (float)RAND_MAX) * 6 - 3);

    bool running = true;
    SDL_Event event;

    while(running) {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT || event.type == SDL_KEYDOWN)
                running = false;
        }

        // Move gravity point
        cx += cvx;
        cy += cvy;
        if(cx < MARGIN || cx > SCREEN_W - MARGIN) { cvx = -cvx*0.9f; cx = std::fmax(MARGIN, std::fmin(SCREEN_W - MARGIN, cx)); }
        if(cy < MARGIN || cy > SCREEN_H - MARGIN) { cvy = -cvy*0.9f; cy = std::fmax(MARGIN, std::fmin(SCREEN_H - MARGIN, cy)); }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);


        // Update and draw particles
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for(auto &p : particles) {
            float dx = cx - p.x;
            float dy = cy - p.y;
            float dist2 = dx*dx + dy*dy + 100.0f;
            float invd = 1.0f / std::sqrt(dist2);
            float ax = G * dx * invd / dist2;
            float ay = G * dy * invd / dist2;

            // Small random noise
            ax += ((rand() / (float)RAND_MAX) - 0.5f) * 0.01f;
            ay += ((rand() / (float)RAND_MAX) - 0.5f) * 0.01f;

            p.vx = (p.vx + ax) * DAMP;
            p.vy = (p.vy + ay) * DAMP;
            p.x += p.vx;
            p.y += p.vy;

            // Keep particle in screen with smooth respawn
            if(p.x < -10 || p.x > SCREEN_W+10 || p.y < -10 || p.y > SCREEN_H+10) {
                p.x = rand() % SCREEN_W;
                p.y = rand() % SCREEN_H;
                p.vx = ((rand() / (float)RAND_MAX) * 2 - 1);
                p.vy = ((rand() / (float)RAND_MAX) * 2 - 1);
            }

            SDL_RenderDrawPoint(renderer, (int)p.x, (int)p.y);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
