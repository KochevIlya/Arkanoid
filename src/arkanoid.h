#pragma once

#include "base.h"
#include <vector>

struct ArkanoidSettings
{
    static constexpr int bricks_columns_min = 10;
    static constexpr int bricks_columns_max = 30;
    static constexpr int bricks_rows_min = 3;
    static constexpr int bricks_rows_max = 10;
    static constexpr int bricks_min_percentage = 0;
    static constexpr int bricks_max_percentage = 100;

    static constexpr int bonus_min_percentage = 0;
    static constexpr int bonus_max_percentage = 100;

    static constexpr float bricks_columns_padding_min = 5.0f;
    static constexpr float bricks_columns_padding_max = 20.0f;
    static constexpr float bricks_rows_padding_min = 5.0f;
    static constexpr float bricks_rows_padding_max = 20.0f;

    static constexpr float ball_radius_min = 5.0f;
    static constexpr float ball_radius_max = 50.0f;
    static constexpr float ball_speed_min = 1.0f;
    static constexpr float ball_speed_max = 1000.0f;

    static constexpr float carriage_speed_min = 0.1f;
    static constexpr float carriage_speed_max = 3.0f;
    static constexpr float carriage_width_min = 5.0f;
    static constexpr float carriage_width_max = 100.0f;
    static constexpr float carriage_height_min = 20.0f;
    static constexpr float carriage_height_max = 50.0f;

    static constexpr float bonus_min_speed = 1.0f;
    static constexpr float bonus_max_speed = 400.0f;


    Vect world_size = Vect(800.0f, 600.f);

    int bricks_count_percentage = 80;
    int bricks_columns_count = 15;
    int bricks_rows_count = 7;

    int bonus_spawn_percentage = 30;
    float bonus_initial_speed = 150.0f;

    float bricks_columns_padding = 5.0f;
    float bricks_rows_padding = 5.0f;

    float ball_radius = 10.0f;
    float ball_speed = 300.0f;

    float carriage_height = 20.0f;
    float carriage_width = 100.0f;
    float carriage_speed = 1.0f;
};

struct ArkanoidDebugData
{
    struct Hit
    {
        Vect screen_pos;        // Hit position, in screen space
        Vect normal;            // Hit normal
        float time = 0.0f;      // leave it default
    };
    
    std::vector<Hit> hits;
};
enum BonusType {
    SCORE, MULTIPLY
};
struct Bonus {
    BonusType type;
    Vect pos;
    bool active = true;
    float radius = 8.0f;
};
struct Brick {
    Vect pos;
    Vect size;
    bool destroyed = false;
    int type = 0;
    ImColor color;
};

class Arkanoid
{
public:
    virtual ~Arkanoid() = default;
    virtual void reset(const ArkanoidSettings& settings) = 0;
    virtual void draw(ImGuiIO& io, ImDrawList& draw_list) = 0;
    virtual void update(ImGuiIO& io, ArkanoidDebugData& debug_data, float elapsed) = 0;
    virtual void apply_bonus(BonusType type) = 0;

};

extern Arkanoid* create_arkanoid();
