#version 430 core

// Compute shader for Raycasting planes (floor/ceiling)
// Each thread: 1 Ray renders 1 Pixel
// All rays/pixels in a horizontal strip will share the same depth/distance (pass this in as a constant from CPU, group shader by horizontals?)