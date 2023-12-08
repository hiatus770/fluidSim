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
const int cellResolution = 50;
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

// Make a charge object 
class Charge{
    public: 
    long double x; 
    long double y; 
    int charge; 
    Charge(long double x, long double y, int charge){
        this->x = x; 
        this->y = y; 
        this->charge = charge; 
    }
};

// Main function
int main()
{


    // Make global vector of charges
    vector<Charge> charges;
    // Past sum used for measuring dt of the sum
    float pastSum = 0;

    // Window initliazation
    InitWindow(screenWidth, screenHeight, "fluid");
    SetTargetFPS(gameFPS);

    // Camera initialization
    Camera2D cam = {0};
    cam.zoom = 1;

    float chargeX = 0;
    float chargeY = 0;

    while (WindowShouldClose() == false)
    {

        // Get a list of coordinates for a circle centered in the middle of the screen
        float radius = 100;
        float pointCount = 1000.0;
        vector<Vector2> circleCoordinates;
        for (int i = 0; i < pointCount; i++)
        {
            float angle = toRadian((i/pointCount) * 360);
            float x = cos(angle) * radius + screenWidth / 2;
            float y = sin(angle) * radius + screenHeight / 2;
            circleCoordinates.push_back(Vector2{x, y});
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

            if (x >= 1 && x <= cellResolution && y > 0 && y <= cellResolution)
            {
                Vector2 mouseDelta = GetMouseDelta();
                Vector2 mousePos = GetMousePosition();
                Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, cam);
                int x = (int)mouseWorldPos.x;
                int y = (int)mouseWorldPos.y;

                cout << mouseDelta.x << " " << mouseDelta.y << endl;
                chargeX = x; 
                chargeY = y;
                DrawText(TextFormat("X: %d Y: %d", x, y), 10, 80, 20, BLUE);
            }
        }

        // Drawing the simulation
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(cam);

        // Draw the coordinates calculated of the circle
        for (int i = 0; i < circleCoordinates.size(); i++)
        {
            DrawCircle(circleCoordinates[i].x, circleCoordinates[i].y, 1, WHITE);
        }

        float yComponent = 0;
        float xComponent = 0;
        // Calculate the force from each circle point 
        for (int i = 0; i < circleCoordinates.size(); i++)
        {
            // Get the position of the circle point
            Vector2 circlePoint = circleCoordinates[i];
            // Calculate the distance from the circle point to the charge
            float distance = sqrt(pow(circlePoint.x - chargeX, 2) + pow(circlePoint.y - chargeY, 2));
            // Calculate the force from the charge
            float force = 1 / (4 * 3.14159 * pow(distance, 2));
            // Calculate the angle of the force
            float angle = atan2(circlePoint.y - chargeY, circlePoint.x - chargeX);
            // Calculate the force vector
            Vector2 forceVector = Vector2{force * cos(angle), force * sin(angle)};
            
            yComponent += forceVector.y;
            xComponent += forceVector.x;

            // Draw line between the circle point and the chargeX and chargeY
            DrawLine(circlePoint.x, circlePoint.y, chargeX, chargeY, RED);
        }

        // End of camera movement
        EndMode2D();

        // Display the total y and x components of the force
        DrawText(TextFormat("X: %f Y: %f", xComponent, yComponent), 10, 40, 20, BLUE);

        EndDrawing();
    }
    CloseWindow();
    return 0;
};
