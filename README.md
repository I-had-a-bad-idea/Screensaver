# Screensaver

A simple fullscreen particle-based screensaver written in C++ using SDL2.

The program simulates particles that are attracted to several moving "gravity points", producing colorful motion trails. It is configured for high particle counts and uses blending + a fading render pass to produce smooth trails.

## Features

- Fullscreen desktop mode (uses current display resolution)
- Particle attraction to configurable number of gravity points
- Motion trails via alpha-blended fade
- Color cycling over time
- Lightweight single-file implementation: `screensaver.cpp`

<img src="Screenshot.png" alt="Image" width="700" height="400">

## Dependencies

- Install SDL2 (development) and ensure the include/lib paths are available to your compiler.

The project does not use external build tools; it compiles with g++.

## Build (Windows / PowerShell)

This repository includes a VS Code build task that uses a MinGW/MSYS g++ toolchain to compile the active file with SDL2 and produce a screensaver executable (`.scr`). It expects SDL2 headers/libs installed under your MSYS/MinGW installation (the task uses `C:\msys64\mingw64` paths by default).

If you prefer to build manually from PowerShell, an example command (adjust include/lib paths to match your SDL2 install) looks like:

```powershell
C:\msys64\ucrt64\bin\g++.exe -O3 -ffast-math -fopenmp -flto -DNDEBUG -s -I C:/msys64/mingw64/include \
		screensaver.cpp -L C:/msys64/mingw64/lib -lmingw32 -finline-functions -lopengl32 -lgdi32 -lSDL2main -lSDL2 -lwinmm -o Screensaver.scr
```

Alternatively run the VS Code task named "C/C++: g++.exe build active file with SDL (Release)" from the Run/Tasks menu.

Right click on the binary, and choose install. Now it is configured as the standard screensaver.

## Run

- The compiled binary opens in fullscreen and runs until you press any key, click the mouse, or close the window.
- Command-line arguments: `screensaver <num_gravity_points> <num_particles>`
	- If no arguments are provided the defaults are used (5 gravity points, 25000 particles).
	- Example: `Screensaver.scr 8 50000` (8 gravity points, 50k particles)

## Controls

- Any key or mouse button: exits the program
- Moving gravity points are autonomous; there are no interactive controls in the current version

## Tuning & Performance

- `NUM_PARTICLES` (second CLI argument) mainly controls workload. Lower the number for slower/older hardware.
- `NUMBER_GRAVITY_POINTS` (first CLI argument) changes visual complexity. Reduce to increase speed.
- The code is compiled with optimizations in the provided build task (`-O3 -flto -ffast-math`) and uses `SDL_RENDERER_ACCELERATED` with vsync.
- If the renderer is slow on your system, try reducing particles/gravity points or disabling vsync in the renderer creation flags.

Suggested edits to experiment with:
- Reduce `NUM_PARTICLES` (e.g., 10000) or `NUMBER_GRAVITY_POINTS` (e.g., 2) for better frame rates.
- Increase `NEAR_RADIUS` and `RESPAWN_TIME` to change how particles stick to gravity points.
- Tweak `G` and `DAMP` constants in `screensaver.cpp` to change attraction strength and damping.

## Troubleshooting

- Black/blank window or immediate exit: make sure SDL2 runtime DLLs (e.g., `SDL2.dll`) are available on PATH or next to the executable.
- Compiler errors about missing headers/libraries: install the SDL2 development package and adjust include/lib paths in the build command or VS Code task.
- If compilation fails due to missing OpenMP or other flags, you can remove `-fopenmp` from the command line, though it may slightly affect performance characteristics.

## Files

- `screensaver.cpp` — main source implementing the particle system and rendering loop
- `README.md` — this file
- `LICENSE` — project license (see file)

## License

This repository includes a `LICENSE` file. See it for the full license text.

Enjoy the screensaver!

