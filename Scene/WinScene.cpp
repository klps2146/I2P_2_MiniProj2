#include <functional>
#include <string>
#include <fstream>
#include <ctime>
#include <iostream>
#include <sstream>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "PlayScene.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "WinScene.hpp"

void WinScene::Initialize() {
    ticks = 0;
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    AddNewObject(new Engine::Label("You Win! ENTER YOUR NAME:", "pirulen.ttf", 48, halfW, halfH / 5, 255, 255, 255, 255, 0.5, 0.5));
    AddNewObject(name_label = new Engine::Label("......", "pirulen.ttf", 36, halfW, halfH / 5 + 53, 0, 255, 255, 200, 0.5, 0.5));

    std::ifstream ifs("../Resource/score_tmp.txt", std::ios::in);
    ifs >> score;

    AddNewObject(new Engine::Label("SCORE:", "pirulen.ttf", 42, halfW + 300, halfH - 130, 255, 255, 255, 200, 0.5, 0.5));
    AddNewObject(new Engine::Label(score, "pirulen.ttf", 38, halfW + 425, halfH - 130, 255, 255, 255, 200, 0.0, 0.5));


    AddNewObject(new Engine::Image("win/benjamin-sad.png", halfW, halfH, 0, 0, 0.5, 0.5));
    Engine::ImageButton *btn;
    btn = new Engine::ImageButton("win/dirt.png", "win/floor.png", halfW - 200, halfH * 7 / 4 - 50, 400, 100);
    btn->SetOnClickCallback(std::bind(&WinScene::BackOnClick, this, 2));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Skip", "pirulen.ttf", 48, halfW, halfH * 7 / 4, 0, 0, 0, 255, 0.5, 0.5));
    bgmId = AudioHelper::PlayAudio("win.wav");
}
void WinScene::Terminate() {
    IScene::Terminate();
    AudioHelper::StopBGM(bgmId);
}
void WinScene::Update(float deltaTime) {
    ticks += deltaTime;
    if (ticks > 4 && ticks < 100 &&
        dynamic_cast<PlayScene *>(Engine::GameEngine::GetInstance().GetScene("play"))->MapId == 2) {
        ticks = 100;
        bgmId = AudioHelper::PlayBGM("happy.ogg");
    }
}
void WinScene::BackOnClick(int stage) {
    // Change to select scene.
    if (stage == 1){
        save_date();
    }
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
}

void WinScene::OnKeyDown(int keyCode) {
    if (keyCode >= ALLEGRO_KEY_A && keyCode <= ALLEGRO_KEY_Z){ // a ~ z
        user_name += keyCode-1+'a';
        name_label->Text = user_name;
    }
    else if (keyCode >= ALLEGRO_KEY_0 && keyCode <= ALLEGRO_KEY_9){
        user_name += keyCode-27+'0';
        name_label->Text = user_name;
    }
    else if (keyCode == 61){
        user_name += "-";
        name_label->Text = user_name;
    }
    else if (keyCode == ALLEGRO_KEY_BACKSPACE){
        user_name.pop_back();
        name_label->Text = user_name;
    }

    if (keyCode == ALLEGRO_KEY_ENTER){
        BackOnClick(1);
    }
}
#include <filesystem>
void WinScene::save_date(){
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    std::cout << (now->tm_year + 1900) << "-"<< (now->tm_mon + 1) << "-"<< now->tm_mday << std::endl;
    
    std::ofstream ofs("../Resource/scoreboard.txt", std::ios::out | std::ios::app);
    ofs << user_name << " " << score << " " << (now->tm_year + 1900) << "-"<< (now->tm_mon + 1) << "-"<< now->tm_mday << "\n";
    user_name = "";
    score = "";
}