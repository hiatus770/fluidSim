#include <raymath.h>
#include <vector>
#include <array>
#include <rlgl.h>
#include <thread>
#include <bits/stdc++.h>
#include <thread>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
const int cellResolution = 250; // This is the amount of cells in simulation per row 
const float k = 0.1; 
const int size = (cellResolution + 2) * (cellResolution + 2);

// Dens is the density, u is the x velocity, v is the y velocity
#define IX(i, j) (int)(i + (cellResolution + 2) * (j))
#define SWAP(x0, x)      \
    {                    \
        float *tmp = x0; \
        x0 = x;          \

// Shader source code
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform float k;\n"
    "uniform int cellResolution;\n"
    "uniform int size;\n"
    "uniform int d[size];\n"
    "void main()\n"
    "{\n"
    "   int i = int(gl_FragCoord.x);\n"
    "   int j = int(gl_FragCoord.y);\n"
    "   int index = IX(i, j);\n"
    "   if (i > 0 && i <= cellResolution && j > 0 && j <= cellResolution) {\n"
    "       // The next value of density is the average of the surrounding values including the k constant\n"
    "       float nextDensity = (d[index] + k * (d[IX(i - 1, j)] + d[IX(i + 1, j)] + d[IX(i, j - 1)] + d[IX(i, j + 1)]) / 4) / (1 + k);\n"
    "       FragColor = vec4(nextDensity, nextDensity, nextDensity, 1.0);\n"
    "   } else {\n"
    "       FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
    "   }\n"
    "}\n";

// Shader program ID
unsigned int shaderProgram;

// Vertex buffer object and vertex array object
unsigned int VBO, VAO;

// Function to initialize the shader program and vertex buffer objects
void initShaders() {
    // Compile vertex shader
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Check for vertex shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }
    // Compile fragment shader
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Check for fragment shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }
    // Link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    // Set up vertex data
    float vertices[] = {
        // Positions
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f
    };
    // Set up vertex buffer object and vertex array object
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Function to render the density field using the shader program
void renderDensity(int *d, float k, int cellResolution) {
    // Use shader program
    glUseProgram(shaderProgram);
    // Set uniforms
    glUniform1f(glGetUniformLocation(shaderProgram, "k"), k);
    glUniform1i(glGetUniformLocation(shaderProgram, "cellResolution"), cellResolution);
    glUniform1i(glGetUniformLocation(shaderProgram, "size"), size);
    glUniform1iv(glGetUniformLocation(shaderProgram, "d"), size, d);
    // Render quad
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    // Unuse shader program
    glUseProgram(0);
}

// Solver 
void addSource(int N, float *x, float *s, float dt)
{
    for (int i = 0; i < N; i++)
    {
        x[i] += dt * s[i];
    }
}

void gaussSiedel(int iter, int *d, int *d0, float k) {
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

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Density Field", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }
    // Initialize shaders
    initShaders();
    // Initialize density field
    int dens[size];
    int pastDens[size];
    memset(dens, 0, sizeof(dens));
    memset(pastDens, 0, sizeof(pastDens));
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate density field
        gaussSiedel(1, dens, pastDens, k);
        // Render density field
        renderDensity(dens, k, cellResolution);
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Clean up
    glfwTerminate();
    return 0;
}