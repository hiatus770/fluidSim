#include <iostream>
#include <cmath>
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <array>
#include <rlgl.h>
#include <thread>
#include <bits/stdc++.h>

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
const int cellResolution = 250; // This means 10x10 cells

const int size = (cellResolution + 2) * (cellResolution + 2);

// Dens is the density, u is the x velocity, v is the y velocity
#define IX(i, j) (i + (cellResolution + 2) * (j))
#define SWAP(x0, x)      \
    {                    \
        float *tmp = x0; \
        x0 = x;          \
        x = tmp;         \
    }

float u[size], v[size], u_prev[size], v_prev[size];
float dens[size], dens_prev[size];

#include "vectorOperators.h"

// This function is used to add a source to the density, what this means is that it adds a certain amount of density to a certain cell
void add_source(int N, float *x, float *s, float dt)
{
    int i, size = (N + 2) * (N + 2);
    for (i = 0; i < size; i++)
        x[i] += dt * s[i];
}
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
    x[IX(0, 0)] = 0.5f * (x[IX(1, 0)] + x[IX(0, 1)]);
    x[IX(0, N + 1)] = 0.5f * (x[IX(1, N + 1)] + x[IX(0, N)]);
    x[IX(N + 1, 0)] = 0.5f * (x[IX(N, 0)] + x[IX(N + 1, 1)]);
    x[IX(N + 1, N + 1)] = 0.5f * (x[IX(N, N + 1)] + x[IX(N + 1, N)]);
}

void diffuse(int N, int b, float *x, float *x0, float diff, float dt)
{
    int i, j, k;
    float a = dt * diff * N * N;
    for (k = 0; k < 20; k++)
    {
        for (i = 1; i <= N; i++)
        {
            for (j = 1; j <= N; j++)
            {
                x[IX(i, j)] = (x0[IX(i, j)] + a * (x[IX(i + 1, j)] + x[IX(i - 1, j)] + x[IX(i, j + 1)] + x[IX(i, j - 1)])) / (1 + 4 * a);
            }
        }
        set_bnd(N, b, x);
    }
}

void advect(int N, int b, float *d, float *d0, float *u, float *v, float dt)
{
    int i, j, i0, j0, i1, j1;
    float x, y, s0, t0, s1, t1, dt0;
    dt0 = dt * N;
    for (i = 1; i <= N; i++)
    {
        for (j = 1; j <= N; j++)
        {
            x = i - dt0 * u[IX(i, j)];
            y = j - dt0 * v[IX(i, j)];
            if (x < 0.5f)
                x = 0.5f;
            if (x > N + 0.5f)
                x = N + 0.5f;
            i0 = (int)x;
            i1 = i0 + 1;
            if (y < 0.5f)
                y = 0.5f;
            if (y > N + 0.5f)
                y = N + 0.5f;
            j0 = (int)y;
            j1 = j0 + 1;
            s1 = x - i0;
            s0 = 1 - s1;
            t1 = y - j0;
            t0 = 1 - t1;
            d[IX(i, j)] = s0 * (t0 * d0[IX(i0, j0)] + t1 * d0[IX(i0, j1)]) + s1 * (t0 * d0[IX(i1, j0)] + t1 * d0[IX(i1, j1)]);
        }
    }
    set_bnd(N, b, d);
}

void project(int N, float *u, float *v, float *p, float *div)
{
    int i, j, k;
    float h;
    h = 1.0 / N;
    for (i = 1; i <= N; i++)
    {
        for (j = 1; j <= N; j++)
        {
            div[IX(i, j)] = -0.5 * h * (u[IX(i + 1, j)] - u[IX(i - 1, j)] + v[IX(i, j + 1)] - v[IX(i, j - 1)]);
            p[IX(i, j)] = 0;
        }
    }
    set_bnd(N, 0, div);
    set_bnd(N, 0, p);
    for (k = 0; k < 20; k++)
    {
        for (i = 1; i <= N; i++)
        {
            for (j = 1; j <= N; j++)
            {
                p[IX(i, j)] = (div[IX(i, j)] + p[IX(i - 1, j)] + p[IX(i + 1, j)] + p[IX(i, j - 1)] + p[IX(i, j + 1)]) / 4;
            }
        }
        set_bnd(N, 0, p);
    }
    for (i = 1; i <= N; i++)
    {
        for (j = 1; j <= N; j++)
        {
            u[IX(i, j)] -= 0.5 * (p[IX(i + 1, j)] - p[IX(i - 1, j)]) / h;
            v[IX(i, j)] -= 0.5 * (p[IX(i, j + 1)] - p[IX(i, j - 1)]) / h;
        }
    }
    set_bnd(N, 1, u);
    set_bnd(N, 2, v);
}

