#include <algorithm>
#include <allegro5/allegro.h>
#include <cmath>
#include <fstream>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "Enemy/Enemy.hpp"
// new
#include "Enemy/PlaneEnemy.hpp"
#include "Enemy/GodEnemy.hpp"
#include "Turret/CoolTurret.hpp"
#include "Tool/ShovelTool.hpp"
//
#include "Enemy/SoldierEnemy.hpp"
#include "Enemy/TankEnemy.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/LOG.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "Turret/LaserTurret.hpp"
#include "Turret/MachineGunTurret.hpp"
#include "Turret/TurretButton.hpp"
#include "UI/Animation/DirtyEffect.hpp"
#include "UI/Animation/Plane.hpp"
#include "UI/Component/Label.hpp"

// OK TODO HACKATHON-4 (1/3): Trace how the game handles keyboard input.
// OK TODO HACKATHON-4 (2/3): Find the cheat code sequence in this file.
// OK TODO HACKATHON-4 (3/3): When the cheat code is entered, a plane should be spawned and added to the scene.
// OK TODO HACKATHON-5 (1/4): There's a bug in this file, which crashes the game when you win. Try to find it.
// OK TODO HACKATHON-5 (2/4): The "LIFE" label are not updated when you lose a life. Try to fix it.

bool PlayScene::DebugMode = false;
const std::vector<Engine::Point> PlayScene::directions = { Engine::Point(-1, 0), Engine::Point(0, -1), Engine::Point(1, 0), Engine::Point(0, 1) };
const int PlayScene::MapWidth = 20, PlayScene::MapHeight = 13;
const int PlayScene::BlockSize = 64;
const float PlayScene::DangerTime = 7.61;
const Engine::Point PlayScene::SpawnGridPoint = Engine::Point(-1, 0);
const Engine::Point PlayScene::EndGridPoint = Engine::Point(MapWidth, MapHeight - 1);
const std::vector<int> PlayScene::code = {
    ALLEGRO_KEY_UP, ALLEGRO_KEY_UP, ALLEGRO_KEY_DOWN, ALLEGRO_KEY_DOWN,
    ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT, ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT,
    ALLEGRO_KEY_B, ALLEGRO_KEY_A, ALLEGRO_KEY_LSHIFT, ALLEGRO_KEY_ENTER
};
Engine::Point PlayScene::GetClientSize() {
    return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
}
void PlayScene::Initialize() {
    mapState.clear();
    keyStrokes.clear();
    //// new
    while (!level_req.empty()) level_req.pop();
    level_req.push(100);
    level_req.push(150);
    level_req.push(200);
    level_req.push(700);
    level_req.push(1500);
    level_req.push(3000);
    level_req.push(6000);
    level_req.push(10451);
    player_exp = player_level = 0;
    player_skill_point = 1;
    turret_coin_mul = turret_coolDown_mul = 1.0f;
    coin_lv = coolDown_lv = 0;

    ticks = 0;
    deathCountDown = -1;
    lives = 10;
    money = 666;
    SpeedMult = 1;
    // Add groups from bottom to top.
    AddNewObject(TileMapGroup = new Group());
    AddNewObject(GroundEffectGroup = new Group());
    AddNewObject(DebugIndicatorGroup = new Group());
    AddNewObject(TowerGroup = new Group());
    AddNewObject(EnemyGroup = new Group());
    AddNewObject(BulletGroup = new Group());
    AddNewObject(EffectGroup = new Group());
    // Should support buttons.
    AddNewControlObject(UIGroup = new Group());
    ReadMap();
    ReadEnemyWave();
    mapDistance = CalculateBFSDistance();
    ConstructUI();
    imgTarget = new Engine::Image("play/target.png", 0, 0);
    imgTarget->Visible = false;
    preview = nullptr;
    UIGroup->AddNewObject(imgTarget);
    // Preload Lose Scene
    deathBGMInstance = Engine::Resources::GetInstance().GetSampleInstance("astronomia.ogg");
    Engine::Resources::GetInstance().GetBitmap("lose/benjamin-happy.png");
    // Start BGM.
    bgmId = AudioHelper::PlayBGM("play.ogg");
}
void PlayScene::Terminate() {
    AudioHelper::StopBGM(bgmId);
    AudioHelper::StopSample(deathBGMInstance);
    deathBGMInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}
