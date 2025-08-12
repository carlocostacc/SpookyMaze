//
// COMP 371 Labs Framework
//
// Created by Nicolas Bergeron on 20/06/2019.
//

// Standard library includes
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <algorithm>
#include <vector>

// ...existing code...
#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // GLFW provides a cross-platform interface for creating a graphical context,
                        // initializing OpenGL and binding inputs

// Shader loader helper (for external shader loading)
#include "shaderloader.h"

#include <glm/glm.hpp>  
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


#include "Headers/Candle.h"
#include "Headers/Pillar.h"
#include "Headers/Projectile.h"
#include "Headers/StandardSkeletonEnemy.h"

#include "Headers/ParticleSystem.h"



using namespace glm;
using namespace std;


// Uniform setter helpers (Tut06.cpp style)
void SetUniformMat4(GLuint shader_id, const char* uniform_name, mat4 uniform_value) {
    glUseProgram(shader_id);
    glUniformMatrix4fv(glGetUniformLocation(shader_id, uniform_name), 1, GL_FALSE, &uniform_value[0][0]);
}

void SetUniformVec3(GLuint shader_id, const char* uniform_name, vec3 uniform_value) {
    glUseProgram(shader_id);
    glUniform3fv(glGetUniformLocation(shader_id, uniform_name), 1, value_ptr(uniform_value));
}

template <class T>
void SetUniform1Value(GLuint shader_id, const char* uniform_name, T uniform_value) {
    glUseProgram(shader_id);
    glUniform1i(glGetUniformLocation(shader_id, uniform_name), uniform_value);
    glUseProgram(0);
}

#include "OBJloader.h"  //For loading .obj files
#include "OBJloaderV2.h"  

