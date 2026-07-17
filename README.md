# 622023007_FANHAS_TR-PEMPAR
tugas rancang pemrosesan paralel fanhas mohammad varres nim 622023007 semester 6

### SIMULASI INTERAKTIF 2D PARTICLE SYSTEM BERBASIS GAYA GRAVITASI PUSAT DENGAN OPENMP DAN SDL2

---

## 1. Identitas Praktikan

*   **Nama Lengkap** : [Fanhas Mohammad Varres]
*   **NIM**          : [622023007]

---

## 2. Setup Development Environment

### A. Pemasangan Dependensi
Sebelum melakukan kompilasi mandiri, pastikan pustaka grafis SDL2 dan runtime compiler GCC pendukung OpenMP telah terpasang:

#### 1. Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential libsdl2-dev
```

---

```mermaid
graph TD
    Start([START]) --> InitSDL[Inisialisasi SDL2, Window, Renderer & gravity_active = true]
    InitSDL --> InitParticles[Alokasi Memori & Init 2500 Partikel]
    InitParticles --> LoopCheck{Apakah Loop Utama Berjalan?}

    LoopCheck -- Ya --> PollEvent[SDL_PollEvent]
    LoopCheck -- Tidak --> CleanUp[Bebaskan Memori & SDL_Quit]
    CleanUp --> End([END])

    PollEvent --> EventType{Cek Jenis Event}

    EventType -- MOUSEMOTION --> UpdateTarget[Update target_x & target_y]
    EventType -- MOUSEBUTTONDOWN --> CheckClicks{Cek Clicks}
    EventType -- QUIT --> SetQuit[is_running = false]

    CheckClicks -- 1x Click --> SetGravTrue[gravity_active = true]
    CheckClicks -- 2x Click --> SetGravFalse[gravity_active = false]

    UpdateTarget --> ParallelRegion
    SetGravTrue --> ParallelRegion
    SetGravFalse --> ParallelRegion
    SetQuit --> ParallelRegion

    subgraph OpenMP Parallel Region [Komputasi Paralel Multi-Threading]
        ParallelRegion[#pragma omp parallel for schedule dynamic] --> LoopParticles{gravity_active == true?}
        LoopParticles -- Ya --> CalcGravity[Hitung Gravitasi, Kecepatan & Damping 0.998]
        LoopParticles -- Tidak --> CalcInertia[Set Gaya = 0, Hambatan Udara 0.995]
        CalcGravity --> UpdatePos[Update Posisi & Handle Pantulan Dinding]
        CalcInertia --> UpdatePos
    end

    UpdatePos --> RenderZone

    subgraph Rendering Zone [Main Thread / Serial Execution]
        RenderZone[Clear Screen -> Gambar Target Kursor -> Gambar Partikel -> SDL_RenderPresent]
    end

    RenderZone --> LoopCheck
```
---
