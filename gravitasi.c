#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <omp.h>

// Konstanta Simulasi
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define NUM_PARTICLES 2500  // Naik ke 2500 agar efek ledakan/pencaran lebih dramatis
#define G_CONSTANT 0.6f     
#define TIME_STEP 0.1f      

// Struktur Data Partikel
typedef struct {
    float x, y;
    float vx, vy;
    Uint8 r, g, b;
} Particle;

// Inisialisasi awal partikel menyebar secara acak di layar
void init_particles(Particle *particles, int num_particles, float start_x, float start_y) {
    srand(time(NULL));
    for (int i = 0; i < num_particles; i++) {
        float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
        float radius = 10.0f + ((float)rand() / RAND_MAX) * 300.0f;
        
        particles[i].x = start_x + radius * cosf(angle);
        particles[i].y = start_y + radius * sinf(angle);
        
        // Kecepatan awal acak tipis
        particles[i].vx = (((float)rand() / RAND_MAX) * 4.0f - 2.0f);
        particles[i].vy = (((float)rand() / RAND_MAX) * 4.0f - 2.0f);
        
        // Gradasi warna neon
        particles[i].r = (Uint8)(100 + (i % 155));
        particles[i].g = (Uint8)(50 + (i % 100));
        particles[i].b = 255;
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL gagal diinisialisasi! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Interactive Gravity Toggle (OpenMP + SDL2)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN
    );

    if (!window) {
        printf("Window gagal dibuat! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Renderer gagal dibuat! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    float target_x = SCREEN_WIDTH / 2.0f;
    float target_y = SCREEN_HEIGHT / 2.0f;
    
    // Status Gravitasi (true = aktif, false = mati)
    bool gravity_active = true; 

    Particle* particles = (Particle*)malloc(NUM_PARTICLES * sizeof(Particle));
    init_particles(particles, NUM_PARTICLES, target_x, target_y);

    printf("Klik 1x untuk AKTIFKAN gravitasi. Klik 2x (Double Click) untuk MATIKAN gravitasi.\n");

    bool is_running = true;
    SDL_Event event;

    while (is_running) {
        // 1. Handling Input / Event
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                is_running = false;
            }
            // Mendeteksi pergerakan mouse
            else if (event.type == SDL_MOUSEMOTION) {
                target_x = (float)event.motion.x;
                target_y = (float)event.motion.y;
            }
            // Mendeteksi Klik Mouse
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    if (event.button.clicks == 1) {
                        gravity_active = true;
                    } 
                    else if (event.button.clicks == 2) {
                        gravity_active = false;
                    }
                }
            }
        }

        // 2. Kalkulasi Fisika (DIPARALELKAN DENGAN OpenMP)
        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < NUM_PARTICLES; i++) {
            
            if (gravity_active) {
                // Jarak ke kursor mouse
                float dx = target_x - particles[i].x;
                float dy = target_y - particles[i].y;
                float distance_sq = (dx * dx) + (dy * dy);
                
                if (distance_sq < 400.0f) distance_sq = 400.0f;
                float distance = sqrtf(distance_sq);

                // Hitung Gaya Gravitasi
                float force = (G_CONSTANT * 8000.0f) / distance_sq;
                float ax = force * (dx / distance);
                float ay = force * (dy / distance);

                // Update Kecepatan jika gravitasi aktif
                particles[i].vx += ax * TIME_STEP;
                particles[i].vy += ay * TIME_STEP;
                
                // Redaman standar saat mengorbit
                particles[i].vx *= 0.998f;
                particles[i].vy *= 0.998f;
            } else {
                // Saat gravitasi mati, berikan efek gesekan udara (hambatan) 
                // supaya partikel lambat laun melambat setelah terlempar, tidak hilang ke luar layar selamanya
                particles[i].vx *= 0.995f;
                particles[i].vy *= 0.995f;
            }

            // Update Posisi (Selalu berjalan agar objek tetap bergerak lurus secara inersia)
            particles[i].x += particles[i].vx * TIME_STEP;
            particles[i].y += particles[i].vy * TIME_STEP;

            // Fitur tambahan: Boundary Check (memantul di batas layar agar partikel tidak hilang)
            if (particles[i].x < 0 || particles[i].x > SCREEN_WIDTH)  particles[i].vx *= -1.0f;
            if (particles[i].y < 0 || particles[i].y > SCREEN_HEIGHT) particles[i].vy *= -1.0f;
        }

        // 3. Rendering / Visualisasi
        SDL_SetRenderDrawColor(renderer, 10, 10, 15, 255); 
        SDL_RenderClear(renderer);

        // Gambar indikator pusat gravitasi di posisi kursor
        if (gravity_active) {
            // Hijau Neon jika gravitasi aktif
            SDL_SetRenderDrawColor(renderer, 0, 255, 150, 255);
            SDL_Rect mouse_point = {(int)target_x - 6, (int)target_y - 6, 12, 12};
            SDL_RenderFillRect(renderer, &mouse_point);
        } else {
            // Merah Redup berbentuk kotak kosong jika gravitasi mati
            SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
            SDL_Rect mouse_point = {(int)target_x - 6, (int)target_y - 6, 12, 12};
            SDL_RenderDrawRect(renderer, &mouse_point);
        }

        // Gambar partikel
        for (int i = 0; i < NUM_PARTICLES; i++) {
            SDL_SetRenderDrawColor(renderer, particles[i].r, particles[i].g, particles[i].b, 255);
            SDL_RenderDrawPoint(renderer, (int)particles[i].x, (int)particles[i].y);
            SDL_RenderDrawPoint(renderer, (int)particles[i].x + 1, (int)particles[i].y);
        }

        SDL_RenderPresent(renderer);
    }

    free(particles);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}