#ifndef SCOREBOARDSCENE_HPP
#define SCOREBOARDSCENE_HPP
#include <allegro5/allegro_audio.h>
#include <memory>

#include "Engine/IScene.hpp"

class ScoreboardScene final : public Engine::IScene {
private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    int page = 0;

    int idx = 0;
    int need_pages;
    Engine::Label *obj_list1[11];
    Engine::Label *obj_list2[11];
    Engine::Label *obj_list3[11];
    std::vector<std::string> names;
    std::vector<std::string> score;
    std::vector<int> score_int;
    std::vector<std::string> dates;

    bool load_score_file();
    void update_page(int page);
    void update_switch_btn(int page);
    void sorting_by_score();    
public:
    explicit ScoreboardScene() = default;
    void Initialize() override;
    void Terminate() override;
    void BackOnClick(int stage);
};



#endif   // SCOREBOARDSCENE_HPP
