#ifndef PLAYSCENE_HPP
#define PLAYSCENE_HPP
#include <allegro5/allegro_audio.h>
#include <list>
#include <memory>
#include <utility>
#include <vector>
#include <queue>

#include "Engine/IScene.hpp"
#include "Engine/Point.hpp"

class Turret;
namespace Engine {
    class Group;
    class Image;
    class Label;
    class Sprite;
}   // namespace Engine

class PlayScene final : public Engine::IScene {
public: //// new 本來是 privte
    enum TileType {
        TILE_DIRT,
        TILE_FLOOR,
        TILE_OCCUPIED,
    };
    ALLEGRO_SAMPLE_ID bgmId;
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> deathBGMInstance;

protected:
    int lives;
    int money;
    int SpeedMult;

public:
    //// new
    float turret_coin_mul = 1.0f; 
    float turret_coolDown_mul = 1.0f; // 越小 頻率越高

    int coin_lv = 0;
    int coolDown_lv = 0;
    Engine::Label* coin_lv_l;
    Engine::Label* coolDown_lv_l = 0;

    float player_exp;
    float player_level = 0;
    int player_skill_point = 1;
    Engine::Label* player_exp_l;
    Engine::Label* player_level_l;
    Engine::Label* player_skill_point_l;


    void buff_adder(int state);

    std::queue<int> level_req;

    static bool DebugMode;
    static const std::vector<Engine::Point> directions;
    static const int MapWidth, MapHeight;
    static const int BlockSize;
    static const float DangerTime;
    static const Engine::Point SpawnGridPoint;
    static const Engine::Point EndGridPoint;
    static const std::vector<int> code;
    int MapId;
    float ticks;
    float deathCountDown;
    // Map tiles.
    Group *TileMapGroup;
    Group *GroundEffectGroup;
    Group *DebugIndicatorGroup;
    Group *BulletGroup;
    Group *TowerGroup;
    Group *EnemyGroup;
    Group *EffectGroup;
    Group *UIGroup;
    Engine::Label *UIMoney;
    Engine::Label *UILives;
    Engine::Image *imgTarget;
    Engine::Sprite *dangerIndicator;
    Turret *preview;
    std::vector<std::vector<TileType>> mapState;
    std::vector<std::vector<int>> mapDistance;
    std::list<std::pair<int, float>> enemyWaveData;
    std::list<int> keyStrokes;
    static Engine::Point GetClientSize();
    explicit PlayScene() = default;
    void Initialize() override;
    void Terminate() override;
    void Update(float deltaTime) override;
    void Draw() const override;
    void OnMouseDown(int button, int mx, int my) override;
    void OnMouseMove(int mx, int my) override;
    void OnMouseUp(int button, int mx, int my) override;
    void OnKeyDown(int keyCode) override;
    void Hit();
    int GetMoney() const;
    void EarnMoney(int money);
    void ReadMap();
    void ReadEnemyWave();
    void ConstructUI();
    void UIBtnClicked(int id);
    bool CheckSpaceValid(int x, int y);
    std::vector<std::vector<int>> CalculateBFSDistance();
    // void ModifyReadMapTiles();
};
#endif   // PLAYSCENE_HPP
