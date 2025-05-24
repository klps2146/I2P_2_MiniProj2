#ifndef SHOVELTOOL_HPP
#define SHOVELTOOL_HPP
#include "Turret/Turret.hpp"

class ShovelTool : public Turret{
public:
    ShovelTool(float x, float y);
    void OnClick(int x, int y);
    void CreateBullet() override;
};
#endif   // SHOVELTOOL_HPP
