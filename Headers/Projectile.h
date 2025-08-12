#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "Pillar.h"

class Projectile {
public:
    Projectile(glm::vec3 position, glm::vec3 velocity, int shaderProgram);
    void Update(float dt);
    void Draw();
    glm::vec3 getPosition() const;
    bool hitsPillar(const Pillar& pillar) const;
private:
    GLuint mWorldMatrixLocation;
    glm::vec3 mPosition;
    glm::vec3 mVelocity;
};