#include <iostream>
#include <cmath>
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <array>
#include <rlgl.h>
#include <thread>
#include <bits/stdc++.h>
#include <thread>

#define WIDTHGAME 100
#define HEIGHTGAME 100
#define toDegree(x) (x * 180.0 / 3.14159)
#define toRadian(x) (x * 3.14159 / 180.0)
#define gameFPS 60
#define getDecimal(x) (x - (int)x)
#define sgn(x) (x < 0 ? -1 : 1)

using namespace std;

const int screenWidth = 1000;
const int screenHeight = 1000;
const int maxWidth = screenWidth;
const int maxHeight = screenHeight;
const int minWidth = 0;
const int minHeight = 0;
const int cellResolution = 100;
#define float double
const float k = 0.1;
const int size = (cellResolution + 2) * (cellResolution + 2);

// Dens is the density, u is the x velocity, v is the y velocity
#define IX(i, j) (int)(i + (cellResolution + 2) * (j))
#define SWAP(x0, x)      \
    {                    \
        float *tmp = x0; \
        x0 = x;          \
        x = tmp;         \
    }

#include "vectorOperators.h"

// Global Densities
float density[size];
float pastDensity[size];
float u[size];  // X component of velocity
float v[size];  // Y component of velocity
float u0[size]; // X component of past velocity
float v0[size]; // Y component of past velocity

int wall[size]; // 0 is no wall, 1 is wall

// This function sets bounds
void set_bnd(int N, int b, float *x)
{
    int i;
    for (i = 1; i <= N; i++)
    {
        x[IX(0, i)] = b == 1 ? -x[IX(1, i)] : x[IX(1, i)];
        x[IX(N + 1, i)] = b == 1 ? -x[IX(N, i)] : x[IX(N, i)];
        x[IX(i, 0)] = b == 2 ? -x[IX(i, 1)] : x[IX(i, 1)];
        x[IX(i, N + 1)] = b == 2 ? -x[IX(i, N)] : x[IX(i, N)];
    }
    x[IX(0, 0)] = 0.5 * (x[IX(1, 0)] + x[IX(0, 1)]);
    x[IX(0, N + 1)] = 0.5 * (x[IX(1, N + 1)] + x[IX(0, N)]);
    x[IX(N + 1, 0)] = 0.5 * (x[IX(N, 0)] + x[IX(N + 1, 1)]);
    x[IX(N + 1, N + 1)] = 0.5 * (x[IX(N, N + 1)] + x[IX(N + 1, N)]);
}

// This function adds a source to the density
void addSource(int N, float *x, float *s, float dt)
{
    for (int i = 0; i < N; i++)
    {
        x[i] += dt * s[i];
    }
}

