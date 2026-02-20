#include <SDL2/SDL.h>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <string>

struct Particle {
    float x, y;
    float vx, vy;
    float nearTime = 0.0f; // seconds near the gravity point
};

struct GravityPoint {
    float x, y;
    float vx, vy;
    float g; // The gravity of the point
};

struct Config {
    int gravity_points = 5;
    int particles = 25000;

    bool cycle_color = true;
    int color_r = 255;
    int color_g = 255;
    int color_b = 255;

    int trail_alpha = 25;

    float g = 500.0f;
    float damp = 0.99f;
};

Config load_config(const std::string& path) {
    Config config;
    std::ifstream file(path);

    if (!file.is_open()) {
        printf("Config file not found. Using defaults.\n");
        return config;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t eq = line.find("=");
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        try {
            int v = std::stoi(value);
            if (key == "gravity_points" && v > 0) {
                config.gravity_points = v;
            } else if (key == "particles" && v > 0) {
                config.particles = v;
            
            } else if (key == "cycle_color" && v == 1) {
                config.cycle_color = true;
            } else if (key == "cycle_color" && v == 0) {
                config.cycle_color = false;
            
            } else if (key == "color_r" && v >= 0 && v <= 255) {
                config.color_r = v;
            } else if (key == "color_g" && v >= 0 && v <= 255) {
                config.color_g = v;
            } else if (key == "color_b" && v >= 0 && v <= 255) {
                config.color_b = v;
            
            } else if (key == "trail_alpha" && v >= 0 && v <= 255) {
                config.trail_alpha = v;
            
            } else if (key == "g" && v > 0) {
                config.g = v;
            } else if (key == "damp" && v >= 0 && v <= 100) {
                config.damp = v / 100.0f;
            }
        } catch (...) {
            // Ignore invalid values
        }
    }
    return config;
}

int main(int argc, char* argv[]) {
    Config config = load_config("screensaver.config");

    const float MAX_GRAVITY_DISTANCE = 100000.0f;
    const float NEAR_RADIUS = 10.0f;
    const float RESPAWN_TIME = 0.5f;
    const float DELTA_TIME = 0.016f; // ~60 FPS
    const float MARGIN = 5.0f;

    SDL_Init(SDL_INIT_VIDEO);

    // Get actual display resolution
    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);
    int SCREEN_W = displayMode.w;
    int SCREEN_H = displayMode.h;

    SDL_Window* window = SDL_CreateWindow("Gravity Particles",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    
    // Enable blending for trail effect
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Initialize particles
    std::vector<Particle> particles(config.particles);
    std::vector<GravityPoint> gravity_points(config.gravity_points);
    std::vector<SDL_Point> sdlPoints(particles.size());
    for(auto &p : particles) {
        p.x = rand() % SCREEN_W;
        p.y = rand() % SCREEN_H;
        p.vx = ((rand() / (float)RAND_MAX) * 2 - 1);
        p.vy = ((rand() / (float)RAND_MAX) * 2 - 1);
    }

    // Gravity points
    for(auto &g : gravity_points){
        g.x = SCREEN_W / 2.0f;
        g.y = SCREEN_H / 2.0f;
        g.vx = ((rand() / (float)RAND_MAX) * 6 - 1);
        g.vy = ((rand() / (float)RAND_MAX) * 6 - 1);
        g.g = ((rand() / (float)RAND_MAX) * config.g);
    }

    bool running = true;
    SDL_Event event;

    while(running) {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT || event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN)
                running = false;
        }

        // Move gravity points
        for(auto &g : gravity_points){
            g.x += g.vx;
            g.y += g.vy;
                    // Bounce off screen edges
            if(g.x < MARGIN) { g.x = MARGIN; g.vx = -g.vx; }
            if(g.x > SCREEN_W - MARGIN) { g.x = SCREEN_W - MARGIN; g.vx = -g.vx; }
            if(g.y < MARGIN) { g.y = MARGIN; g.vy = -g.vy; }
            if(g.y > SCREEN_H - MARGIN) { g.y = SCREEN_H - MARGIN; g.vy = -g.vy; }
        }



        // Motion trail: fade screen instead of clearing it
        // Low alpha = long trails
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, config.trail_alpha);
        SDL_RenderFillRect(renderer, nullptr);

        // Update particles

        for(auto& p : particles){
            float ax = 0.0f, ay = 0.0f;
            for(auto &g : gravity_points){
                float dx = g.x - p.x;
                float dy = g.y - p.y;
                float dist2 = dx * dx + dy * dy + 100.0f; // Softening
                
                if (dist2 > MAX_GRAVITY_DISTANCE){
                    continue;
                }

                float invd = 1.0f / std::sqrt(dist2);
                ax += g.g * dx * invd / dist2 + ((rand() / (float)RAND_MAX) - 0.5f) * 0.01f;
                ay += g.g * dy * invd / dist2 + ((rand() / (float)RAND_MAX) - 0.5f) * 0.01f;

                // Track time near gravity point 
                if ((dx * dx + dy * dy) < NEAR_RADIUS * NEAR_RADIUS){
                    p.nearTime += DELTA_TIME;
                }
            }
            p.vx = (p.vx + ax) * config.damp;
            p.vy = (p.vy + ay) * config.damp;
            p.x += p.vx;
            p.y += p.vy;
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
        
        // Colors
        int r, g, b;

        if (config.cycle_color) {
            // Color cycling over time
            float t = SDL_GetTicks() * 0.001f; // time in seconds
            r = (int)(128 + 127 * std::sin(t * 0.5));
            g = (int)(128 + 127 * std::sin(t * 0.1));
            b = (int)(128 + 127 * std::sin(t * 0.9));
        } else {
            r = config.color_r;
            g = config.color_g;
            b = config.color_b;
        }
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);


        // Prepare and draw points
        for (size_t i = 0; i < particles.size(); ++i) {
            sdlPoints[i].x = (int)particles[i].x;
            sdlPoints[i].y = (int)particles[i].y;
        }
        SDL_RenderDrawPoints(renderer, sdlPoints.data(), (int)sdlPoints.size());

        SDL_RenderPresent(renderer);
        SDL_Delay((int)(DELTA_TIME * 1000));
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_Quit();
    return 0;
}
