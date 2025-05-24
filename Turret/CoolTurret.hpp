#ifndef COOLTURRET_HPP
#define COOLTURRET_HPP
#include "Turret.hpp"

class CoolTurret : public Turret {
public:
    static const int Price;
    CoolTurret(float x, float y);
    void CreateBullet() override;
};
#endif   // COOLTURRET_HPP
