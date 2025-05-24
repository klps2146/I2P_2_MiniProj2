#include <allegro5/base.h>
#include <random>
#include <string>

#include "Scene/PlayScene.hpp"
#include "Engine/Point.hpp"
#include "GodEnemy.hpp"

const float set_HP = 600;

GodEnemy::GodEnemy(int x, int y)
    : Enemy("play/enemy-6.png", x, y, 20, 20, set_HP, 250),
      head("play/enemy-10.png", x, y), 
      buff_picture("play/gold-circle.png", x, y, buff_radus * 2, buff_radus * 2), targetRotation(0) {
    buff_picture.Visible = false;
    buff_picture.Tint = al_map_rgba(255, 255, 255, 200);
}
void GodEnemy::Draw() const {
    Enemy::Draw();
    head.Draw();
    if (buff_picture.Visible) {
        buff_picture.Draw();
    }
}
void GodEnemy::Update(float deltaTime) {
    Enemy::Update(deltaTime);
    head.Position = Position;
    // Choose arbitrary one.
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<> dist(0.0f, 4.0f);
    float rnd = dist(rng);
    if (rnd < deltaTime) {
        // Head arbitrary rotation.
        std::uniform_real_distribution<> distRadian(-ALLEGRO_PI, ALLEGRO_PI);
        targetRotation = distRadian(rng);
    }
    head.Rotation = (head.Rotation + deltaTime * targetRotation) / (1 + deltaTime);

    //// new

    float hp_rate = hp / set_HP;
    float real_buff_freq = base_buff_freq * (0.2 + 0.8 * hp_rate);

    buff_picture.Position = Position;

    if (buff_duration_timer > 0){
        buff_duration_timer -= deltaTime;
        buff_picture.Visible = true;
    }
    else{
        buff_picture.Visible = false;
    }

    buff_timer += deltaTime;
    if (buff_timer >= real_buff_freq){
        buff_timer = 0;
        buff_duration_timer = buff_duration;
    }

    if (buff_duration_timer > 0){
        buff_timer = 0;
        auto* scene = getPlayScene();
        buff_picture.Visible = true;
        for (auto &obj : scene->EnemyGroup->GetObjects()){
            Enemy *enemy = dynamic_cast<Enemy*>(obj);
            if (enemy == nullptr || enemy == this) continue;
            float dx = enemy->Position.x - Position.x;
            float dy = enemy->Position.y - Position.y;
            if (dx*dx + dy*dy <= buff_radus*buff_radus){
                enemy->change_speed(2.5, 3.5);
            }
        }
        
    }
    else buff_picture.Visible = false;


}
