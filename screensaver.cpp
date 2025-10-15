#include <SDL2/SDL.h>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <algorithm>

struct Particle {
    float x, y;
    float vx, vy;
    float nearTime = 0.0f; // seconds near the gravity point
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <num_particles>\n", argv[0]);
        return 1;
    }

    int NUM_PARTICLES = std::atoi(argv[1]);
    if (NUM_PARTICLES <= 0) {
        printf("Error: Number of particles must be a positive integer.\n");
        return 1;
    }
    const float G = 500.0f;
    const float DAMP = 0.99f;
    const float NEAR_RADIUS = 10.0f;
    const float RESPAWN_TIME = 0.5f;
    const float DELTA_TIME = 0.016f; // ~60 FPS
    const int SCREEN_W = 1920;
    const int SCREEN_H = 1080;
    const float MARGIN = 5.0f;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Gravity Particles",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_W, SCREEN_H, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Enable blending for trail effect
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

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
    float cx = SCREEN_W / 2.0f, cy = SCREEN_H / 2.0f;
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

        // Bounce off screen edges
        if(cx < MARGIN) { cx = MARGIN; cvx = -cvx; }
        if(cx > SCREEN_W - MARGIN) { cx = SCREEN_W - MARGIN; cvx = -cvx; }
        if(cy < MARGIN) { cy = MARGIN; cvy = -cvy; }
        if(cy > SCREEN_H - MARGIN) { cy = SCREEN_H - MARGIN; cvy = -cvy; }

        // Motion trail: fade screen instead of clearing it
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 25); // Low alpha = long trails
        SDL_RenderFillRect(renderer, nullptr);

        // Update particles
        for(auto& p : particles){
            float dx = cx - p.x;
            float dy = cy - p.y;
            float dist2 = dx * dx + dy * dy + 100.0f; // Softening
            float invd = 1.0f / std::sqrt(dist2);
            float ax = G * dx * invd / dist2 + ((rand() / (float)RAND_MAX) - 0.5f) * 0.01f;
            float ay = G * dy * invd / dist2 + ((rand() / (float)RAND_MAX) - 0.5f) * 0.01f;

            p.vx = (p.vx + ax) * DAMP;
            p.vy = (p.vy + ay) * DAMP;
            p.x += p.vx;
            p.y += p.vy;

            // Track time near gravity point
            if (std::sqrt(dx * dx + dy * dy) < NEAR_RADIUS)
                p.nearTime += DELTA_TIME;
            else
                p.nearTime = 0.0f;

            // Respawn if stuck
            if(p.nearTime > RESPAWN_TIME) {
                p.x = rand() % SCREEN_W;
                p.y = rand() % SCREEN_H;
                p.vx = ((rand() / (float)RAND_MAX) * 2 - 1);
                p.vy = ((rand() / (float)RAND_MAX) * 2 - 1);
                p.nearTime = 0.0f;
            }

            // Respawn if offscreen
            if (p.x < -10 || p.x > SCREEN_W + 10 || p.y < -10 || p.y > SCREEN_H + 10) {
                p.x = rand() % SCREEN_W;
                p.y = rand() % SCREEN_H;
                p.vx = ((rand() / (float)RAND_MAX) * 2 - 1);
                p.vy = ((rand() / (float)RAND_MAX) * 2 - 1);
                p.nearTime = 0.0f;
            }
        }

        // Collect particle positions
        std::vector<SDL_Point> sdlPoints;
        sdlPoints.reserve(particles.size());
        for (const auto& p : particles) {
            sdlPoints.push_back({ (int)p.x, (int)p.y });
        }

        // Draw all particles in white
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawPoints(renderer, sdlPoints.data(), (int)sdlPoints.size());

        SDL_RenderPresent(renderer);
        SDL_Delay((int)(DELTA_TIME * 1000));
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
