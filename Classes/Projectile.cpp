#include "../Headers/Projectile.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>

// Forward declaration for setWorldMatrix
void setWorldMatrix(int shaderProgram, glm::mat4 worldMatrix);

Projectile::Projectile(glm::vec3 position, glm::vec3 velocity, int shaderProgram)
    : mPosition(position), mVelocity(velocity)
{
    mWorldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");
}

void Projectile::Update(float dt) {
    mPosition += mVelocity * dt;
}

void Projectile::Draw() {
    glm::mat4 worldMatrix = glm::translate(glm::mat4(1.0f), mPosition)
        * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
    glUniformMatrix4fv(mWorldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 36); // tried all the possible primitives, still getting holes in the projectile... 
}
glm::vec3 Projectile::getPosition() const {
    return mPosition;
}
bool Projectile::hitsPillar(const Pillar& pillar) const {
    return pillar.isCollidingwithProjectile(mPosition);
}