void vel_step(int N, float *u, float *v, float *u0, float *v0, float visc, float dt)
{
    add_source(N, u, u0, dt);
    add_source(N, v, v0, dt);
    SWAP(u0, u);
    diffuse(N, 1, u, u0, visc, dt);
    SWAP(v0, v);
    diffuse(N, 2, v, v0, visc, dt);
    project(N, u, v, u0, v0);
    SWAP(u0, u);
    SWAP(v0, v);
    advect(N, 1, u, u0, u0, v0, dt);
    advect(N, 2, v, v0, u0, v0, dt);
    project(N, u, v, u0, v0);
}

void dens_step(int N, float *x, float *x0, float *u, float *v, float diff, float dt)
{
    add_source(N, x, x0, dt);
    SWAP(x0, x);
    diffuse(N, 0, x, x0, diff, dt);
    SWAP(x0, x);
    advect(N, 0, x, x0, u, v, dt);
}

/**
 * This function returns the velocity on the grid, so given a point x, y coordinate form it will return the grid index of the respective velocities
 */
int getGridIndex(float x, float y)
{
    int xIndex = x / (screenWidth / cellResolution);
    int yIndex = y / (screenHeight / cellResolution);

    if (IX(xIndex, yIndex) >= size){
        return 0; 
    }

    return IX(xIndex, yIndex);
}