// OBJLoader
inline GLuint setupModelVBO(const std::string& path, int& vertexCount) {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> UVs;


    // After loading OBJ data in setupModelVBO, ensure normals are present and smooth
    loadOBJ(path.c_str(), vertices, normals, UVs);
    // If normals are missing or incorrect, generate smooth normals
    if (normals.size() != vertices.size()) {
        normals.resize(vertices.size(), glm::vec3(0.0f, 0.0f, 1.0f)); // Default normal
        // Optionally, implement a function to compute smooth normals from faces
    }

    std::string debugFileName;
    if (path.find("staff.obj") != std::string::npos) {
        debugFileName = "staff_debug.txt";
    } else if (path.find("lantern2.obj") != std::string::npos) {
        debugFileName = "lantern_debug.txt";
    } else if (path.find("ball.obj") != std::string::npos) {
        debugFileName = "ball_debug.txt";
    } else {
        debugFileName = "model_debug.txt";
    }
    std::ofstream debugFile(debugFileName);
    debugFile << "DEBUG: OBJ loaded from " << path << ", vertex count = " << vertices.size() << std::endl;
    if (!vertices.empty()) {
        debugFile << "DEBUG: First 5 vertex positions:" << std::endl;
        for (size_t i = 0; i < std::min(vertices.size(), size_t(5)); ++i) {
            debugFile << "  [" << vertices[i].x << ", " << vertices[i].y << ", " << vertices[i].z << "]" << std::endl;
        }
        glm::vec3 minV = vertices[0];
        glm::vec3 maxV = vertices[0];
        for (const auto& v : vertices) {
            minV = glm::min(minV, v);
            maxV = glm::max(maxV, v);
        }
        glm::vec3 size = maxV - minV;
        float maxDim = glm::max(size.x, glm::max(size.y, size.z));
        debugFile << "DEBUG: minV = [" << minV.x << ", " << minV.y << ", " << minV.z << "]" << std::endl;
        debugFile << "DEBUG: maxV = [" << maxV.x << ", " << maxV.y << ", " << maxV.z << "]" << std::endl;
        debugFile << "DEBUG: maxDim = " << maxDim << std::endl;
        if (maxDim > 0.0f) {
            for (auto& v : vertices) {
                v = (v - (minV + maxV) * 0.5f) / maxDim; // Center and scale
            }
        }
    }

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    debugFile << "DEBUG: VAO = " << VAO << std::endl;
    debugFile.close();

    // Vertex positions
    GLuint vertices_VBO;
    glGenBuffers(1, &vertices_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Normals 
    if (!normals.empty()) {
        GLuint normals_VBO;
        glGenBuffers(1, &normals_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals.front(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
        glEnableVertexAttribArray(1);
    }

    // UVs 
    if (!UVs.empty()) {
        GLuint uvs_VBO;
        glGenBuffers(1, &uvs_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, uvs_VBO);
        glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs.front(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
        glEnableVertexAttribArray(2);
    }

    glBindVertexArray(0);
    vertexCount = vertices.size();
    std::cout << "DEBUG: VAO = " << VAO << std::endl;
    return VAO;
}

int maze[20][20] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,1},
    {1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1},
    {1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,0,0,0,0,0,0,0,1,0,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,2,0,0,0,0,1,0,1,0,1,0,1},
    {1,0,1,0,1,0,0,0,0,0,1,0,0,1,0,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,1},
    {1,0,1,1,1,0,0,2,0,0,2,0,0,1,0,1,0,1,0,1},
    {1,0,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1},
    {1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1}
};
void setWorldMatrix(int shaderProgram, glm::mat4 worldMatrix);


GLuint loadTexture(const char *filename);

const char* getVertexShaderSource();

const char* getFragmentShaderSource();

const char* getTexturedVertexShaderSource();

const char* getTexturedFragmentShaderSource();

int compileAndLinkShaders(const char* vertexShaderSource, const char* fragmentShaderSource);

struct TexturedColoredVertex
{
    TexturedColoredVertex(vec3 _position, vec3 _color, vec2 _uv, vec3 _normal)
        : position(_position), color(_color), uv(_uv), normal(_normal) {}

    vec3 position;
    vec3 color;
    vec2 uv;
    vec3 normal;
};


// Textured Cube model
const TexturedColoredVertex texturedCubeVertexArray[] = {
     // Left face (-X)
    TexturedColoredVertex(vec3(-0.5f,-0.5f,-0.5f), vec3(1,0,0), vec2(0,0), vec3(-1,0,0)),
    TexturedColoredVertex(vec3(-0.5f,-0.5f, 0.5f), vec3(1,0,0), vec2(0,1), vec3(-1,0,0)),
    TexturedColoredVertex(vec3(-0.5f, 0.5f, 0.5f), vec3(1,0,0), vec2(1,1), vec3(-1,0,0)),
    TexturedColoredVertex(vec3(-0.5f,-0.5f,-0.5f), vec3(1,0,0), vec2(0,0), vec3(-1,0,0)),
    TexturedColoredVertex(vec3(-0.5f, 0.5f, 0.5f), vec3(1,0,0), vec2(1,1), vec3(-1,0,0)),
    TexturedColoredVertex(vec3(-0.5f, 0.5f,-0.5f), vec3(1,0,0), vec2(1,0), vec3(-1,0,0)),

    // Right face (+X)
    TexturedColoredVertex(vec3(0.5f,-0.5f, 0.5f), vec3(1,0,1), vec2(0,1), vec3(1,0,0)),
    TexturedColoredVertex(vec3(0.5f,-0.5f,-0.5f), vec3(1,0,1), vec2(0,0), vec3(1,0,0)),
    TexturedColoredVertex(vec3(0.5f, 0.5f,-0.5f), vec3(1,0,1), vec2(1,0), vec3(1,0,0)),
    TexturedColoredVertex(vec3(0.5f,-0.5f, 0.5f), vec3(1,0,1), vec2(0,1), vec3(1,0,0)),
    TexturedColoredVertex(vec3(0.5f, 0.5f,-0.5f), vec3(1,0,1), vec2(1,0), vec3(1,0,0)),
    TexturedColoredVertex(vec3(0.5f, 0.5f, 0.5f), vec3(1,0,1), vec2(1,1), vec3(1,0,0)),

    // Bottom face (-Y)
    TexturedColoredVertex(vec3(-0.5f,-0.5f,-0.5f), vec3(0,1,1), vec2(0,0), vec3(0,-1,0)),
    TexturedColoredVertex(vec3(0.5f,-0.5f,-0.5f), vec3(0,1,1), vec2(1,0), vec3(0,-1,0)),
    TexturedColoredVertex(vec3(0.5f,-0.5f, 0.5f), vec3(0,1,1), vec2(1,1), vec3(0,-1,0)),
    TexturedColoredVertex(vec3(-0.5f,-0.5f,-0.5f), vec3(0,1,1), vec2(0,0), vec3(0,-1,0)),
    TexturedColoredVertex(vec3(0.5f,-0.5f, 0.5f), vec3(0,1,1), vec2(1,1), vec3(0,-1,0)),
    TexturedColoredVertex(vec3(-0.5f,-0.5f, 0.5f), vec3(0,1,1), vec2(0,1), vec3(0,-1,0)),

    // Top face (+Y)
    TexturedColoredVertex(vec3(-0.5f,0.5f, 0.5f), vec3(1,1,0), vec2(0,1), vec3(0,1,0)),
    TexturedColoredVertex(vec3(0.5f,0.5f, 0.5f), vec3(1,1,0), vec2(1,1), vec3(0,1,0)),
    TexturedColoredVertex(vec3(0.5f,0.5f,-0.5f), vec3(1,1,0), vec2(1,0), vec3(0,1,0)),
    TexturedColoredVertex(vec3(-0.5f,0.5f, 0.5f), vec3(1,1,0), vec2(0,1), vec3(0,1,0)),
    TexturedColoredVertex(vec3(0.5f,0.5f,-0.5f), vec3(1,1,0), vec2(1,0), vec3(0,1,0)),
    TexturedColoredVertex(vec3(-0.5f,0.5f,-0.5f), vec3(1,1,0), vec2(0,0), vec3(0,1,0)),

    // Front face (+Z)
    TexturedColoredVertex(vec3(-0.5f,-0.5f,0.5f), vec3(0,1,0), vec2(0,0), vec3(0,0,1)),
    TexturedColoredVertex(vec3(0.5f,-0.5f,0.5f), vec3(0,1,0), vec2(1,0), vec3(0,0,1)),
    TexturedColoredVertex(vec3(0.5f,0.5f,0.5f), vec3(0,1,0), vec2(1,1), vec3(0,0,1)),
    TexturedColoredVertex(vec3(-0.5f,-0.5f,0.5f), vec3(0,1,0), vec2(0,0), vec3(0,0,1)),
    TexturedColoredVertex(vec3(0.5f,0.5f,0.5f), vec3(0,1,0), vec2(1,1), vec3(0,0,1)),
    TexturedColoredVertex(vec3(-0.5f,0.5f,0.5f), vec3(0,1,0), vec2(0,1), vec3(0,0,1)),

    // Back face (-Z)
    TexturedColoredVertex(vec3(0.5f,-0.5f,-0.5f), vec3(0,0,1), vec2(0,0), vec3(0,0,-1)),
    TexturedColoredVertex(vec3(-0.5f,-0.5f,-0.5f), vec3(0,0,1), vec2(1,0), vec3(0,0,-1)),
    TexturedColoredVertex(vec3(-0.5f,0.5f,-0.5f), vec3(0,0,1), vec2(1,1), vec3(0,0,-1)),
    TexturedColoredVertex(vec3(0.5f,-0.5f,-0.5f), vec3(0,0,1), vec2(0,0), vec3(0,0,-1)),
    TexturedColoredVertex(vec3(-0.5f,0.5f,-0.5f), vec3(0,0,1), vec2(1,1), vec3(0,0,-1)),
    TexturedColoredVertex(vec3(0.5f,0.5f,-0.5f), vec3(0,0,1), vec2(0,1), vec3(0,0,-1)),
};

int createTexturedCubeVertexArrayObject();

void setProjectionMatrix(int shaderProgram, mat4 projectionMatrix)
{
    glUseProgram(shaderProgram);
    GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
}

void setViewMatrix(int shaderProgram, mat4 viewMatrix)
{
    glUseProgram(shaderProgram);
    GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
}

void setWorldMatrix(int shaderProgram, mat4 worldMatrix)
{
    glUseProgram(shaderProgram);
    GLuint worldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
}

bool isCollidingWithPillars(const glm::vec3& pos, float buffer = 1.0f) {
    // Pillar grid: 20x20, each pillar at (-100 + i*10, y, -100 + j*10), size 2x20x2
    float pillarHalfSize = 1.0f; // half-width of pillar (pillar is 2 units wide)
    for (int i = 0; i < 20; ++i) {
        for (int j = 0; j < 20; ++j) {
            float px = -100.0f + i * 10.0f;
            float pz = -100.0f + j * 10.0f;
            if (abs(pos.x - px) < pillarHalfSize + buffer && abs(pos.z - pz) < pillarHalfSize + buffer) {
                return true;
            }
        }
    }
    return false;
}


int main(int argc, char*argv[])
{
    // Initialize GLFW and OpenGL version
    glfwInit();
    
#if defined(PLATFORM_OSX)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    // On windows, we set OpenGL version to 2.1, to support more hardware
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif

    // Create Window and rendering context using GLFW, resolution is 1600x900
    GLFWwindow* window = glfwCreateWindow(1600, 900, "Spooky maze", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwMakeContextCurrent(window);

    // Lock and hide the cursor inside the window
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Particle system instance (must be after OpenGL context and GLEW init)
    ParticleSystem impactParticleSystem;

    
    // Load Textures
    GLuint brickTextureID = loadTexture("Textures/brick.jpg");
    GLuint cementTextureID = loadTexture("Textures/cement.jpg");
    GLuint pavingStoneTextureID = loadTexture("Textures/PavingStones068_1K-PNG_Color.png");
    GLuint pavingStoneNormalID = loadTexture("Textures/PavingStones068_1K-PNG_NormalGL.png");
    GLuint brickWallTextureID = loadTexture("Textures/brick_wall_13_diff_4k.jpg");
    GLuint brickWallNormalID = loadTexture("Textures/brick_wall_13_nor_gl_4k.exr"); // If your loader supports EXR
    GLuint tiles089TextureID = loadTexture("Textures/Tiles089_1K-JPG_Color.jpg");
    GLuint tiles089NormalID = loadTexture("Textures/Tiles089_1K-JPG_NormalGL.jpg");
    GLuint lanternTextureID = loadTexture("Textures/Metal047B_1K-JPG_Color.jpg");
    GLuint lavaBallTextureID = loadTexture("Textures/Lava004_1K-JPG_Color.jpg"); // Load lava texture once
    GLuint staffWoodTextureID = loadTexture("Textures/rough_wood_diff_4k.jpg");
    // Black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Compile and link shaders here ...
    int colorShaderProgram = compileAndLinkShaders(getVertexShaderSource(), getFragmentShaderSource());
    int texturedShaderProgram = compileAndLinkShaders(getTexturedVertexShaderSource(), getTexturedFragmentShaderSource());
        // Compile and link skeleton shaders
        std::ifstream skeletonVertFile("Shaders/skeleton_vertex.glsl");
        std::ifstream skeletonFragFile("Shaders/skeleton_fragment.glsl");
        std::string skeletonVertSrc((std::istreambuf_iterator<char>(skeletonVertFile)), std::istreambuf_iterator<char>());
        std::string skeletonFragSrc((std::istreambuf_iterator<char>(skeletonFragFile)), std::istreambuf_iterator<char>());
        int skeletonShaderProgram = compileAndLinkShaders(skeletonVertSrc.c_str(), skeletonFragSrc.c_str());

    // Camera parameters for view transform
    // Spawn camera at the bottom entrance of the maze
    // bottom entrance: maze[15][10] == 0 (open cell)
    float spawnX = -50.0f + 15 * 5.0f; // pillarSpacing = 5.0f
    float spawnZ = -50.0f + 10 * 5.0f;
    vec3 cameraPosition(spawnX, 1.0f, spawnZ);
    vec3 cameraLookAt(0.0f, 0.0f, -1.0f);
    vec3 cameraUp(0.0f, 1.0f, 0.0f);
    
    // Other camera parameters
    float cameraSpeed = 1.0f;
    float cameraFastSpeed = 5 * cameraSpeed;
    float cameraHorizontalAngle = 90.0f;
    float cameraVerticalAngle = 0.0f;
    bool  cameraFirstPerson = true; // press 1 or 2 to toggle this variable
    float walkTimer = 0.0f;
    float walkSpeed = 8.0f;      // Controls speed of bobbing
    float walkAmplitude = 0.05f; // Controls height of bobbing
    // Spinning cube at camera position
    float spinningCubeAngle = 0.0f;
    
    // Set projection matrix for shader, this won't change
    mat4 projectionMatrix = glm::perspective(70.0f,            // field of view in degrees
                                             1600.0f / 900.0f,  // aspect ratio
                                             0.01f, 100.0f);   // near and far (near > 0)
    
    // Set initial view matrix
    mat4 viewMatrix = lookAt(cameraPosition,  // eye
                             cameraPosition + cameraLookAt,  // center
                             cameraUp ); // up
    
    // Set View and Projection matrices on both shaders 
    setViewMatrix(colorShaderProgram, viewMatrix);
    setViewMatrix(texturedShaderProgram, viewMatrix);
    setProjectionMatrix(colorShaderProgram, projectionMatrix);
    setProjectionMatrix(texturedShaderProgram, projectionMatrix);

    // Define and upload geometry to the GPU here 
    int texturedCubeVAO = createTexturedCubeVertexArrayObject();

    // Load staff.obj for player-held staff
    int staffVertexCount = 0;
    GLuint staffVAO = setupModelVBO("Models/staff.obj", staffVertexCount);

    // Load skeleton2.obj for enemy mesh
    int skeletonVertexCount = 0;
    GLuint skeletonVAO = setupModelVBO("Models/skeleton2.obj", skeletonVertexCount);

    // For frame time
    float lastFrameTime = glfwGetTime();
    int lastMouseLeftState = GLFW_RELEASE;
    double lastMousePosX, lastMousePosY;
    glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);
    
    // Other OpenGL states to set once
    // Enable Backface culling
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
    // Container for projectiles to be implemented in tutorial
    list<Projectile> projectileList;

    glBindVertexArray(texturedCubeVAO);
    // Load ball.obj for projectile mesh
    int ballVertexCount = 0;
    GLuint ballVAO = setupModelVBO("Models/ball.obj", ballVertexCount);
    // Pillars
    std::vector<Pillar> pillars;
    float pillarSpacing = 5.0f; // spacing between pillars
    float pillarWidth = 5.0f;   // width of each pillar (X and Z)
    for (int i = 0; i < 20; ++i) {
        for (int j = 0; j < 20; ++j) {
            if (maze[i][j] == 1) {
                glm::vec3 pos(-50.0f + i * pillarSpacing, 10.0f, -50.0f + j * pillarSpacing);
                glm::vec3 dims(pillarWidth, 20.0f, pillarWidth);
                pillars.emplace_back(pos, dims, texturedShaderProgram);
            }
        }
    }

    // Spawn skeleton enemies at maze cells with value 2
    std::vector<StandardSkeletonEnemy> skeletonEnemies;
    for (int i = 0; i < 20; ++i) {
        for (int j = 0; j < 20; ++j) {
            if (maze[i][j] == 2) {
                glm::vec3 spawnPos(-50.0f + i * pillarSpacing, 0.83f, -50.0f + j * pillarSpacing);
                skeletonEnemies.emplace_back(spawnPos);
            }
        }
    }

    // Candles: spawn one candle every 6 pillars
    std::vector<Candle> candles;
    int lanternVertexCount = 0;
    GLuint lanternVAO = setupModelVBO("Models/lantern2.obj", lanternVertexCount);
    // Lantern at center of maze
    glm::vec3 mazeCenter(0.0f, 11.0f, 0.0f); // y = 11 to sit on top of ground

    for (size_t i = 0; i < pillars.size(); i += 6) {
        const Pillar& p = pillars[i];
        float sideOffset = p.getDimensions().x / 2.0f + 0.5f;
        float lowerY = p.getPosition().y + p.getDimensions().y / 2.0f - 14.0f; // Lower by 8 units (was 5)
        glm::vec3 offsets[] = {
            glm::vec3( sideOffset, lowerY - p.getPosition().y, 0), // +X
            glm::vec3(-sideOffset, lowerY - p.getPosition().y, 0), // -X
            glm::vec3(0, lowerY - p.getPosition().y,  sideOffset), // +Z
            glm::vec3(0, lowerY - p.getPosition().y, -sideOffset)  // -Z
        };
        for (int side = 0; side < 4; ++side) {
            candles.emplace_back(p.getPosition() + offsets[side], lanternVAO, lanternVertexCount);
        }
    }

    // Debug: Write candle positions to file once per run
    {
        std::ofstream debugFile("candle_debug.txt");
        debugFile << "Candle count: " << candles.size() << std::endl;
        for (size_t i = 0; i < candles.size(); ++i) {
            debugFile << "Candle " << i << " pos: (" << candles[i].getPosition().x << ", " << candles[i].getPosition().y << ", " << candles[i].getPosition().z << ")" << std::endl;
        }
        debugFile.close();
    }
    // Staff tilt state
    bool staffTilting = false;
    float staffTiltTimer = 0.0f;
    const float staffTiltDuration = 0.25f; // seconds
    // Staff orientation at last shot
    float staffYawAtShot = 90.0f;   // horizontal angle
    float staffPitchAtShot = 0.0f;  // vertical angle
    // Entering Main Loop
    while(!glfwWindowShouldClose(window))
    {
        glm::mat4 groundWorldMatrix;
        // Frame time calculation
        float dt = glfwGetTime() - lastFrameTime;
        lastFrameTime += dt;
        // Animate staff tilt if active
        if (staffTilting) {
            staffTiltTimer += dt;
            if (staffTiltTimer >= staffTiltDuration) {
                staffTilting = false;
                staffTiltTimer = 0.0f;
            }
        }

        // --- CAMERA AND INPUT UPDATE ---
        // - Calculate mouse motion dx and dy
        // - Update camera horizontal and vertical angle
        double mousePosX, mousePosY;
        glfwGetCursorPos(window, &mousePosX, &mousePosY);

        double dx = mousePosX - lastMousePosX;
        double dy = mousePosY - lastMousePosY;

        lastMousePosX = mousePosX;
        lastMousePosY = mousePosY;

        // Convert to spherical coordinates
        const float cameraAngularSpeed = 60.0f;
        cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
        cameraVerticalAngle   -= dy * cameraAngularSpeed * dt;

        // Clamp vertical angle to [-85, 85] degrees
        cameraVerticalAngle = std::max(-85.0f, std::min(85.0f, cameraVerticalAngle));

        float theta = radians(cameraHorizontalAngle);
        float phi = radians(cameraVerticalAngle);

        cameraLookAt = vec3(
            cosf(phi) * sinf(theta),
            sinf(phi),
            cosf(phi) * cosf(theta)
        );
        vec3 cameraSideVector = glm::cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f));
        cameraSideVector = glm::normalize(cameraSideVector);

        //walktimer
        bool isWalking = 
        glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

        bool fastCam = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
               glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

        float effectiveWalkSpeed = walkSpeed;
        if (fastCam) {
            effectiveWalkSpeed *= 1.8f; // Sprint bob speed multiplier (adjust as needed)
        }

        if (isWalking) {
            walkTimer += dt * effectiveWalkSpeed;
        } else {
            walkTimer = 0.0f;
        }

        float cameraBobOffset = isWalking ? sin(walkTimer) * walkAmplitude : 0.0f;

        // This was solution for Lab02 - Moving camera exercise
        // We'll change this to be a first or third person camera
        float currentCameraSpeed = (fastCam) ? cameraFastSpeed : cameraSpeed;

        // Define proposedPosition before using it
        vec3 proposedPosition = cameraPosition;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            proposedPosition += glm::normalize(vec3(cameraLookAt.x, 0, cameraLookAt.z)) * dt * currentCameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            proposedPosition -= glm::normalize(vec3(cameraLookAt.x, 0, cameraLookAt.z)) * dt * currentCameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            proposedPosition += glm::normalize(glm::cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f))) * dt * currentCameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            proposedPosition -= glm::normalize(glm::cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f))) * dt * currentCameraSpeed;

        // Sliding collision: check X and Z separately
        auto collides = [&](const glm::vec3& pos) {
            for (const Pillar& p : pillars)
                if (p.isColliding(pos)) return true;
            return false;
        };

        vec3 tryMove = cameraPosition;
        vec3 tryX = vec3(proposedPosition.x, cameraPosition.y, cameraPosition.z);
        if (!collides(tryX)) {
            tryMove.x = proposedPosition.x;
        }
        vec3 tryZ = vec3(tryMove.x, cameraPosition.y, proposedPosition.z);
        if (!collides(tryZ)) {
            tryMove.z = proposedPosition.z;
        }
        cameraPosition = tryMove;

        // Set the view and projection matrices for first and third person cameras
        // - In first person, camera lookat is set like below
        // - In third person, camera position is on a sphere looking towards center
        if (cameraFirstPerson)
        {
            vec3 cameraPositionWithBob = cameraPosition + vec3(0.0f, cameraBobOffset, 0.0f);
            viewMatrix = lookAt(cameraPositionWithBob, cameraPositionWithBob + cameraLookAt, cameraUp );
        }
        else
        {
            // In third person view, camera is on a sphere looking at the point of interest (cameraPosition)
            float radius = 5.0f;
            vec3 position = cameraPosition - vec3(radius * cosf(phi)*cosf(theta),
                                                  radius * sinf(phi),
                                                  -radius * cosf(phi)*sinf(theta));
            viewMatrix = lookAt(position, cameraPosition, cameraUp);
        }
        // Update view and projection matrices for both shaders before drawing
        setViewMatrix(colorShaderProgram, viewMatrix);
        setViewMatrix(texturedShaderProgram, viewMatrix);
        setProjectionMatrix(colorShaderProgram, projectionMatrix);
        setProjectionMatrix(texturedShaderProgram, projectionMatrix);

        // Each frame, reset color of each pixel to glClearColor
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = glfwGetTime();
    
        // Draw textured geometry
        glUseProgram(texturedShaderProgram);
        vec3 lightPos = cameraPosition;
        vec3 lightDir = glm::normalize(cameraLookAt);

        glUseProgram(texturedShaderProgram);
        GLuint lightLoc = glGetUniformLocation(texturedShaderProgram, "lightPosition");
        glUniform3fv(lightLoc, 1, &lightPos[0]);
        GLuint lightDirLoc = glGetUniformLocation(texturedShaderProgram, "lightDirection");
        glUniform3fv(lightDirLoc, 1, &lightDir[0]);
        
        glActiveTexture(GL_TEXTURE0);
        GLuint textureLocation = glGetUniformLocation(texturedShaderProgram, "textureSampler");
        glBindTexture(GL_TEXTURE_2D, pavingStoneTextureID);
        glUniform1i(textureLocation, 0);                // Set Texture sampler to user Texture Unit 0

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pavingStoneNormalID);
        GLuint normalLocation = glGetUniformLocation(texturedShaderProgram, "normalSampler");
        glUniform1i(normalLocation, 1); // Use texture unit 1 for the normal map

        glUseProgram(texturedShaderProgram);
        GLuint uvTilingLoc = glGetUniformLocation(texturedShaderProgram, "uvTiling");
        glUniform1f(uvTilingLoc, 100.0f); // Tile floor 20x20

        // Draw ground
        glBindVertexArray(texturedCubeVAO);
    groundWorldMatrix = translate(mat4(1.0f), vec3(0.0f, -0.01f, 0.0f)) * scale(mat4(1.0f), vec3(1000.0f, 0.02f, 1000.0f));
    setWorldMatrix(texturedShaderProgram, groundWorldMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0

        // Draw pillars with Tiles089 texture and normal map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tiles089TextureID);
        glUniform1i(textureLocation, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tiles089NormalID);
        glUniform1i(normalLocation, 1);

        glUniform1f(uvTilingLoc, 4.0f); // No tiling for pillars
        glBindVertexArray(texturedCubeVAO);
        for (const Pillar& p : pillars) {
            p.draw(texturedShaderProgram);
        }
        glBindVertexArray(0);
        
        // Draw lanterns (candles) with metal texture and textured shader
        glUseProgram(texturedShaderProgram);
        setViewMatrix(texturedShaderProgram, viewMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, lanternTextureID);
        GLuint textureLocationLantern = glGetUniformLocation(texturedShaderProgram, "textureSampler");
        glUniform1i(textureLocationLantern, 0);
        glUniform1f(uvTilingLoc, 1.0f); // No tiling for lantern
        for (const Candle& candle : candles) {
            setWorldMatrix(texturedShaderProgram, glm::translate(glm::mat4(1.0f), candle.getPosition()));
            glBindVertexArray(lanternVAO);
            glDrawArrays(GL_TRIANGLES, 0, lanternVertexCount);
        }
        glBindVertexArray(0);

        // Draw lantern OBJ at center of maze with metal texture
        glm::vec3 lanternCenter(0.0f, 20.0f, 0.0f); // y = 20 for visibility
        glm::mat4 lanternWorld = glm::translate(glm::mat4(1.0f), lanternCenter) * glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));
        glUseProgram(texturedShaderProgram);
        setWorldMatrix(texturedShaderProgram, lanternWorld);
        setViewMatrix(texturedShaderProgram, viewMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, lanternTextureID);
        glUniform1i(textureLocationLantern, 0);
        glUniform1f(uvTilingLoc, 1.0f);
        glBindVertexArray(lanternVAO);
        glDisable(GL_CULL_FACE);
        glDrawArrays(GL_TRIANGLES, 0, 900);
        glEnable(GL_CULL_FACE);
        glBindVertexArray(0);

        // Update and draw projectiles
        glUseProgram(colorShaderProgram);
        glBindVertexArray(ballVAO);
        for (auto it = projectileList.begin(); it != projectileList.end(); ) {
            bool hit = false;
            // Check enemy collision
            for (auto& enemy : skeletonEnemies) {
                if (enemy.isAlive() && glm::distance(it->getPosition(), enemy.getPosition()) < 1.0f) {
                    enemy.takeDamage(50.0f);
                    impactParticleSystem.emitParticles(it->getPosition(), 32);
                    hit = true;
                    break;
                }
            }
            // Check wall collision
            if (!hit) {
                for (const Pillar& pillar : pillars) {
                    if (it->hitsPillar(pillar)) {
                        impactParticleSystem.emitParticles(it->getPosition(), 32);
                        hit = true;
                        break;
                    }
                }
            }
            // Check floor collision (y < 0.0f)
            if (!hit && it->getPosition().y < 0.0f) {
                impactParticleSystem.emitParticles(it->getPosition(), 32);
                hit = true;
            }
            if (hit) {
                it = projectileList.erase(it);
            } else {
                it->Update(dt);
                // Draw ball.obj with lava texture
                glUseProgram(texturedShaderProgram);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, lavaBallTextureID);
                GLuint textureLocationBall = glGetUniformLocation(texturedShaderProgram, "textureSampler");
                glUniform1i(textureLocationBall, 0);
                GLuint uvTilingLocBall = glGetUniformLocation(texturedShaderProgram, "uvTiling");
                glUniform1f(uvTilingLocBall, 1.0f);
                setWorldMatrix(texturedShaderProgram, glm::translate(glm::mat4(1.0f), it->getPosition()) * glm::scale(glm::mat4(1.0f), glm::vec3(0.75f)));
                glBindVertexArray(ballVAO);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glDrawArrays(GL_TRIANGLES, 0, ballVertexCount);
                glBindVertexArray(0);
                ++it;
            }
        }
        glBindVertexArray(0);

        // DEBUG: Draw a ball at maze center for visibility check
        
        // Spinning cube at camera position
        spinningCubeAngle += 180.0f * dt;
        
        // Draw avatar in view space for first person camera
        // and in world space for third person camera
        if (cameraFirstPerson)
        {
            // Render staff in view space, fixed position, but tilt in direction of camera
            glUseProgram(texturedShaderProgram);
            float tiltYaw, tiltPitch;
            float yawMult, pitchMult;
            if (staffTilting) {
                // Strong tilt in shot direction
                tiltYaw = glm::clamp(staffYawAtShot - 90.0f, -60.0f, 60.0f);
                tiltPitch = glm::clamp(staffPitchAtShot, -45.0f, 45.0f);
                yawMult = 1.0f;    // Stronger tilt when shooting
                pitchMult = 1.0f;
            } else {
                // Subtle idle tilt
                tiltYaw = glm::clamp(cameraHorizontalAngle - 90.0f, -30.0f, 30.0f);
                tiltPitch = glm::clamp(cameraVerticalAngle, -20.0f, 20.0f);
                yawMult = 0.2f;
                pitchMult = 0.5f;
            }
            glm::mat4 staffTilt = glm::rotate(glm::mat4(1.0f), glm::radians(tiltYaw * yawMult), glm::vec3(0, 1, 0));
            staffTilt = glm::rotate(staffTilt, glm::radians(-tiltPitch * pitchMult), glm::vec3(1, 0, 0));
            glm::mat4 staffModelInView =
                glm::translate(glm::mat4(1.0f), glm::vec3(0.3f, -0.3f, -1.0f))
                * staffTilt
                * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
            glm::mat4 staffWorldMatrix = glm::inverse(viewMatrix) * staffModelInView;
            setWorldMatrix(texturedShaderProgram, staffWorldMatrix);
            setViewMatrix(texturedShaderProgram, viewMatrix);
            glDisable(GL_CULL_FACE);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, staffWoodTextureID);
            GLuint staffTextureLocation = glGetUniformLocation(texturedShaderProgram, "textureSampler");
            glUniform1i(staffTextureLocation, 0);
            GLuint uvTilingLocStaff = glGetUniformLocation(texturedShaderProgram, "uvTiling");
            glUniform1f(uvTilingLocStaff, 1.0f);
            glBindVertexArray(staffVAO);
            glDrawArrays(GL_TRIANGLES, 0, staffVertexCount);
            glBindVertexArray(0);
            glEnable(GL_CULL_FACE);
            // Write staff and camera position to file for debugging
            glm::vec3 staffPos = glm::vec3(staffWorldMatrix[3][0], staffWorldMatrix[3][1], staffWorldMatrix[3][2]);
            {
                std::ofstream posFile("camera_staff_position.txt", std::ios::app);
                posFile << "Staff position: (" << staffPos.x << ", " << staffPos.y << ", " << staffPos.z << ")\n";
                posFile << "Camera position: (" << cameraPosition.x << ", " << cameraPosition.y << ", " << cameraPosition.z << ")\n";
                posFile.close();
            }
        }
        else
        {
            // In third person view, let's draw the spinning cube in world space, like any other models
            mat4 spinningCubeWorldMatrix = translate(mat4(1.0f), cameraPosition) *
                                           rotate(mat4(1.0f), radians(spinningCubeAngle), vec3(0.0f, 1.0f, 0.0f)) *
                                           scale(mat4(1.0f), vec3(0.1f, 0.1f, 0.1f));
            setWorldMatrix(colorShaderProgram, spinningCubeWorldMatrix);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            // Restore normal view matrix for color shader
            setViewMatrix(colorShaderProgram, viewMatrix);
        }

        
        // Before setting the uniforms, extract positions:
        std::vector<glm::vec3> candlePositions;
        // Add candle positions
        for (const Candle& candle : candles) {
            candlePositions.push_back(candle.getPosition());
        }
        // Add projectile positions as dynamic lights
        for (const Projectile& proj : projectileList) {
            candlePositions.push_back(proj.getPosition());
        }
        if (candlePositions.size() > 512) candlePositions.resize(512);
        // DEBUG: Print candle count and positions
        std::cout << "Candle count: " << candles.size() << ", CandlePositions used for lighting: " << candlePositions.size() << std::endl;
        for (size_t i = 0; i < candles.size(); ++i) {
            std::cout << "Candle " << i << " pos: (" << candles[i].getPosition().x << ", " << candles[i].getPosition().y << ", " << candles[i].getPosition().z << ")" << std::endl;
        }

        // Print camera position each frame
        {
            std::ofstream camDebug("camera_debug.txt", std::ios::app);
            camDebug << "Camera position: (" << cameraPosition.x << ", " << cameraPosition.y << ", " << cameraPosition.z << ")" << std::endl;
        }

        glUseProgram(texturedShaderProgram);
        GLuint candleCountLoc = glGetUniformLocation(texturedShaderProgram, "candleCount");
        glUniform1i(candleCountLoc, candlePositions.size());
        GLuint candlePosLoc = glGetUniformLocation(texturedShaderProgram, "candlePositions");
        glUniform3fv(candlePosLoc, candlePositions.size(), &candlePositions[0].x);
        GLuint timeLoc = glGetUniformLocation(texturedShaderProgram, "time");
        glUniform1f(timeLoc, time);


        // Load skeleton texture once not every frame, better fps
        static GLuint skeletonTextureID = loadTexture("Textures/skeletonDiffMap.png");
        // Update and draw skeleton enemies
            glUseProgram(skeletonShaderProgram);
            setViewMatrix(skeletonShaderProgram, viewMatrix);
            setProjectionMatrix(skeletonShaderProgram, projectionMatrix);
            float agroDistance = 7.0f; // agro distance of the enemies
            for (auto& enemy : skeletonEnemies) {
                if (enemy.isAlive() && glm::distance(enemy.getPosition(), cameraPosition) < agroDistance) {
                    enemy.SetAgro();
                }
                enemy.Update(dt, cameraPosition);
                if (enemy.isAlive()) {
                    enemy.Draw(skeletonShaderProgram, skeletonVAO, skeletonVertexCount, skeletonTextureID);
                }
            }

        // Update and render impact particles
        impactParticleSystem.update(dt);
        impactParticleSystem.render(viewMatrix, projectionMatrix);

        // End Frame
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // Handle inputs
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetWindowShouldClose(window, true);
        }
        
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) // move camera down
        {
            cameraFirstPerson = true;
        }

        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) // move camera down
        {
            cameraFirstPerson = false;
        }

        


        
        // Shoot projectiles on mouse left click
        // To detect onPress events, we need to check the last state and the current state to detect the state change
        // Otherwise, would shoot many projectiles on each mouse press
        if (lastMouseLeftState == GLFW_RELEASE && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            const float projectileSpeed = 25.0f;
            projectileList.push_back(Projectile(cameraPosition, projectileSpeed * cameraLookAt, colorShaderProgram));
            staffTilting = true;
            staffTiltTimer = 0.0f;
            // Store the camera orientation at the moment of the shot
            staffYawAtShot = cameraHorizontalAngle;
            staffPitchAtShot = cameraVerticalAngle;
        }
        lastMouseLeftState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    }

    glfwTerminate();
    
    return 0;
}

