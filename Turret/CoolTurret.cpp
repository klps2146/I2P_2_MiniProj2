#include <allegro5/base.h>
#include <cmath>
#include <string>

#include "Bullet/CoolBullet.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/Group.hpp"
#include "Engine/Point.hpp"
#include "CoolTurret.hpp"
#include "Scene/PlayScene.hpp"

const int CoolTurret::Price = 490;
CoolTurret::CoolTurret(float x, float y) : Turret("play/tower-base.png", "play/turret-6.png", x, y, 180, Price, 2) {
    // Move center downward, since we the turret head is slightly biased upward.
    Anchor.y += 8.0f / GetBitmapHeight();
}
void CoolTurret::CreateBullet() {
    Engine::Point diff = Engine::Point(cos(Rotation - ALLEGRO_PI / 2), sin(Rotation - ALLEGRO_PI / 2));
    float rotation = atan2(diff.y, diff.x);
    Engine::Point normalized = diff.Normalize();
    Engine::Point normal = Engine::Point(-normalized.y, normalized.x);
    // Change bullet position to the front of the gun barrel.
    getPlayScene()->BulletGroup->AddNewObject(new CoolBullet(Position + normalized * 36 - normal * 6, diff, rotation, this));
    getPlayScene()->BulletGroup->AddNewObject(new CoolBullet(Position + normalized * 36 + normal * 6, diff, rotation, this));
    AudioHelper::PlayAudio("laser.wav");
}