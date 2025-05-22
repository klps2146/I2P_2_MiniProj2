#include <allegro5/allegro_audio.h>
#include <functional>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm> // for sort

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "ScoreboardScene.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Slider.hpp"

void ScoreboardScene::Initialize() {
    page = 0;
    idx = 0;
    names.clear();
    score.clear();
    score_int.clear();
    dates.clear();

    const int NUM_PER_PAGE = 10;
    const int TITLE_GAP = 100;
    const int WIDTH_GAP = 50;

    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;
    int top_h = halfH / 5 - 25;
    int bottom_h = halfH * 3 / 2 + 125;

    AddNewObject(new Engine::Label("SCOREBOARD", "pirulen.ttf", 54, halfW, halfH / 5 - 25, 200, 120, 255, 255, 0.5, 0.5));
    Engine::ImageButton *btn;

    for (int i = 0; i < NUM_PER_PAGE; i++){
        int now_h = top_h + TITLE_GAP + (i%10) * WIDTH_GAP;
        AddNewObject(obj_list1[i] = new Engine::Label("EMPTY", "pirulen.ttf", 36, halfW - 350, now_h, 200, 177, 255, 255, 0.5, 0.5));
        AddNewObject(obj_list2[i] = new Engine::Label("EMPTY", "pirulen.ttf", 36, halfW + 125, now_h, 200, 177, 255, 255, 0.5, 0.5));
        AddNewObject(obj_list3[i] = new Engine::Label("EMPTY", "pirulen.ttf", 22, halfW + 350, now_h + 5, 200, 199, 255, 255, 0.5, 0.5));
    }

    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - 320, halfH * 3 / 2 + 75, 100, 100);
    btn->SetOnClickCallback(std::bind(&ScoreboardScene::update_page, this, 1));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("<", "pirulen.ttf", 48, halfW - 270, halfH * 3 / 2 + 125, 0, 0, 0, 255, 0.5, 0.5));

    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW + 220, halfH * 3 / 2 + 75, 100, 100);
    btn->SetOnClickCallback(std::bind(&ScoreboardScene::update_page, this, 2));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label(">", "pirulen.ttf", 48, halfW + 270, halfH * 3 / 2 + 125, 0, 0, 0, 255, 0.5, 0.5));

    idx = 0;
    if (load_score_file()){
        update_page(0);
    }
    else std::cout << "[ERROR] Invalid scoreboard datas!\n";
    ////

    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - 200, halfH * 3 / 2 + 75, 400, 100);
    btn->SetOnClickCallback(std::bind(&ScoreboardScene::BackOnClick, this, 1));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "pirulen.ttf", 48, halfW, halfH * 3 / 2 + 125, 0, 0, 0, 255, 0.5, 0.5));

    // Not safe if release resource while playing, however we only free while change scene, so it's fine.
    bgmInstance = AudioHelper::PlaySample("select.ogg", true, AudioHelper::BGMVolume);
}
void ScoreboardScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}
void ScoreboardScene::BackOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
}
////
bool ScoreboardScene::load_score_file(){
    std::ifstream ifs("../Resource/scoreboard.txt", std::ios::in);
    std::string buf;
    for (int cnt = 0; ifs >> buf; cnt++){
        if (cnt == 0){
            names.push_back(buf);
            std::cout << "NEW: " << buf << " ";
        }
        else if (cnt == 1){
            std::stringstream ss(buf);
            int tmp; ss >> tmp;
            score.push_back(buf);
            score_int.push_back(tmp);
            std::cout << buf << " ";
        }
        else if (cnt == 2){
            dates.push_back(buf);
            std::cout << buf << "\n";
            idx++;
            cnt = -1;
        }
        else return 0;
    }
    need_pages = idx/10 + (idx%10 != 0 ? 1 : 0);
    ifs.close();
    sorting_by_score();
    return 1;
}

void ScoreboardScene::sorting_by_score(){
    std::vector<int> indexx(score_int.size());
    for (int i = 0; i < score_int.size(); i++){
        indexx[i] = i;
    }
    std::sort(indexx.begin(), indexx.end(),
        [&](int a, int b) {
            return score_int[a] > score_int[b];
        }
    );
    std::vector<std::string> sorted_names, sorted_score, sorted_dates;
    std::vector<int> sorted_score_int;

    for (int i : indexx){
        sorted_names.push_back(names[i]);
        sorted_score.push_back(score[i]);
        sorted_score_int.push_back(score_int[i]);
        sorted_dates.push_back(dates[i]);
    }

    names = sorted_names;
    score = sorted_score;
    score_int = sorted_score_int;
    dates = sorted_dates;
}


void ScoreboardScene::update_page(int state){
    if (state == 1){
        (page > 0) ? --page : page;
    }
    else if (state == 2){
        (page+1 < need_pages) ? ++page : page;
    }
    int i;
    for (i = page*10; i < page*10 + 10; i++){
        obj_list1[i%10]->Text = (i >= idx)?"":names[i];
        obj_list2[i%10]->Text = (i >= idx)?"":score[i];
        obj_list3[i%10]->Text = (i >= idx)?"":dates[i];
    }
}