const char* getVertexShaderSource()
{
    // For now, you use a string for your shader code, in the assignment, shaders will be stored in .glsl files
    return
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "uniform mat4 worldMatrix;\n"
        "uniform mat4 viewMatrix = mat4(1.0);\n"
        "uniform mat4 projectionMatrix = mat4(1.0);\n"
        "uniform vec3 solidColor;\n"
        "out vec3 vertexColor;\n"
        "void main()\n"
        "{\n"
        "   vertexColor = solidColor;\n"
        "   mat4 modelViewProjection = projectionMatrix * viewMatrix * worldMatrix;\n"
        "   gl_Position = modelViewProjection * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}";
}

const char* getFragmentShaderSource()
{
    return
                "#version 330 core\n"
                "in vec3 vertexColor;"
                "out vec4 FragColor;"
                "void main()"
                "{"
                "   FragColor = vec4(vertexColor.r, vertexColor.g, vertexColor.b, 1.0f);"
                "}";
}

const char* getTexturedVertexShaderSource()
{
    return
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "layout (location = 2) in vec2 aUV;\n"
        "layout (location = 3) in vec3 aNormal;\n"
        "\n"
        "uniform mat4 worldMatrix;\n"
        "uniform mat4 viewMatrix;\n"
        "uniform mat4 projectionMatrix;\n"
        "uniform float uvTiling;\n" // <--- Add this line
        "\n"
        "out vec3 vertexColor;\n"
        "out vec2 vertexUV;\n"
        "out vec3 fragWorldPos;\n"
        "out vec3 fragNormal;\n"
        "\n"
        "void main()\n"
        "{\n"
        "   vertexColor = aColor;\n"
        "   vertexUV = aUV * uvTiling;\n" // <--- Use the uniform
        "   fragWorldPos = vec3(worldMatrix * vec4(aPos, 1.0));\n"
        "   fragNormal = mat3(transpose(inverse(worldMatrix))) * aNormal;\n"
        "   mat4 modelViewProjection = projectionMatrix * viewMatrix * worldMatrix;\n"
        "   gl_Position = modelViewProjection * vec4(aPos, 1.0);\n"
        "}\n";
}