void PlayScene::Update(float deltaTime) {
    // If we use deltaTime directly, then we might have Bullet-through-paper problem.
    // Reference: Bullet-Through-Paper
    if (SpeedMult == 0)
        deathCountDown = -1;
    else if (deathCountDown != -1)
        SpeedMult = 1;
    // Calculate danger zone.
    std::vector<float> reachEndTimes;
    for (auto &it : EnemyGroup->GetObjects()) {
        reachEndTimes.push_back(dynamic_cast<Enemy *>(it)->reachEndTime);
    }
    // Can use Heap / Priority-Queue instead. But since we won't have too many enemies, wsorting is fast enough.
    std::sort(reachEndTimes.begin(), reachEndTimes.end());
    float newDeathCountDown = -1;
    int danger = lives;
    for (auto &it : reachEndTimes) {
        if (it <= DangerTime) {
            danger--;
            if (danger <= 0) {
                // Death Countdown
                float pos = DangerTime - it;
                if (it > deathCountDown) {
                    // Restart Death Count Down BGM.
                    AudioHelper::StopSample(deathBGMInstance);
                    if (SpeedMult != 0)
                        deathBGMInstance = AudioHelper::PlaySample("astronomia.ogg", false, AudioHelper::BGMVolume, pos);
                }
                float alpha = pos / DangerTime;
                alpha = std::max(0, std::min(255, static_cast<int>(alpha * alpha * 255)));
                dangerIndicator->Tint = al_map_rgba(255, 255, 255, alpha);
                newDeathCountDown = it;
                break;
            }
        }
    }
    deathCountDown = newDeathCountDown;
    if (SpeedMult == 0)
        AudioHelper::StopSample(deathBGMInstance);
    if (deathCountDown == -1 && lives > 0) {
        AudioHelper::StopSample(deathBGMInstance);
        dangerIndicator->Tint.a = 0;
    }
    if (SpeedMult == 0)
        deathCountDown = -1;
    for (int i = 0; i < SpeedMult; i++) {
        IScene::Update(deltaTime);
        // Check if we should create new enemy.
        ticks += deltaTime;
        if (enemyWaveData.empty()) {
            if (EnemyGroup->GetObjects().empty()) {
                // Free resources.
                /*delete TileMapGroup;
                delete GroundEffectGroup;
                delete DebugIndicatorGroup;
                delete TowerGroup;
                delete EnemyGroup;
                delete BulletGroup;
                delete EffectGroup;
                delete UIGroup;
                delete imgTarget;*/
                // Win.
                std::ofstream ofs("../Resource/score_tmp.txt", std::ios::out);
                ofs << money * 2 + lives * 495;
                Engine::GameEngine::GetInstance().ChangeScene("win");
            }
            continue;
        }
        auto current = enemyWaveData.front();
        if (ticks < current.second)
            continue;
        ticks -= current.second;
        enemyWaveData.pop_front();
        const Engine::Point SpawnCoordinate = Engine::Point(SpawnGridPoint.x * BlockSize + BlockSize / 2, SpawnGridPoint.y * BlockSize + BlockSize / 2);
        Enemy *enemy;
        switch (current.first) {
            case 1:
                EnemyGroup->AddNewObject(enemy = new SoldierEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
                break;
            // OK TODO HACKATHON-3 (2/3): Add your new enemy here.
            // case 2:
            //     ..
            //// new
            case 2:
                EnemyGroup->AddNewObject(enemy = new PlaneEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
                break;
            case 3:
                EnemyGroup->AddNewObject(enemy = new TankEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
                break;
            case 4:
                EnemyGroup->AddNewObject(enemy = new GodEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
                break;
            default:
                continue;
        }
        enemy->UpdatePath(mapDistance);
        // Compensate the time lost.
        enemy->Update(ticks);
    }
    if (preview) {
        preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
        // To keep responding when paused.
        preview->Update(deltaTime);
    }
}
void PlayScene::Draw() const {
    IScene::Draw();
    if (DebugMode) {
        // Draw reverse BFS distance on all reachable blocks.
        for (int i = 0; i < MapHeight; i++) {
            for (int j = 0; j < MapWidth; j++) {
                if (mapDistance[i][j] != -1) {
                    // Not elegant nor efficient, but it's quite enough for debugging.
                    Engine::Label label(std::to_string(mapDistance[i][j]), "pirulen.ttf", 32, (j + 0.5) * BlockSize, (i + 0.5) * BlockSize);
                    label.Anchor = Engine::Point(0.5, 0.5);
                    label.Draw();
                }
            }
        }
    }
}
void PlayScene::OnMouseDown(int button, int mx, int my) {
    if ((button & 1) && !imgTarget->Visible && preview) {
        // Cancel turret construct.
        UIGroup->RemoveObject(preview->GetObjectIterator());
        preview = nullptr;
    }
    IScene::OnMouseDown(button, mx, my);
}
void PlayScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
    const int x = mx / BlockSize;
    const int y = my / BlockSize;
    if (!preview || x < 0 || x >= MapWidth || y < 0 || y >= MapHeight) {
        imgTarget->Visible = false;
        return;
    }
    imgTarget->Visible = true;
    imgTarget->Position.x = x * BlockSize;
    imgTarget->Position.y = y * BlockSize;
}
void PlayScene::OnMouseUp(int button, int mx, int my) {
    IScene::OnMouseUp(button, mx, my);
    if (!imgTarget->Visible)
        return;
    const int x = mx / BlockSize;
    const int y = my / BlockSize;
    // & 1 左鍵 & 2 右鍵 & 4 中鍵
    if (button & 1) {
        //// new shovel
        if (mapState[y][x] == TILE_OCCUPIED){
            if (dynamic_cast<ShovelTool*>(preview) != nullptr){
                dynamic_cast<ShovelTool*>(preview)->OnClick(x, y);
                UIGroup->RemoveObject(preview->GetObjectIterator());
                preview = nullptr;
                return;
            }
        }
      
        if (mapState[y][x] != TILE_OCCUPIED) {
            if (!preview)
                return;

            //// new
            if (dynamic_cast<ShovelTool*>(preview) != nullptr){
                UIGroup->RemoveObject(preview->GetObjectIterator());
                preview = nullptr;
                return;
            }
            
            // 放置Turret
            // Check if valid.
            if (!CheckSpaceValid(x, y)) {
                Engine::Sprite *sprite;
                GroundEffectGroup->AddNewObject(sprite = new DirtyEffect("play/target-invalid.png", 1, x * BlockSize + BlockSize / 2, y * BlockSize + BlockSize / 2));
                sprite->Rotation = 0;
                return;
            }
            // Purchase.
            EarnMoney(-preview->GetPrice());
            // Remove Preview.
            preview->GetObjectIterator()->first = false;
            UIGroup->RemoveObject(preview->GetObjectIterator());
            // Construct real turret.
            preview->Position.x = x * BlockSize + BlockSize / 2;
            preview->Position.y = y * BlockSize + BlockSize / 2;

            //// new
            preview->coolDown = preview->coolDown * turret_coolDown_mul;

            preview->Enabled = true;
            preview->Preview = false;
            preview->Tint = al_map_rgba(255, 255, 255, 255);

            // 這裡
            TowerGroup->AddNewObject(preview);
            // To keep responding when paused.
            preview->Update(0);
            // Remove Preview.
            preview = nullptr;

            mapState[y][x] = TILE_OCCUPIED;
            OnMouseMove(mx, my);
        }
    }
}

void PlayScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    // new (for count cheat code)
    static int idx = 0;
    if (PlayScene::code[idx] == keyCode) {
        idx++; 
        // std::cout << "PROC:" << idx << std::endl;
    }
    else {
        idx = 0; 
        // std::cout << "WRONG KEY SEQUENCE" << std::endl;
    }

    // FAST P chaet key======================================================================================================
    // FAST P chaet key======================================================================================================
    // FAST P chaet key======================================================================================================
    // FAST P chaet key======================================================================================================
    // FAST P chaet key======================================================================================================
    if (/*keyCode == ALLEGRO_KEY_P ||*/idx >= PlayScene::code.size()){
        // std::cout <<"EXPLOSION!!!!" << std::endl;
        UIGroup->AddNewObject(new Plane);
        money += 10000;
        idx = 0;
    }
    //

    if (keyCode == ALLEGRO_KEY_TAB) {
        DebugMode = !DebugMode;
    } else {
        keyStrokes.push_back(keyCode);
        if (keyStrokes.size() > code.size())
            keyStrokes.pop_front();
    }
    if (keyCode == ALLEGRO_KEY_Q) {
        // Hotkey for MachineGunTurret.
        UIBtnClicked(0);
    } else if (keyCode == ALLEGRO_KEY_W) {
        // Hotkey for LaserTurret.
        UIBtnClicked(1);
    }
    else if (keyCode == ALLEGRO_KEY_E){
        UIBtnClicked(2); //// new
    }
    else if (keyCode >= ALLEGRO_KEY_0 && keyCode <= ALLEGRO_KEY_9) {
        // Hotkey for Speed up.
        SpeedMult = keyCode - ALLEGRO_KEY_0;
    }
}
void PlayScene::Hit() {
    lives--;
    if (lives <= 0) {
        Engine::GameEngine::GetInstance().ChangeScene("lose");
    }
    // new
    UILives ->Text = std::string("Life ") + std::to_string(this->lives);
}
int PlayScene::GetMoney() const {
    return money;
}
void PlayScene::EarnMoney(int money) {
    //// modify
    this->money += (money > 0) ? money * turret_coin_mul : money;
    UIMoney->Text = std::string("$") + std::to_string(this->money);

    //// new 
    // 只有earnmoney 才會升等
    if (money == 10000) return;
    if (player_exp >= level_req.front() && player_level < 8){
        float divid = player_exp - level_req.front();
        player_level += 1;
        player_skill_point += 1;
        player_exp = divid;
        level_req.pop();
    }
    else if (player_level >= 8){
        player_exp_l->Text = std::string("EXP: MAX");
        player_level_l->Text = std::string("Level: " + std::to_string((int)player_level) + "/8");
        player_skill_point_l->Text = std::string("Point: " + std::to_string((int)player_skill_point));
        return;
    }
    player_exp_l->Text = std::string("EXP: " + std::to_string((int)player_exp)) + "/" + std::to_string(level_req.front());
    player_level_l->Text = std::string("Level: " + std::to_string((int)player_level) + "/8");
    player_skill_point_l->Text = std::string("Point: " + std::to_string((int)player_skill_point));
}
void PlayScene::ReadMap() {
    std::string filename = std::string("Resource/map") + std::to_string(MapId) + ".txt";
    // Read map file.
    char c;
    std::vector<bool> mapData;
    std::ifstream fin(filename);
    while (fin >> c) {
        switch (c) {
            case '0': mapData.push_back(false); break;
            case '1': mapData.push_back(true); break;
            case '\n':
            case '\r':
                if (static_cast<int>(mapData.size()) / MapWidth != 0)
                    throw std::ios_base::failure("Map data is corrupted.");
                break;
            default: throw std::ios_base::failure("Map data is corrupted.");
        }
    }
    fin.close();
    // Validate map data.
    if (static_cast<int>(mapData.size()) != MapWidth * MapHeight)
        throw std::ios_base::failure("Map data is corrupted.");
    // Store map in 2d array.
    mapState = std::vector<std::vector<TileType>>(MapHeight, std::vector<TileType>(MapWidth));
    for (int i = 0; i < MapHeight; i++) {
        for (int j = 0; j < MapWidth; j++) {
            const int num = mapData[i * MapWidth + j];
            mapState[i][j] = num ? TILE_FLOOR : TILE_DIRT;
            if (num)
                TileMapGroup->AddNewObject(new Engine::Image("play/floor.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
            else
                TileMapGroup->AddNewObject(new Engine::Image("play/dirt.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
        }
    }
}

void PlayScene::ReadEnemyWave() {
    std::string filename = std::string("Resource/enemy") + std::to_string(MapId) + ".txt";
    // Read enemy file.
    float type, wait, repeat;
    enemyWaveData.clear();
    std::ifstream fin(filename);
    while (fin >> type && fin >> wait && fin >> repeat) {
        for (int i = 0; i < repeat; i++)
            enemyWaveData.emplace_back(type, wait);
    }
    fin.close();
}

void PlayScene::ConstructUI() {
    // Background
    UIGroup->AddNewObject(new Engine::Image("play/sand.png", 1280, 0, 320, 832));
    // Text
    UIGroup->AddNewObject(new Engine::Label(std::string("Stage ") + std::to_string(MapId), "pirulen.ttf", 32, 1294, 0));
    UIGroup->AddNewObject(UIMoney = new Engine::Label(std::string("$") + std::to_string(money), "pirulen.ttf", 24, 1294, 48));
    UIGroup->AddNewObject(UILives = new Engine::Label(std::string("Life ") + std::to_string(lives), "pirulen.ttf", 24, 1294, 88));
    
    //// new
    UIGroup->AddNewObject(player_exp_l = new Engine::Label(std::string("EXP ") + std::to_string((int)player_exp) + "/" + std::to_string((int)level_req.front()), "pirulen.ttf", 24, 1294, 130));
    UIGroup->AddNewObject(player_level_l= new Engine::Label(std::string("Level ") + std::to_string((int)player_level) + "/8", "pirulen.ttf", 24, 1294, 155));
    UIGroup->AddNewObject(player_skill_point_l = new Engine::Label(std::string("Points: ") + std::to_string((int)player_skill_point), "pirulen.ttf", 24, 1294, 180));
    
    TurretButton *btn;
    // Button 1
    btn = new TurretButton("play/floor.png", "play/dirt.png",
                           Engine::Sprite("play/tower-base.png", 1294, 136+112, 0, 0, 0, 0),
                           Engine::Sprite("play/turret-1.png", 1294, 136+112 - 8, 0, 0, 0, 0), 1294, 136+112, MachineGunTurret::Price);
    // Reference: Class Member Function Pointer and std::bind.
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 0));
    UIGroup->AddNewControlObject(btn);
    // Button 2
    btn = new TurretButton("play/floor.png", "play/dirt.png",
                           Engine::Sprite("play/tower-base.png", 1370, 136+112, 0, 0, 0, 0),
                           Engine::Sprite("play/turret-2.png", 1370, 136+112 - 8, 0, 0, 0, 0), 1370, 136+112, LaserTurret::Price);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 1));
    UIGroup->AddNewControlObject(btn);
    //// new
    btn = new TurretButton("play/floor.png", "play/dirt.png",
                           Engine::Sprite("play/tower-base.png", 1446, 136+112, 0, 0, 0, 0),
                           Engine::Sprite("play/turret-6.png", 1446, 136+112 - 8, 0, 0, 0, 0), 1446, 136+112, CoolTurret::Price);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 2));
    UIGroup->AddNewControlObject(btn);
    // 鏟子
    btn = new TurretButton("play/floor.png", "play/dirt.png",
                           Engine::Sprite("play/sand.png", 1294, 700, 0, 0, 0, 0),
                           Engine::Sprite("play/shovel.png", 1294, 700, 0, 0, 0, 0), 1294, 700, 0);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 666));
    UIGroup->AddNewControlObject(btn);
    
    /// 新家
    btn = new TurretButton("play/floor.png", "play/dirt.png",
                           Engine::Sprite("play/sand.png", 1294, 231+212, 0, 0, 0, 0),
                           Engine::Sprite("play/coin.png", 1294, 231+212, 0, 0, 0, 0), 1294, 231+212, 0);

    UIGroup->AddNewObject(coin_lv_l= new Engine::Label("LV. " + std::to_string(coin_lv), "pirulen.ttf", 30, 1294+70, 231+212+10));
    btn->SetOnClickCallback(std::bind(&PlayScene::buff_adder, this, 0));
    UIGroup->AddNewControlObject(btn);

        btn = new TurretButton("play/floor.png", "play/dirt.png",
                           Engine::Sprite("play/sand.png", 1294, 231+212+78, 0, 0, 0, 0),
                           Engine::Sprite("play/speed-up.png", 1294, 231+212+78, 0, 0, 0, 0), 1294, 231+212+78, 0);

    UIGroup->AddNewObject(coolDown_lv_l= new Engine::Label("LV. " + std::to_string(coolDown_lv), "pirulen.ttf", 30, 1294+70, 231+212+78+10));
    btn->SetOnClickCallback(std::bind(&PlayScene::buff_adder, this, 1));
    UIGroup->AddNewControlObject(btn);

    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int shift = 135 + 25;
    dangerIndicator = new Engine::Sprite("play/benjamin.png", w - shift, h - shift);
    dangerIndicator->Tint.a = 0;
    UIGroup->AddNewObject(dangerIndicator);
}

