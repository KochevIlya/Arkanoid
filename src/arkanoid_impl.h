#pragma once

#include "arkanoid.h"

#define USE_ARKANOID_IMPL

class ArkanoidImpl : public Arkanoid
{
public:


    void reset(const ArkanoidSettings& settings) override;
    void update(ImGuiIO& io, ArkanoidDebugData& debug_data, float elapsed) override;
    void draw(ImGuiIO& io, ImDrawList& draw_list) override;
    void apply_bonus(BonusType type) override;
private:

    enum GameState{
        PLAYING,
        PAUSE,
        GAME_OVER,
        VICTORY
    };

    struct Ball {
        Vect pos;
        Vect vel;
        bool active = true;
    };

    void draw_bricks(ImDrawList& draw_list);
    void draw_balls(ImDrawList& draw_list);
    void draw_bonuses(ImDrawList& draw_list);
    void draw_carriage(ImDrawList& draw_list);
    void draw_ui(ImGuiIO& io, ImDrawList& draw_list);
    void draw_menus(ImGuiIO& io, ImDrawList& draw_list);
    void update_carriage(ImGuiIO& io, float elapsed);
    void update_balls(float elapsed, ArkanoidDebugData& debug_data, bool& any_brick_alive);
    void update_bonuses(float elapsed);
    void add_debug_hit(ArkanoidDebugData& debug_data, Vect world_pos, Vect normal);

    ArkanoidSettings saved_settings;
    GameState state = PLAYING;

    std::vector<Brick> bricks;
    std::vector<Bonus> bonuses;
    std::vector<Ball> balls;

    Vect carriage_pos;
    Vect world_size;
    Vect world_to_screen_scale;

    float game_time = 0.0f;
    int score = 0;

    float ball_radius = 10.0f;
    float carriage_width = 100.0f;
    float carriage_height = 20.0f;
    float carriage_speed = 1.0f;
    float carriage_offset = 30.0f;

    float ball_initial_speed = 300.0f;
    float bonus_initial_speed = 150.0f;
    float brick_height = 20.0f;
    float top_brick_padding = 50.0f;
    float top_ui_text_padding = 20.0f;

    int bonus_spawn_percentage = 30;

};