const char* getTexturedFragmentShaderSource()
{
    return
    "#version 330 core\n"
    "in vec2 vertexUV;\n"
    "in vec3 fragWorldPos;\n"
    "in vec3 fragNormal;\n"
    "uniform sampler2D textureSampler;\n"
    "uniform sampler2D normalSampler;\n" 
    "uniform vec3 lightPosition;\n"
    "uniform vec3 lightDirection;\n"
    "uniform int candleCount;\n"
    "uniform vec3 candlePositions[512];\n"
    "uniform float time;\n"
    "\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   vec3 normal = normalize(fragNormal);\n"
    "   vec3 toLight = normalize(lightPosition - fragWorldPos);\n"
    "   float distance = length(lightPosition - fragWorldPos);\n"
    "   float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);\n"
    "   float ambient = 0.05;\n"
    "   float baseDiffuse = max(dot(normal, toLight), 0.0) + ambient;\n"
    "   baseDiffuse = min(baseDiffuse, 1.0);\n"
    "   float theta = dot(normalize(lightDirection), -normalize(toLight));\n"
    "   float cutoff = cos(radians(25.0));\n"      // Inner cone (smaller angle = tighter beam)
    "   float outerCutoff = cos(radians(35.0));\n" // Outer cone
    "   float intensity = smoothstep(outerCutoff, cutoff, theta);\n"
    "   intensity = pow(intensity, 1.5);\n"        // Sharper falloff
    "   float candleLight = 0.0;\n"
    "   for (int i = 0; i < candleCount; ++i) {\n"
    "       float dist = length(candlePositions[i] - fragWorldPos);\n"
    "       float att = 1.0 / (1.0 + 0.2 * dist + 0.05 * dist * dist);\n"
    "       vec3 toCandle = normalize(candlePositions[i] - fragWorldPos);\n"
    "       float flicker = 0.85 + 0.15 * sin(time * 7.0 + float(i) * 3.0 + fragWorldPos.x * 0.2 + fragWorldPos.z * 0.2);\n"
    "       candleLight += max(dot(normal, toCandle), 0.0) * att * flicker;\n"
    "   }\n"
    "   candleLight = min(candleLight, 1.0);\n"
    "   float diffuse = baseDiffuse * intensity * attenuation + candleLight * 0.5;\n"
    "   vec4 textureColor = texture(textureSampler, vertexUV);\n"
    "   FragColor = vec4(diffuse * textureColor.rgb, textureColor.a);\n"
    "}\n";
}

