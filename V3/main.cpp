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
#define gameFPS 200
#define getDecimal(x) (x - (int)x)
#define sgn(x) (x < 0 ? -1 : 1)

using namespace std;

const int screenWidth = 1000;
const int screenHeight = 1000;
const int maxWidth = screenWidth;
const int maxHeight = screenHeight;
const int minWidth = 0;
const int minHeight = 0;
const int cellResolution = 250;
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
float u[size]; // X component of velocity
float v[size]; // Y component of velocity

int wall[size];

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
void gaussSiedel(int iter, float *d, float *d0, float a, float dt)
{
    // Density is the current value, we do not change this
    int temp[size];

    addSource(size, density, pastDensity, dt);

    memset(temp, 0, sizeof(temp));
    float k = a * cellResolution * cellResolution * dt;

    // SWAP(d0, d);

    // Density is given to us as our current value for density, the next value can be solved for using a system of simultaneous equations
    for (int iters = 0; iters < iter; iters++)
    {
        for (int i = 1; i < cellResolution + 1; i++)
        {
            for (int j = 1; j < cellResolution + 1; j++)
            {
                // The next value of density is the average of the surrounding values including the k cfonstnat
                // d[IX(i, j)] = (d0[IX(i, j)] + k * (d[IX(i - 1, j)] + d[IX(i + 1, j)] + d[IX(i, j - 1)] + d[IX(i, j + 1)])) / (1 + 4 * k);
                // Add a case for each wall type, and make sure to not use the 0 density, so if there are only 3 valid densities, use those
                if (wall[IX(i, j)] == 1)
                {
                    d[IX(i, j)] = 0;
                }
                else
                {
                    d[IX(i, j)] = (d0[IX(i, j)] + k * (d[IX(i - 1, j)] + d[IX(i + 1, j)] + d[IX(i, j - 1)] + d[IX(i, j + 1)])) / (1 + 4 * k);
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

void drawDensity()
{
    for (int i = 0; i < cellResolution; i++)
    {
        for (int j = 0; j < cellResolution; j++)
        {
            // Make sure density is capped at 255

            Color color = {0};
            float d = density[IX(i, j)];
            if (d > 255)
            {
                d = 255;
            }
            if (d < 10 && d > 0)
            {
                color.r = 255;
                d += 50;
            }
            if (d > 0)
            {
                // Add transparency based on the density
                color.a = d;
                color.g = 255;
                color.b = 255;
                DrawRectangle(i * (screenWidth / cellResolution), j * (screenHeight / cellResolution), screenWidth / cellResolution, screenHeight / cellResolution, color);
            }
        }
    }
}

// Main function
int main()
{
    
    float shiftSigmoid = 248;
    float multiplierSigmoid = 20;
    float yMultiplierSigmoid = 255;
    float minVel = 10000000;
    float maxVel = 0;

    memset(wall, 0, sizeof(wall));
    memset(density, 0, sizeof(density));
    memset(pastDensity, 0, sizeof(pastDensity));
    memset(v,0, sizeof(v)); 
    memset(u,0, sizeof(u)); 

    InitWindow(screenWidth, screenHeight, "Fluiduh");
    SetTargetFPS(gameFPS);

    Camera2D cam = {0};
    cam.zoom = 1;

    // for(int i = 0; i < 10; i++){
    //     for (int j = 50; j < 200; j++){
    //         u[IX(j+i,j+i)] = 10000; 
    //         v[IX(j+i,j+i)] = 10000; 
    //     }
    // }

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

            if (x > 0 && x < cellResolution && y > 0 && y < cellResolution)
            {
                pastDensity[IX(floor(x), floor(y))] = 10000;
            }
        }
        // Velocity 
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
                    Vector2 mouseDelta = GetMouseDelta();
                    u[IX(x, y)] = mouseDelta.x * 200;
                    v[IX(x, y)] = mouseDelta.y * 200;
                }
            }
        }

        // Diffuse
        // K is the delta t betwene the last frame and the current frame
        float k = GetFrameTime();

        // Density Steps 
        gaussSiedel(8, density, pastDensity, 0.01, 0.001);
        swap(density, pastDensity); 
        advect(density, pastDensity, u, v, 0.001); 

        // Velocity Steps 


        

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(cam);

        // Displaying the density
        drawDensity();
        for (int i = 0; i < cellResolution; i++)
        {
            for (int j = 0; j < cellResolution; j++)
            {
                if (u[IX(i, j)] == 0 || v[IX(i, j)] == 0){
                    continue; 
                }
                // Shift by half a cell to get the center of the cell
                float x = i * (screenWidth / cellResolution) + (screenWidth / cellResolution) / 2;
                float y = j * (screenHeight / cellResolution) + (screenHeight / cellResolution) / 2;
                float uVel = u[IX(i, j)];
                float vVel = v[IX(i, j)];
                float velMagnitude = sqrt(uVel * uVel + vVel * vVel);
                minVel = min(minVel, velMagnitude);
                maxVel = max(maxVel, velMagnitude);
                float sigmoided = yMultiplierSigmoid / (1 + exp(-(velMagnitude - shiftSigmoid) / multiplierSigmoid));
                // Scale both down if greater than 10
                if (velMagnitude > 5)
                {
                    uVel /= velMagnitude / 5;
                    vVel /= velMagnitude / 5;
                }
                Color color = ColorFromHSV((sigmoided), 255, 150);
                // Draw the veolcity as a square
                // DrawRectangle(x, y, 10, 10, color);
                // drawStreamLine(x, y, 10); 
                DrawLine(x, y, x + 4 * uVel / velMagnitude, y + (vVel) / velMagnitude * 4, ColorFromHSV((sigmoided), 255, 150));
            }
        }

        // End of camera movement
        EndMode2D();

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
