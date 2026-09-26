#ifndef DATA_H
#define DATA_H

#include <vector>
#include <cmath>

enum ZombieType {
    NORMAL,
    RUNNER,
    STRAFER
};

struct BloodParticle {
    float x, y, z;
    float vx, vy, vz;
    float life;
    float r, g, b;

    BloodParticle(float px, float py, float pz)
        : x(px), y(py), z(pz), life(1.0f) {
        vx = ((rand() % 100) - 50) / 100.0f * 3.0f;
        vy = ((rand() % 100) - 20) / 100.0f * 3.0f;
        vz = ((rand() % 100) - 50) / 100.0f * 3.0f;
        r = 0.7f + ((rand() % 30) / 100.0f); g = 0.0f; b = 0.0f;
    }
};

struct Zombie {
    float x, y, z;
    float health;
    float speed;
    float radius;

    ZombieType type;
    float startX;
    float timeAlive;

    bool isAttacking;
    float attackTimer;
    bool hasHead;

    Zombie(float startX, float startZ, ZombieType t)
        : x(startX), startX(startX), y(1.8f), z(startZ),
        type(t), timeAlive(0.0f),
        isAttacking(false), attackTimer(0.0f), hasHead(true) {

        switch (type) {
        case RUNNER:
            health = 100.0f; speed = 6.0f; break;
        case STRAFER:
            health = 100.0f; speed = 2.5f; break;
        case NORMAL:
            health = 100.0f; speed = 2.5f; break;
        }
        radius = 0.25f;
    }

    void update(float deltaTime) {
        timeAlive += deltaTime;
        if (!isAttacking) {
            z += speed * deltaTime;

            if (type == STRAFER) {
                x = startX + sin(timeAlive * 3.0f) * 0.5f;
            }
        }
    }
};

extern float playerPosX;
extern float playerPosY;
extern float playerPosZ;

extern std::vector<Zombie> zumbis;
extern float barricadeHealth;
extern float barricadeZPosition;
extern int ammo;
extern bool gameOver;

extern float currentYaw;
extern float startYaw;
extern float endYaw;

extern bool isTurning;
extern float turnTimer;
const float turnDuration = 0.3f;

extern std::vector<BloodParticle> bloodParticles;

#endif