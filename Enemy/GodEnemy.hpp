#ifndef GODENEMY_HPP
#define GODENEMY_HPP
#include "Enemy.hpp"
#include "Engine/Sprite.hpp"

class GodEnemy : public Enemy {
private:
    const float buff_radus = 110.0f;

    Sprite head;
    Engine::Sprite buff_picture;
    float targetRotation;

    float buff_timer = 0; // 計算frequency 要不要開始duration_time
    const float buff_duration = 1.5;
    float buff_duration_timer = 0;  // 是否還在持續buff

    float base_buff_freq = 5.0f;

public:
    GodEnemy(int x, int y);
    void Draw() const override;
    void Update(float deltaTime) override;
};
#endif   // GODENEMY_HPP