// This function is used to diffuse the density and the velocity
void gaussSiedel(int iter, float *d, float *d0, float a, float dt, int b = 0)
{

    float c = a * dt * (cellResolution) * (cellResolution);

    for (int k = 0; k < iter; k++)
    {
        for (int i = 1; i <= cellResolution; i++)
        {
            // Logic for updating average values, if statements are used for wall conditions, otherwise average all 4 cells for the diffusion algorithm
            for (int j = 1; j <= cellResolution; j++)
            {
                // Diffusion
                if (b == 0)
                {
                    if (wall[IX(i, j)] == 1)
                    {
                        d[IX(i, j)] = 0;
                        continue;
                    }

                    // If 3 cells have a wall
                    if (wall[IX(i + 1, j)] && wall[IX(i, j + 1)] && wall[IX(i, j - 1)])
                    {
                        d[IX(i, j)] = (d0[IX(i, j)] + c * (d[IX(i - 1, j)])) / (1 + c);
                    }
                    else if (wall[IX(i - 1, j)] && wall[IX(i, j + 1)] && wall[IX(i, j - 1)])
                    {
                        d[IX(i, j)] = (d0[IX(i, j)] + c * (d[IX(i + 1, j)])) / (1 + c);
                    }
                    else if (wall[IX(i + 1, j)] && wall[IX(i - 1, j)] && wall[IX(i, j - 1)])
                    {
                        d[IX(i, j)] = (d0[IX(i, j)] + c * (d[IX(i, j + 1)])) / (1 + c);
                    }
                    else if (wall[IX(i + 1, j)] && wall[IX(i - 1, j)] && wall[IX(i, j + 1)])
                    {
                        d[IX(i, j)] = (d0[IX(i, j)] + c * (d[IX(i, j - 1)])) / (1 + c);
                    }
                    // Two cell combinations
                    else if (wall[IX(i + 1, j)] && wall[IX(i, j + 1)])
                    {
                        d[IX(i, j)] = (d0[IX(i, j)] + c * (d[IX(i - 1, j)] + d[IX(i, j - 1)])) / (1 + 2 * c);
                    }
                    else if (wall[IX(i - 1, j)] && wall[IX(i, j + 1)])
                    {
                        d[IX(i, j)] = (d0[IX(i, j)] + c * (d[IX(i + 1, j)] + d[IX(i, j - 1)])) / (1 + 2 * c);
                    }
                    else if (wall[IX(i + 1, j)] && wall[IX(i, j - 1)])
                    {
                        d[IX(i, j)] = (d0[IX(i, j)] + c * (d[IX(i - 1, j)] + d[IX(i, j + 1)])) / (1 + 2 * c);
                    }
                    else if (wall[IX(i - 1, j)] && wall[IX(i, j - 1)])
                    {
                        d[IX(i, j)] = (d0[IX(i, j)] + c * (d[IX(i + 1, j)] + d[IX(i, j + 1)])) / (1 + 2 * c);
                    }
                    else if (wall[IX(i + 1, j)] == 1)
                    {
                        d[IX(i, j)] = (d0[IX(i, j)] + c * (d[IX(i - 1, j)] + d[IX(i, j + 1)] + d[IX(i, j - 1)])) / (1 + 3 * c);
                    }
                    else if (wall[IX(i - 1, j)] == 1)
                    {
                        d[IX(i, j)] = (d0[IX(i, j)] + c * (d[IX(i + 1, j)] + d[IX(i, j + 1)] + d[IX(i, j - 1)])) / (1 + 3 * c);
                    }
                    else if (wall[IX(i, j + 1)] == 1)
                    {
                        d[IX(i, j)] = (d0[IX(i, j)] + c * (d[IX(i + 1, j)] + d[IX(i - 1, j)] + d[IX(i, j - 1)])) / (1 + 3 * c);
                    }
                    else if (wall[IX(i, j - 1)] == 1)
                    {
                        d[IX(i, j)] = (d0[IX(i, j)] + c * (d[IX(i + 1, j)] + d[IX(i - 1, j)] + d[IX(i, j + 1)])) / (1 + 3 * c);
                    }
                    else
                    {
                        d[IX(i, j)] = (d0[IX(i, j)] + c * (d[IX(i + 1, j)] + d[IX(i - 1, j)] + d[IX(i, j + 1)] + d[IX(i, j - 1)])) / (1 + 4 * c);
                    }
                } else {
                    // Velocity 
                    d[IX(i, j)] = (d0[IX(i, j)] + c * (d[IX(i + 1, j)] + d[IX(i - 1, j)] + d[IX(i, j + 1)] + d[IX(i, j - 1)])) / (1 + 4 * c);
                    
                }
            }
        }
        set_bnd(cellResolution, 0, d);
    }
}

