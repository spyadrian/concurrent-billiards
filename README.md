# Concurrent Billiards

A billiards simulation where **every ball runs in its own thread** – hepia, Concurrent Programming, 2024.
Team project: **Adrian Spycher** & Jad Tayan.

## Overview

- One **POSIX thread** per ball, plus threads for display and keyboard
- Collisions between balls protected with **mutexes** (ordered locking to avoid deadlocks) and synchronisation with **semaphores**
- Graphics and sound with **SDL2**
- Flowchart of the threads: `organigramme.png`

## Build & run

Requirements: `gcc`, SDL2, SDL2_image, SDL2_mixer.

```bash
make run
```

`libcalc_collision_speeds.a` (collision physics) was provided by the course.

## Tech

C · pthreads · mutexes · semaphores · SDL2
