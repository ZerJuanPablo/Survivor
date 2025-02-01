#include <vector>
#include <memory>
#include "entities/Projectile.hpp"

class BulletManager {
private:
    std::vector<std::unique_ptr<Projectile>> projectiles;

public:
    void AddProjectile(std::unique_ptr<Projectile> projectile) {
        projectiles.emplace_back(std::move(projectile));
    }

    void Update(float deltaTime) {
        for (auto it = projectiles.begin(); it != projectiles.end();) {
            (*it)->Update(deltaTime);
            if (!(*it)->IsAlive()) {
                it = projectiles.erase(it);
            } else {
                ++it;
            }
        }
    }

    void Render() {
        for (auto& projectile : projectiles) {
            projectile->Render();
        }
    }

    // Add collision checking and other necessary methods
};