void advect(float *dens, float *dens0, float *u, float *v, float dt)
{
    int i, j, i0, j0, i1, j1;
    float x, y, s0, t0, s1, t1;
    for (i = 1; i <= cellResolution; i++)
    {
        for (j = 1; j <= cellResolution; j++)
        {
            x = i - dt * u[IX(i, j)];
            y = j - dt * v[IX(i, j)];
            if (x < 0.5)
                x = 0.5;
            if (x > size + 0.5)
                x = size + 0.5;
            if (y < 0.5)
                y = 0.5;
            if (y > size + 0.5)
                y = size + 0.5;
            i0 = (int)x;
            i1 = i0 + 1;
            j0 = (int)y;
            j1 = j0 + 1;
            s1 = x - i0;
            s0 = 1 - s1;
            t1 = y - j0;
            t0 = 1 - t1;
            dens[IX(i, j)] = t0 * (s0 * dens0[IX(i0, j0)] + s1 * dens0[IX(i0, j1)]) + t1 * (s0 * dens0[IX(i1, j0)] + s1 * dens0[IX(i1, j1)]);
        }
    }
}

void drawArray(float *v, int t = 0)
{
    for (int i = 0; i <= cellResolution + 1; i++)
    {
        for (int j = 0; j <= cellResolution + 1; j++)
        {
            // Make sure density is capped at 255

            Color color = {0};
            float d = v[IX(i, j)];

            // DrawText(TextFormat("I: %d J: %d", i, j), i * (screenWidth / cellResolution), j * (screenHeight / cellResolution) + 15, 10, WHITE);
            // DrawText(TextFormat("%f", d), i * (screenWidth / cellResolution), j * (screenHeight / cellResolution), 10, WHITE);

            if (d > 255)
            {
                d = 255;
            }
            if (d < 0.1 && d > 0)
            {
                color.r = 255;
            }
            if (d > 0)
            {
                // Add transparency based on the density
                color.a = d;
                color.g = 255;
                color.b = 255;
                if (t == 1)
                {
                    color.r = d;
                }
                if (t == 2)
                {
                    color.g = d;
                }
                // Stats for each cube
                // I and J values
                // If its a border color it red
                if (i == 0 || i == cellResolution + 1 || j == 0 || j == cellResolution + 1)
                {
                    if (v[IX(i, j)] > 0.0000001)
                    {
                        // DRaw text bside the delta sum
                        DrawText(TextFormat("TOUCHED BORDER"), -10, -80, 20, WHITE);
                    }
                    color.r = 255;
                    color.g = 0;
                    color.b = 0;
                }

                DrawRectangle(i * (screenWidth / cellResolution), j * (screenHeight / cellResolution), screenWidth / cellResolution, screenHeight / cellResolution, color);
            }
        }
    }
}

void densityStep()
{
    // Density Steps
    float dt = 0.004;
    float a = 0.01;

    // Adding the user input from the mouse in this step, all user input is put into past density and then added to the density
    addSource(size, density, pastDensity, dt);
    swap(pastDensity, density); // Now we set pastDensity to be our main density
    gaussSiedel(40, density, pastDensity, a, dt);
}

void velocityStep()
{
    float dt = 0.004;
    float a = 0.01;

    // Velocity Steps for the simulation
    swap(v, v0);
    swap(u, u0);
    gaussSiedel(40, v, v0, a, dt, 1);
    gaussSiedel(40, u, u0, a, dt, 1);
    advect(v0, v, u, v, dt);
    advect(u0, u, u, v, dt);
    set_bnd(cellResolution, 1, u);
    set_bnd(cellResolution, 2, v);
    
   // Project code goes here

}

