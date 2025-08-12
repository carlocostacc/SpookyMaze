#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>

enum SkeletonState { Idle, Agro };

class StandardSkeletonEnemy {
public:
    StandardSkeletonEnemy(const glm::vec3& spawnPosition);
    void Update(float dt, const glm::vec3& playerPosition);
    void Draw(int shaderProgram, GLuint skeletonVAO, int skeletonVertexCount, GLuint skeletonTextureID);
    glm::vec3 getPosition() const;
    bool isAlive() const;
    void takeDamage(float amount);
    bool isFlashing() const;
    void SetAgro();

private:
    glm::vec3 position;
    glm::vec3 spawnPosition;
    float health;
    float Idle_speed;
    float Agro_speed;
    bool alive;
    float flashTimer; // Time remaining for red flash

    // Rectangle patrol
    int patrolCorner; // 0-3, which corner is the skeleton moving toward
    float patrolTimer; // Time spent moving toward current corner
    static constexpr float RECT_WIDTH = 6.0f; // X size
    static constexpr float RECT_HEIGHT = 0.0f; // Y stays constant
    static constexpr float RECT_LENGTH = 10.0f; // Z size
    static constexpr float CORNER_REACH_DIST = 0.2f;

    float yaw; // Rotation angle in degrees
    SkeletonState state;
};
