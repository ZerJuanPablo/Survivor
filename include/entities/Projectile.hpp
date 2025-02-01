#pragma once
#include <glm/glm.hpp>

class Projectile {
protected:
    glm::vec2 position;
    glm::vec2 direction;
    float speed;
    float damage;
    float lifetime;
    float radius; // For collision detection
    bool isAlive;

public:
    Projectile(glm::vec2 startPos, glm::vec2 dir, float spd, float dmg, float life)
        : position(startPos), direction(glm::normalize(dir)), 
          speed(spd), damage(dmg), lifetime(life), radius(5.0f), isAlive(true) {}

    virtual ~Projectile() = default;

    // Update projectile position and state
    virtual void Update(float deltaTime) {
        position += direction * speed * deltaTime;
        lifetime -= deltaTime;
        if (lifetime <= 0) isAlive = false;
    }

    // Pure virtual function for rendering (implementation depends on bullet type)
    virtual void Render() const = 0;

    // Getters
    bool IsAlive() const { return isAlive; }
    glm::vec2 GetPosition() const { return position; }
    float GetRadius() const { return radius; }
    float GetDamage() const { return damage; }

    // Setters
    void SetAlive(bool alive) { isAlive = alive; }
    void SetDirection(glm::vec2 newDir) { direction = glm::normalize(newDir); }
};

// Example of a basic straight-moving bullet
class BasicBullet : public Projectile {
public:
    BasicBullet(glm::vec2 startPos, glm::vec2 dir, float spd = 500.0f, 
                float dmg = 10.0f, float life = 2.0f)
        : Projectile(startPos, dir, spd, dmg, life) {
        radius = 3.0f;
    }

    void Update(float deltaTime) override {
        Projectile::Update(deltaTime);
        // Add any specific behavior here
    }


};

// Example of a homing projectile
class HomingBullet : public Projectile {
    glm::vec2 targetPosition;
    float rotation;
    float homingStrength;

public:
    HomingBullet(glm::vec2 startPos, glm::vec2 dir, glm::vec2 target, 
                 float spd = 400.0f, float dmg = 15.0f, float life = 3.0f)
        : Projectile(startPos, dir, spd, dmg, life), 
          targetPosition(target), homingStrength(5.0f), rotation(0.0f) {
        radius = 4.0f;
    }

    void Update(float deltaTime) override {
        // Calculate direction towards target
        glm::vec2 toTarget = targetPosition - position;
        direction = glm::mix(direction, glm::normalize(toTarget), homingStrength * deltaTime);
        direction = glm::normalize(direction);
        
        Projectile::Update(deltaTime);
        rotation += 10.0f * deltaTime;
    }

    void Render() const override {

    }
};