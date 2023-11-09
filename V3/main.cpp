// #include <iostream>
#include <cmath>
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <array>
#include <rlgl.h>
#include <thread>
#include <bits/stdc++.h>
#include <thread>

#define WIDTHGAME 400
#define HEIGHTGAME 400
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
#define float long double
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
float u[size];  // X component of velocity
float v[size];  // Y component of velocity
float u0[size]; // X component of past velocity
float v0[size]; // Y component of past velocity

int wall[size]; // 0 is no wall, 1 is wall


void drawVelocities()
{
    float shiftSigmoid = 248;
    float multiplierSigmoid = 20;
    float yMultiplierSigmoid = 255;
    for (int i = 0; i < cellResolution; i++)
    {
        for (int j = 0; j < cellResolution; j++)
        {
            // Check if velocity is 0
            if (u[IX(i, j)] == 0 && v[IX(i, j)] == 0)
            {
                continue;
            }
            // Shift by half a cell to get the center of the cell
            float x = i * (screenWidth / cellResolution) + (screenWidth / cellResolution) / 2;
            float y = j * (screenHeight / cellResolution) + (screenHeight / cellResolution) / 2;
            float uVel = u[IX(i, j)];
            float vVel = v[IX(i, j)];
            float velMagnitude = sqrt(uVel * uVel + vVel * vVel);
            float sigmoided = yMultiplierSigmoid / (1 + exp(-(velMagnitude - shiftSigmoid) / multiplierSigmoid));
            // Scale both down if greater than 10
            Color color = ColorFromHSV((sigmoided), 255, 150);
            // Draw the veolcity as a square
            // DrawRectangle(x, y, 10, 10, color);
            DrawLine(x, y, x + 8 * uVel / velMagnitude, y + (vVel) / velMagnitude * 8, ColorFromHSV((sigmoided), 255, 150));
            // Draw arrow head with two lines
            DrawLine(x + 8 * uVel / velMagnitude, y + (vVel) / velMagnitude * 8, x + 8 * uVel / velMagnitude - 5 * cos(atan2(vVel, uVel) + 3.14159 / 4), y + (vVel) / velMagnitude * 8 - 5 * sin(atan2(vVel, uVel) + 3.14159 / 4), ColorFromHSV((sigmoided), 255, 150));
            DrawLine(x + 8 * uVel / velMagnitude, y + (vVel) / velMagnitude * 8, x + 8 * uVel / velMagnitude - 5 * cos(atan2(vVel, uVel) - 3.14159 / 4), y + (vVel) / velMagnitude * 8 - 5 * sin(atan2(vVel, uVel) - 3.14159 / 4), ColorFromHSV((sigmoided), 255, 150));
        }
    }
}