//// new
// strength 0 / sped up 1
void PlayScene::buff_adder(int state){
    if (player_skill_point <= 0) return;
    player_skill_point -= 1;
    player_skill_point_l->Text = std::string("Point: " + std::to_string((int)player_skill_point));
    std::cout << "ADD BUFF" << state << std::endl;
    if (state == 0){
        turret_coin_mul *= 1.17;
        coin_lv += 1;
        coin_lv_l->Text = "LV." + std::to_string(coin_lv);
    }
    else if (state == 1){
        turret_coolDown_mul *= 0.88;
        coolDown_lv += 1;
        coolDown_lv_l->Text = "LV." + std::to_string(coolDown_lv);    
    }
}

void PlayScene::UIBtnClicked(int id) {
    if (preview)
        UIGroup->RemoveObject(preview->GetObjectIterator());
    if (id == 0 && money >= MachineGunTurret::Price)
        preview = new MachineGunTurret(0, 0);
    else if (id == 1 && money >= LaserTurret::Price)
        preview = new LaserTurret(0, 0);
    else if (id == 2 && money >= CoolTurret::Price)
        preview = new CoolTurret(0, 0);  //// new

    if (id == 666){
        preview = new ShovelTool(0, 0);
    }
    
    if (!preview)
        return;
    preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
    preview->Tint = al_map_rgba(255, 255, 255, 200);
    preview->Enabled = false;
    preview->Preview = true;
    UIGroup->AddNewObject(preview);
    OnMouseMove(Engine::GameEngine::GetInstance().GetMousePosition().x, Engine::GameEngine::GetInstance().GetMousePosition().y);
}