void drawStreamLine(float x, float y, int n)
{
    float coordX = x;
    float coordY = y;

    for (int i = 0; i < n; i++)
    {
        int gridIndex = getGridIndex(coordX, coordY);
        // Draw test line
        float uVel = u[gridIndex];
        float vVel = v[gridIndex];
        float velMagnitude = sqrt(uVel * uVel + vVel * vVel);
        if (velMagnitude == 0 || gridIndex == 0 || gridIndex >= size){
            break;
        }
        
        DrawLine(coordX, coordY, coordX + 9 * u[gridIndex] / velMagnitude, coordY + (v[gridIndex]) / velMagnitude * 9, WHITE);
        coordX += 9 * uVel / velMagnitude;
        coordY += 9 * vVel / velMagnitude;
        
        // Draw a circle
        // DrawCircle(coordX, coordY, 1, BLACK);

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

    InitWindow(screenWidth, screenHeight, "Fluiduh");
    SetTargetFPS(gameFPS);

    Camera2D cam = {0};
    cam.zoom = 1;

    float lastTime = 0;

    // test ball

    float computationTime = 0;
    float drawTime = 0;
    float lastComputationTime = 0;
    float lastDrawTime = 0;

    // Set all velocities to 0
    for (int i = 0; i < size; i++)
    {
        u[i] = 0;
        v[i] = 0;
        u_prev[i] = 0;
        v_prev[i] = 0;
    }

    while (WindowShouldClose() == false)
    {
        minVel = 10000000;
        maxVel = 0;
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

        // Mouse human interaction
        Vector2 mousePos = GetMousePosition();
        Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, cam);
        int x = (int)mouseWorldPos.x;
        int y = (int)mouseWorldPos.y;
        x = x / (screenWidth / cellResolution);
        y = y / (screenHeight / cellResolution);
        if (x >= 0 && x < cellResolution && y >= 0 && y < cellResolution && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            // Make sure its not out of bounds
            if (x >= 1 && x < cellResolution && y >= 1 && y < cellResolution)
            {
                dens[IX(x, y)] = 100;
            }
            // dens_prev[IX(x, y)] = 255;
        }

        // Mkae a velocity field where the mouse is
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
                    // u[IX(x, y)] = mouseDelta.x * 20;
                    // v[IX(x, y)] = mouseDelta.y * 20;
                }
            }
        }

        // Compute the velocity field
        // vel_step(cellResolution, u, v, u_prev, v_prev, 0.001, 0.001);
        dens_step(cellResolution, dens, dens_prev, u, v, 0.01, 0.001);
        // // Dissipate the density field
        // for (int i = 0; i < cellResolution; i++)
        // {
        //     for (int j = 0; j < cellResolution; j++)
        //     {
        //         dens[IX(i, j)] *= 0.999;
        //     }
        // }

        drawTime = GetTime();

        // Start drawing in the camera frame
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(cam);

        // Find minimum density
        float minDensity = 100000000;
        for (int i = 0; i < cellResolution; i++)
        {
            for (int j = 0; j < cellResolution; j++)
            {
                minDensity = min(minDensity, dens[IX(i, j)]);
            }
        }

        // Draw the density field
        for (int i = 0; i < cellResolution; i++)
        {
            for (int j = 0; j < cellResolution; j++)
            {
                // Make sure density is capped at 255
                float density = dens[IX(i, j)];
                if (density > 255)
                {
                    density = 255;
                }
                if (density > 0)
                {
                    // Make it based on the density for transparency and the velocity for color
                    Color color = ColorFromHSV((abs(u[IX(i, j)]) + abs(v[IX(i, j)])) / 2, 255, 100);
                    // Add transparency based on the density
                    color.a = (density) * 10;

                    color.g = 255;
                    color.b = 255;

                    DrawRectangle(i * (screenWidth / cellResolution), j * (screenHeight / cellResolution), screenWidth / cellResolution, screenHeight / cellResolution, color);
                }
            }
        }

        // Draw the streamlines, 100 rays for the entire scren
        for (int i = 0; i < 30; i++)
        {
            float x = i * (screenWidth / 30);
            for (int j = 0; j < 30; j++)
            {
                float y = j * (screenHeight / 30);
                // drawStreamLine(x, y, 15);
            }
        }
    
        // Draw the velocity field
        for (int i = 0; i < cellResolution; i++)
        {
            for (int j = 0; j < cellResolution; j++)
            {
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
                // drawStreamLine(x, y, 1); 
                // DrawLine(x, y, x + 4 * uVel / velMagnitude, y + (vVel) / velMagnitude * 4, ColorFromHSV((sigmoided), 255, 150));
            }
        }


        EndMode2D();
        // End drawing

        drawTime = GetTime() - drawTime;
        lastDrawTime = drawTime;

        // Write image to video memory

        // Display the FPS currently
        DrawFPS(10, 10);

        // Check keys h, j, k, l to chage the sigmoid variables
        if (IsKeyDown(KEY_H))
        {
            shiftSigmoid -= 0.1;
        }
        if (IsKeyDown(KEY_J))
        {
            shiftSigmoid += 0.1;
        }
        if (IsKeyDown(KEY_K))
        {
            multiplierSigmoid -= 0.1;
        }
        if (IsKeyDown(KEY_L))
        {
            multiplierSigmoid += 0.1;
        }
        if (IsKeyDown(KEY_U))
        {
            yMultiplierSigmoid -= 0.1;
        }
        if (IsKeyDown(KEY_I))
        {
            yMultiplierSigmoid += 0.1;
        }

        // Draw sigmoid variables
        DrawText(("Shift: " + to_string(shiftSigmoid)).c_str(), 10, 30, 20, WHITE);
        DrawText(("Multiplier: " + to_string(multiplierSigmoid)).c_str(), 10, 50, 20, WHITE);
        DrawText(("Y Multiplier: " + to_string(yMultiplierSigmoid)).c_str(), 10, 70, 20, WHITE);
        DrawText(("Min Vel: " + to_string(minVel)).c_str(), 10, 90, 20, WHITE);
        DrawText(("Max Vel: " + to_string(maxVel)).c_str(), 10, 110, 20, WHITE);

        // // Display the computation time
        // DrawText(("Computation time: " + to_string(lastComputationTime)).c_str(), 10, 30, 20, WHITE);
        // // Display the draw time
        // DrawText(("Draw time: " + to_string(lastDrawTime)).c_str(), 10, 50, 20, WHITE);

        // Draw ball count
        EndDrawing();
    }
    CloseWindow();
    return 0;
};