// Main function
int main()
{
    // Set all boundary tiles to 1 for the wall array
    for (int row = 0; row < cellResolution + 2; row++)
    {
        for (int col = 0; col < cellResolution + 2; col++)
        {
            if (row == 0 || row == cellResolution + 1 || col == 0 || col == cellResolution + 1)
            {
                wall[IX(row, col)] = 1;
            }
            else
            {
                wall[IX(row, col)] = 0;
            }
        }
    }

    // Set a test value for the wall
    wall[IX(10, 10)] = 1;
    wall[IX(10, 11)] = 1;
    wall[IX(11, 10)] = 1;
    wall[IX(11, 11)] = 1;

    // Make a 3 sided square
    wall[IX(20, 20)] = 1;
    wall[IX(20, 21)] = 1;
    wall[IX(20, 22)] = 1;
    wall[IX(21, 20)] = 1;

    wall[IX(21, 22)] = 1;

    // Set all arrays to 0
    memset(density, 1, sizeof(density));
    memset(pastDensity, 0, sizeof(pastDensity));
    memset(v, 0, sizeof(v));
    memset(u, 0, sizeof(u));
    memset(v0, 0, sizeof(v0));
    memset(u0, 0, sizeof(u0));

    // Past sum used for measuring dt of the sum
    float pastSum = 0;

    // Window initliazation
    InitWindow(screenWidth, screenHeight, "fluid");
    SetTargetFPS(gameFPS);

    // Camera initialization
    Camera2D cam = {0};
    cam.zoom = 1;

    while (WindowShouldClose() == false)
    {
        // Camera movement
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f / cam.zoom);
            cam.target = Vector2Add(cam.target, delta);
        }
        float wheel = GetMouseWheelMove();
        if (wheel != 0)
        {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), cam);
            cam.offset = GetMousePosition();
            cam.target = mouseWorldPos;
            cam.zoom += wheel * 0.5f;
            if (cam.zoom < 0.1f)
                cam.zoom = 0.1f;
        }

        // Set density to 0 in order to add new sources from the moue every frame
        memset(pastDensity, 0, sizeof(pastDensity));

        // Adding a source for the densities
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            // Mouse human interaction
            Vector2 mousePos = GetMousePosition();
            Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, cam);
            int x = (int)mouseWorldPos.x;
            int y = (int)mouseWorldPos.y;
            x = x / (screenWidth / cellResolution);
            y = y / (screenHeight / cellResolution);

            if (x >= 1 && x < cellResolution && y > 0 && y < cellResolution)
            {
                pastDensity[IX(floor(x), floor(y))] = 100000;
            }
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            Vector2 mousePos = GetMousePosition();
            Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, cam);
            int x = (int)mouseWorldPos.x;
            int y = (int)mouseWorldPos.y;
            x = x / (screenWidth / cellResolution);
            y = y / (screenHeight / cellResolution);
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Vector2 mousePos = GetMousePosition();
            Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, cam);
            int x = (int)mouseWorldPos.x;
            int y = (int)mouseWorldPos.y;
            x = x / (screenWidth / cellResolution);
            y = y / (screenHeight / cellResolution);
            if (x >= 0 && x < cellResolution && y >= 0 && y < cellResolution)
            {
                if (x >= 1 && x < cellResolution && y >= 1 && y < cellResolution)
                {
                    // Make velocity changedirection based on the mouse movement
                    // GetMouseDelta
                    // Vector2 mouseDelta = GetMouseDelta();
                    // u[IX(x, y)] = mouseDelta.x * 200;
                    // v[IX(x, y)] = mouseDelta.y * 200;
                }
            }
        }

        // Reset all of the densities to be 0
        if (IsKeyDown(KEY_SPACE))
        {
            memset(density, 0, sizeof(density));
            memset(pastDensity, 0, sizeof(pastDensity));
        }

        // Simulation steps
        densityStep();
        velocityStep();

        // Drawing the simulation
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(cam);

        // Displaying the density
        drawArray(density);

        // End of camera movement
        EndMode2D();

        // Sum up the entire array
        float sum = 0;

        for (int x = 1; x <= cellResolution; x++)
        {
            for (int y = 1; y <= cellResolution; y++)
            {
                sum += density[IX(x, y)];
            }
        }

        // Display
        DrawText(TextFormat("Sum: %f", sum), 10, 40, 20, WHITE);
        DrawText(TextFormat("Delta Sum: %f", (sum - pastSum) / GetFrameTime()), 10, 20, 20, WHITE);

        // Average Sum
        DrawText(TextFormat("Average Sum: %f", sum / size), 10, 60, 20, WHITE);
        pastSum = sum;

        // Display the FPS currently
        DrawFPS(10, 10);
        EndDrawing();

        for (int i = 0; i < size; i++)
        {
            pastDensity[i] = density[i];
        }
    }
    CloseWindow();
    return 0;
};