bool PlayScene::CheckSpaceValid(int x, int y) {
    if (x < 0 || x >= MapWidth || y < 0 || y >= MapHeight)
        return false;
    auto map00 = mapState[y][x];
    mapState[y][x] = TILE_OCCUPIED;
    std::vector<std::vector<int>> map = CalculateBFSDistance();
    mapState[y][x] = map00;
    if (map[0][0] == -1)
        return false;
    for (auto &it : EnemyGroup->GetObjects()) {
        Engine::Point pnt;
        pnt.x = floor(it->Position.x / BlockSize);
        pnt.y = floor(it->Position.y / BlockSize);
        if (pnt.x < 0) pnt.x = 0;
        if (pnt.x >= MapWidth) pnt.x = MapWidth - 1;
        if (pnt.y < 0) pnt.y = 0;
        if (pnt.y >= MapHeight) pnt.y = MapHeight - 1;
        if (map[pnt.y][pnt.x] == -1)
            return false;
    }
    // All enemy have path to exit.
    mapState[y][x] = TILE_OCCUPIED;
    mapDistance = map;
    for (auto &it : EnemyGroup->GetObjects())
        dynamic_cast<Enemy *>(it)->UpdatePath(mapDistance);
    return true;
}
std::vector<std::vector<int>> PlayScene::CalculateBFSDistance() {
    // Reverse BFS to find path.
    std::vector<std::vector<int>> map(MapHeight, std::vector<int> (std::vector<int>(MapWidth, -1)));
    std::queue<Engine::Point> que;
    // Push end point.
    // BFS from end point.
    if (mapState[MapHeight - 1][MapWidth - 1] != TILE_DIRT)
        return map;
    que.push(Engine::Point(MapWidth - 1, MapHeight - 1));
    map[MapHeight - 1][MapWidth - 1] = 0;
    while (!que.empty()) {
        Engine::Point p = que.front();
        que.pop();

        // TODO PROJECT-1 (1/1): Implement a BFS starting from the most right-bottom block in the map.
        //               For each step you should assign the corresponding distance to the most right-bottom block.
        //               mapState[y][x] is TILE_DIRT if it is empty.
        
        const int dx[4] = {1, -1, 0, 0};
        const int dy[4] = {0, 0, 1, -1};

        for (int i = 0; i < 4; i++){
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];
            if (nx < 0 || nx >= MapWidth || ny < 0 || ny >= MapHeight) continue;
            if (mapState[ny][nx] == TILE_DIRT && map[ny][nx] == -1){
                map[ny][nx] = map[p.y][p.x] + 1; // 下一格的距離是上一格+1
                que.push(Engine::Point(nx, ny)); // 這次(下次上一格)的位置 
            }

        }
        
    }
    return map;
}
