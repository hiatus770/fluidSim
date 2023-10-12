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
const int cellResolution = 500; // This is the amount of cells in simulation per row 
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
int density[size];
int pastDensity[size];

// Solver 
void addSource(int N, float *x, float *s, float dt)
{
    for (int i = 0; i < N; i++)
    {
        x[i] += dt * s[i];
    }
}

void gaussSiedel2(int iter, int *d, int *d0, float k) {
    // Density is the current value, we do not change this 
    int temp[size];
    memset(temp, 0, sizeof(temp));

    // Density is given to us as our current value for density, the next value can be solved for using a system of simultaneous equations
    for (int k = 0; k < iter; k++) {
        std::vector<std::thread> threads;
        int numThreads = std::thread::hardware_concurrency();
        int chunkSize = (cellResolution * cellResolution) / numThreads;
        int remainder = (cellResolution * cellResolution) % numThreads;
        int start = 1;
        for (int i = 0; i < numThreads; i++) {
            int end = start + chunkSize - 1;
            if (i < remainder) {
                end++;
            }
            threads.emplace_back([=]() {
                for (int index = start; index <= end; index++) {
                    int i = (index - 1) / cellResolution + 1;
                    int j = (index - 1) % cellResolution + 1;
                    // The next value of density is the average of the surrounding values including the k constant
                    d[IX(i, j)] = (d0[IX(i, j)] + k * (temp[IX(i - 1, j)] + temp[IX(i + 1, j)] + temp[IX(i, j - 1)] + temp[IX(i, j + 1)]) / 4) / (1 + k);
                }
            });
            start = end + 1;
        }
        for (auto& thread : threads) {
            thread.join();
        }
        for (int i = 0; i < size; i++) {
            temp[i] = d[i];
        }
    }
}

void gaussSiedel (int iter, int *d, int *d0, float k){
    // Density is the current value, we do not change this 
    int temp[size];
    memset(temp, 0, sizeof(temp));

    // Density is given to us as our current value for density, the next value can be solved for using a system of simultaneous equations
    for (int k = 0; k < iter; k++){
        for (int i = 1; i < cellResolution + 1; i++){
            for (int j = 1; j < cellResolution + 1; j++){
                // The next value of density is the average of the surrounding values including the k cfonstnat
                d[IX(i,j)] = (d0[IX(i,j)] + k * (temp[IX(i-1,j)] + temp[IX(i+1,j)] + temp[IX(i,j-1)] + temp[IX(i,j+1)])) / (1 + 4*k);

            }
        }
        for (int i = 0; i < size; i++){
            temp[i] = d[i]; 
        }
    }

    
}

// Main function
int main()
{

    InitWindow(screenWidth, screenHeight, "Fluiduh");
    SetTargetFPS(gameFPS);

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

            if (x > 0 && x < cellResolution && y > 0 && y < cellResolution){
                cout << "Added" << endl; 
                pastDensity[IX(floor(x),floor(y))] = 1000;
            }
        }

        // Diffuse
        // K is the delta t betwene the last frame and the current frame
        float k = GetFrameTime();
        gaussSiedel(3, density, pastDensity, k); 

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(cam);
        // Draw a rectangle repesented the grid 
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
                if (d < 10 && d > 0){
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


        // End of camera movement
        EndMode2D();

        // Display the FPS currently
        DrawFPS(10, 10);
        EndDrawing();

        for (int i = 0; i < size; i++){
            pastDensity[i] = density[i]; 
        }
    }
    CloseWindow();
    return 0;
};