// Main function
int main()
{

    // Point charge coordinates 
    int Px = 10; 
    int Py = 10; 
    int Nx = 60;
    int Ny = 60;

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
        memset(v, 0, sizeof(v));
        memset(u, 0, sizeof(u));

        // Draw charges
        // DrawCircle(Px *, 10, ColorFromHSV(255, 255, 255));
        // DrawCircle(Nx, Ny, 10, ColorFromHSV(255, 255, 255));

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

            if (x >= 1 && x <= cellResolution && y > 0 && y <= cellResolution)
            {
                Vector2 mouseDelta = GetMouseDelta();
                Vector2 mousePos = GetMousePosition();
                Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, cam);
                int x = (int)mouseWorldPos.x;
                int y = (int)mouseWorldPos.y;
                x = x / (screenWidth / cellResolution);
                y = y / (screenHeight / cellResolution);
                cout << mouseDelta.x << " " << mouseDelta.y << endl;
                u[IX(x, y)] = mouseDelta.x * 15;
                v[IX(x, y)] = mouseDelta.y * 15;
                Px = x;
                Py = y;
                // v[IX(x, y)] = 200;
                // Print out the mouse position
                DrawText(TextFormat("X: %d Y: %d", x, y), 10, 80, 20, BLUE);
            }
        }

        // Calculate the force cause by the point charges and their directions
        for (int i = 1; i <= cellResolution; i++)
        {
            for (int j = 1; j <= cellResolution; j++)
            {
                // Calculate the force caused by the point charges
                float forceX = 0;
                float forceY = 0;
                float distanceP = sqrt(pow(i - Px, 2) + pow(j - Py, 2));
                float distanceN = sqrt(pow(i - Nx, 2) + pow(j - Ny, 2));
                if (distanceP == 0 || distanceN == 0){
                    u[IX(i, j)] = 0;
                    v[IX(i, j)] = 0;
                    continue; 
                }
                int k = 20; 
                forceX +=  5 * k * (i - Px) / pow(distanceP, 2);
                forceY +=  5 * k * (j - Py) / pow(distanceP, 2);
                forceX += -k * (i - Nx) / pow(distanceN, 2);
                forceY += -k * (j - Ny) / pow(distanceN, 2);
                u[IX(i, j)] += forceX;
                v[IX(i, j)] += forceY;
            }
        }



        // Drawing the simulation
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(cam);

        // Draw the velocities

        drawVelocities();
        // Draw field lines, start by drawing 8 vectors around the positive point 
        for(int i = 0; i < 10; i++){
            float ang = 2 * 3.14159 * i / 10;
            // Start at the positive point
            float x = Px;
            float y = Py;
            float pastX = x;
            float pastY = y;
            float forceX = cos (ang) * 400;
            float forceY = sin (ang) * 400;
            for (int i = 0; i < 1000; i++){
                // Treat the force as velocity

                pastY = y;
                pastX = x;
                x += forceX * 0.001;
                y += forceY * 0.001;
                forceX = 0;
                forceY = 0;
                // Calculate the new force
                float distanceP = sqrt(pow(x - Px, 2) + pow(y - Py, 2));
                float distanceN = sqrt(pow(x - Nx, 2) + pow(y - Ny, 2));
                if (distanceP < 0.1 || distanceN < 0.1){
                    break; 
                }
                int k = 20;
                forceX +=  k * (x - Px) / pow(distanceP, 2);
                forceY +=  k * (y - Py) / pow(distanceP, 2);
                forceX += -k * (x - Nx) / pow(distanceN, 2);
                forceY += -k * (y - Ny) / pow(distanceN, 2);
                // Draw the line from the old point to the new point
                // DrawLine((x/100) * screenWidth, (y/100) * screenHeight, (pastX/100) * screenWidth, (pastY/100) * screenHeight, WHITE);

            }
        }


        // for (int i = 1; i <= cellResolution; i++)
        // {
        //     for (int j = 1; j <= cellResolution; j++)
        //     {
        //         // Draw the veolcity as a square
        //         float x = i * (screenWidth / cellResolution) + (screenWidth / cellResolution) / 2;
        //         float y = j * (screenHeight / cellResolution) + (screenHeight / cellResolution) / 2;
        //         float uVel = u[IX(i, j)];
        //         float vVel = v[IX(i, j)];
        //         float velMagnitude = sqrt(uVel * uVel + vVel * vVel);
        //         if (abs(0.4 * uVel) < 0.01 && abs(0.4 * vVel) < 0.01)
        //         {
        //             // Draw vector with magnitude of one
        //             DrawLine(x, y, x + 1 * uVel / uVel, y + 0.4 * (vVel / vVel), ColorFromHSV(255, 255, 150));
        //         }
        //         else
        //         {
        //             // DrawLine(x, y, x + 1.5 * uVel, y + 1.5 * (vVel), ColorFromHSV(255, 255, 150));
        //             DrawLine(i * (screenWidth / cellResolution) + (screenWidth / cellResolution) / 2, j * (screenHeight / cellResolution) + (screenHeight / cellResolution) / 2, i * (screenWidth / cellResolution) + (screenWidth / cellResolution) / 2 + 4 * u[IX(i, j)], j * (screenHeight / cellResolution) + (screenHeight / cellResolution) / 2 + (v[IX(i, j)]) * 4, ColorFromHSV(255, 255, 150));
        //         }
        //     }
        // }

        // End of camera movement
        EndMode2D();


        // Display
        // DrawText(TextFormat("Sum: %f", sum), 10, 40, 20, WHITE);
        // DrawText(TextFormat("Delta Sum: %f", (sum - pastSum) / GetFrameTime()), 10, 20, 20, WHITE);

        // // Average Sum
        // DrawText(TextFormat("Average: %f", sum / size), 10, 60, 20, WHITE);
        // pastSum = sum;

        // // Display the FPS currently
        // DrawFPS(10,100);
        EndDrawing();
    }
    CloseWindow();
    return 0;
};