int compileAndLinkShaders(const char* vertexShaderSource, const char* fragmentShaderSource)
{
    // compile and link shader program
    // return shader program id
    // ------------------------------------

    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        {
            std::ofstream shaderDebug("shader_debug.txt", std::ios::app);
            shaderDebug << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }
    
    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        {
            std::ofstream shaderDebug("shader_debug.txt", std::ios::app);
            shaderDebug << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }
    
    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        {
            std::ofstream shaderDebug("shader_debug.txt", std::ios::app);
            shaderDebug << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

GLuint loadTexture(const char *filename)
{
    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if(!data){
        std::ofstream texDebug("texture_debug.txt", std::ios::app);
        texDebug << "Failed to load texture: " << filename << std::endl;
        texDebug.close();
        return 0; // Return 0 if texture loading failed
    }
    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    assert(textureId != 0);

    glBindTexture(GL_TEXTURE_2D, textureId);

    //set filler parameters 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // upload texture to the pu
    GLenum format =0;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    //free resurces
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // unbind texture
    return textureId; //TODO: replace with texture loading code
}

int createTexturedCubeVertexArrayObject()
{
    // Create a vertex array
    GLuint vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);
    
    // Upload Vertex Buffer to the GPU, keep a reference to it (vertexBufferObject)
    GLuint vertexBufferObject;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texturedCubeVertexArray), texturedCubeVertexArray, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0,                   // attribute 0 matches aPos in Vertex Shader
                          3,                   // size
                          GL_FLOAT,            // type
                          GL_FALSE,            // normalized?
                          sizeof(TexturedColoredVertex), // stride - each vertex contain 2 vec3 (position, color)
                          (void*)0             // array buffer offset
                          );
    glEnableVertexAttribArray(0);
    
    
    glVertexAttribPointer(1,                            // attribute 1 matches aColor in Vertex Shader
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(TexturedColoredVertex),
                          (void*)sizeof(vec3)      // color is offseted a vec3 (comes after position)
                          );
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2,                            // attribute 2 matches aUV in Vertex Shader
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(TexturedColoredVertex),
                          (void*)(2*sizeof(vec3))      // uv is offseted by 2 vec3 (comes after position and color)
                          );
    glEnableVertexAttribArray(2);

    // Block for normals (attribute 3)
    glVertexAttribPointer(3,                            // attribute 3 matches aNormal in Vertex Shader
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(TexturedColoredVertex),
                          (void*)(2*sizeof(vec3) + sizeof(vec2)) // normal is after position, color, uv
                          );
    glEnableVertexAttribArray(3);

    return vertexArrayObject;
}