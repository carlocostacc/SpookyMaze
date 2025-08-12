#include <GL/glew.h>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../Headers/StandardSkeletonEnemy.h"

// Declaration for setWorldMatrix 
void setWorldMatrix(int shaderProgram, glm::mat4 worldMatrix);


StandardSkeletonEnemy::StandardSkeletonEnemy(const glm::vec3& spawnPosition)
    : position(spawnPosition), spawnPosition(spawnPosition), health(100.0f), Idle_speed(1.5f), Agro_speed(2.5f), alive(true), flashTimer(0.0f), patrolCorner(0), patrolTimer(0.0f), yaw(0.0f), state(Idle) {}

void StandardSkeletonEnemy::Update(float dt, const glm::vec3& playerPosition) {
        // Debug: log yaw value to skeleton_debug.txt
        {
            static std::ofstream yawLog("skeleton_debug.txt", std::ios::app);
            yawLog << "Yaw: " << yaw << " Position: (" << position.x << ", " << position.y << ", " << position.z << ")\n";
        }
    if (!alive) return;
    if (state == Idle) {
        // Rectangle patrol logic (idle)
        glm::vec3 corners[4] = {
            spawnPosition + glm::vec3(-RECT_WIDTH/2, RECT_HEIGHT, -RECT_LENGTH/2),
            spawnPosition + glm::vec3( RECT_WIDTH/2, RECT_HEIGHT, -RECT_LENGTH/2),
            spawnPosition + glm::vec3( RECT_WIDTH/2, RECT_HEIGHT,  RECT_LENGTH/2),
            spawnPosition + glm::vec3(-RECT_WIDTH/2, RECT_HEIGHT,  RECT_LENGTH/2)
        };
        glm::vec3 target = corners[patrolCorner];
        glm::vec3 toTarget = target - position;
        float dist = glm::length(toTarget);
        if (dist < CORNER_REACH_DIST) {
            patrolCorner = (patrolCorner + 1) % 4;
            patrolTimer = 0.0f;
        } else {
            glm::vec3 dir = glm::normalize(toTarget);
            position += dir * Idle_speed * dt;
            patrolTimer += dt;
        }
        // Always face the next corner, regardless of movement direction
        glm::vec3 faceVec = corners[patrolCorner] - position;
        if (glm::length(glm::vec2(faceVec.x, faceVec.z)) > 0.001f) {
            float baseYaw = glm::degrees(atan2(faceVec.z, faceVec.x));
            if (patrolCorner == 0 || patrolCorner == 2) {
                yaw = baseYaw - 90.0f;
            } else {
                yaw = baseYaw + 90.0f;
            }
        }
    } else if (state == Agro) {
        // Always face the player
        glm::vec3 toPlayer = playerPosition - position;
        glm::vec2 toPlayerXZ(toPlayer.x, toPlayer.z);
        if (glm::length(toPlayerXZ) > 0.001f) {
            float baseYaw = -1*glm::degrees(atan2(toPlayer.z, toPlayer.x));
            yaw = baseYaw + 90.0f;
        }
        // Move toward player
        float dist = glm::length(toPlayer);
        if (dist > 0.01f) {
            glm::vec3 dir = glm::normalize(toPlayer);
            position += dir * Agro_speed * dt;
        }
    }
    if (flashTimer > 0.0f) {
        flashTimer -= dt;
        if (flashTimer < 0.0f) flashTimer = 0.0f;
    }
}

// Public method to trigger Agro state
void StandardSkeletonEnemy::SetAgro() {
    state = Agro;
}


void StandardSkeletonEnemy::Draw(int shaderProgram, GLuint skeletonVAO, int skeletonVertexCount, GLuint skeletonTextureID) {
    glUseProgram(shaderProgram);
    // Side-to-side rocking animation (pendulum effect)
    float rockAmplitude = 15.0f; // degrees, adjust for desired effect
    float rockSpeed = 6.0f;      // adjust for desired speed
    float rockAngle = 0.0f;
    if (state == Idle) {
        rockAngle = sin(patrolTimer * rockSpeed) * rockAmplitude;
    } else if (state == Agro) {
        // Use flashTimer as a proxy for elapsed time in Agro, or add a new timer if needed
        static float agroTimer = 0.0f;
        agroTimer += 0.016f; // Approximate frame time, or pass dt if available
        rockAngle = sin(agroTimer * rockSpeed) * rockAmplitude;
    }
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0, 1, 0));
    glm::mat4 rocking = glm::rotate(glm::mat4(1.0f), glm::radians(rockAngle), glm::vec3(0, 0, 1));
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position)
        * rotation
        * rocking
        * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
    setWorldMatrix(shaderProgram, model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, skeletonTextureID);
    glUniform1i(glGetUniformLocation(shaderProgram, "diffuseMap"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "isFlashing"), isFlashing());
    glBindVertexArray(skeletonVAO);
    glDrawArrays(GL_TRIANGLES, 0, skeletonVertexCount);
    glBindVertexArray(0);
}

glm::vec3 StandardSkeletonEnemy::getPosition() const {
    return position;
}

bool StandardSkeletonEnemy::isAlive() const {
    return alive;
}

void StandardSkeletonEnemy::takeDamage(float amount) {
    if (!alive) return;
    health -= amount;
    flashTimer = 0.2f; // Flash red for 0.2 seconds
    if (health <= 0.0f) {
        alive = false;
    }
}

bool StandardSkeletonEnemy::isFlashing() const {
    return flashTimer > 0.0f;
}
