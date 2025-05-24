#include <allegro5/base.h>
#include <cmath>
#include <string>
#include <iostream>

#include "Bullet/CoolBullet.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/Group.hpp"
#include "Engine/Point.hpp"
#include "ShovelTool.hpp"
#include "Scene/PlayScene.hpp"

// 借用Turret 不用重新實作preview
ShovelTool::ShovelTool(float x, float y) : Turret("play/tool-base.png", "play/shovel.png", x, y, 0, 0, 0) {
    Anchor = Engine::Point(0.5, 0.5);
}

void ShovelTool::OnClick(int x, int y){
    std::cout << "CLICK SHOVEL\n";
    auto* scene = getPlayScene();
    for (auto& obj : scene->TowerGroup->GetObjects()){ // std::list<IObject *> list;
        Turret* turret = dynamic_cast<Turret*>(obj);
        if (turret == nullptr) continue;
        int tx = (turret->Position.x - PlayScene::BlockSize / 2)/ PlayScene::BlockSize;
        int ty = (turret->Position.y - PlayScene::BlockSize / 2)/ PlayScene::BlockSize;
        std::cout << x << " " << y << "state" << scene->mapState[ty][tx] << "\n";
        std::cout <<"T:" << tx << " " << ty << "\n";
        std::cout <<"Val:" << (tx==x) << " " << (ty==y) << "\n";
        if (tx == x && ty == y){
            std::cout << "CLICK TURRET: " << tx << " " << ty << std::endl; 
            scene->EarnMoney(turret->GetPrice() / 4);
            scene->TowerGroup->RemoveObject(obj->GetObjectIterator());
            scene->mapState[y][x] = PlayScene::TileType::TILE_FLOOR;
            return;
        }
    }
}

void ShovelTool::CreateBullet(){}
