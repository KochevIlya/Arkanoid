#include "arkanoid_impl.h"
#include <map>
#include <iostream>
#include <GLFW/glfw3.h>

#ifdef USE_ARKANOID_IMPL
Arkanoid* create_arkanoid()
{
    return new ArkanoidImpl();
}
#endif

std::map<std::string, ImColor> colors = {
    {"red",    ImColor(255, 50,  50,  200)},
    {"green",  ImColor(50,  255, 50,  200)},
    {"blue",   ImColor(50,  100, 255, 200)},
    {"yellow", ImColor(255, 255, 50,  200)},
};
std::vector<std::string> color_names = {"red", "green", "blue", "yellow"};

void ArkanoidImpl::apply_bonus(BonusType type) {
    if (type == SCORE)
        score += 50;
    else if (type == MULTIPLY) {

        Vect spawn_pos = Vect(carriage_pos.x, carriage_pos.y - (carriage_height * 0.5f + ball_radius + 1.0f));

        Ball b1;
        b1.pos = spawn_pos;
        b1.vel = Vect(-ball_initial_speed * 0.5f, -ball_initial_speed * 0.8f);
        b1.active = true;

        Ball b2;
        b2.pos = spawn_pos;
        b2.vel = Vect(0.0f, -ball_initial_speed);
        b2.active = true;

        Ball b3;
        b3.pos = spawn_pos;
        b3.vel = Vect(ball_initial_speed * 0.5f, -ball_initial_speed * 0.8f);
        b3.active = true;

        balls.push_back(b1);
        balls.push_back(b2);
        balls.push_back(b3);
    }
}

void ArkanoidImpl::update_carriage(ImGuiIO& io, float elapsed) {
    float carriage_vel = world_size.x * carriage_speed;

    if (io.KeysDown[GLFW_KEY_A]) carriage_pos.x -= carriage_vel * elapsed;
    if (io.KeysDown[GLFW_KEY_D]) carriage_pos.x += carriage_vel * elapsed;

    float half_carriage = carriage_width / 2.0f;
    if (carriage_pos.x < half_carriage) carriage_pos.x = half_carriage;
    if (carriage_pos.x > world_size.x - half_carriage) carriage_pos.x = world_size.x - half_carriage;
}

void ArkanoidImpl::update_balls(float elapsed, ArkanoidDebugData& debug_data, bool& any_brick_alive) {
    float half_carriage_width = carriage_width / 2.0f;
    float carriage_top = carriage_pos.y - carriage_height / 2.0f;

    for (Ball& ball : balls) {
        if (!ball.active) continue;
        ball.pos += ball.vel * elapsed;

        if (ball.pos.x < ball_radius && ball.vel.x < 0) { ball.vel.x *= -1; add_debug_hit(debug_data, ball.pos, Vect(1, 0)); }
        if (ball.pos.x > world_size.x - ball_radius && ball.vel.x > 0) { ball.vel.x *= -1; add_debug_hit(debug_data, ball.pos, Vect(-1, 0)); }
        if (ball.pos.y < ball_radius && ball.vel.y < 0) { ball.vel.y *= -1; add_debug_hit(debug_data, ball.pos, Vect(0, 1)); }

        if (ball.pos.y > world_size.y - ball_radius && ball.vel.y > 0) {
            ball.active = false;
            add_debug_hit(debug_data, ball.pos, Vect(0, -1));
            continue;
        }

        if (ball.vel.y > 0 && ball.pos.y + ball_radius >= carriage_top && ball.pos.y - ball_radius <= carriage_top + carriage_height) {
            if (ball.pos.x >= carriage_pos.x - half_carriage_width && ball.pos.x <= carriage_pos.x + half_carriage_width) {
                ball.pos.y = carriage_top - ball_radius - 0.1f;
                ball.vel.y *= -1;
                ball.vel.x += ((ball.pos.x - carriage_pos.x) / half_carriage_width) * ball_initial_speed;
                add_debug_hit(debug_data, ball.pos, Vect(0, -1));
            }
        }

        for (Brick& brick : bricks) {
            if (brick.destroyed) continue;
            any_brick_alive = true;

            float cx = std::max(brick.pos.x, std::min(ball.pos.x, brick.pos.x + brick.size.x));
            float cy = std::max(brick.pos.y, std::min(ball.pos.y, brick.pos.y + brick.size.y));
            float dx = ball.pos.x - cx;
            float dy = ball.pos.y - cy;

            if (dx*dx + dy*dy < ball_radius * ball_radius) {
                score += 10;
                brick.destroyed = true;
                if (std::rand() % 100 < bonus_spawn_percentage) {
                    Bonus b;
                    b.pos = brick.pos + brick.size * 0.5f;
                    b.active = true;
                    b.type = (std::rand() % 2 == 0) ? MULTIPLY : SCORE;
                    bonuses.push_back(b);
                }
                if (std::abs(dx) > std::abs(dy)) ball.vel.x *= -1; else ball.vel.y *= -1;
                add_debug_hit(debug_data, Vect(cx, cy), (std::abs(dx) > std::abs(dy)) ? Vect(dx > 0 ? 1.f : -1.f, 0) : Vect(0, dy > 0 ? 1.f : -1.f));
                break;
            }
        }
    }
}

void ArkanoidImpl::update_bonuses(float elapsed) {
    float half_carriage_width = carriage_width / 2.0f;

    for (Bonus& bonus : bonuses) {
        if (!bonus.active) continue;
        bonus.pos.y += bonus_initial_speed * elapsed;

        if (std::abs(bonus.pos.x - carriage_pos.x) < half_carriage_width && std::abs(bonus.pos.y - carriage_pos.y) < carriage_height / 2.0f) {
            bonus.active = false;
            apply_bonus(bonus.type);
        }
        if (bonus.pos.y > world_size.y) bonus.active = false;
    }
}

void ArkanoidImpl::draw_bricks(ImDrawList& draw_list) {
    for (const Brick& brick : bricks) {
        if (brick.destroyed) continue;
        ImVec2 p_min(brick.pos.x * world_to_screen_scale.x, brick.pos.y * world_to_screen_scale.y);
        ImVec2 p_max((brick.pos.x + brick.size.x) * world_to_screen_scale.x, (brick.pos.y + brick.size.y) * world_to_screen_scale.y);
        draw_list.AddRectFilled(p_min, p_max, brick.color);
    }
}

void ArkanoidImpl::draw_balls(ImDrawList& draw_list) {
    float ballRadiusScreen = ball_radius * world_to_screen_scale.x;
    for (const Ball& ball : balls) {
        if (!ball.active) continue;
        ImVec2 center(ball.pos.x * world_to_screen_scale.x, ball.pos.y * world_to_screen_scale.y);
        draw_list.AddCircleFilled(center, ballRadiusScreen, ImColor(100, 255, 100));
    }
}

void ArkanoidImpl::add_debug_hit(ArkanoidDebugData& debug_data, Vect world_pos, Vect normal) {
    ArkanoidDebugData::Hit hit;
    hit.screen_pos = Vect(world_pos.x * world_to_screen_scale.x, world_pos.y * world_to_screen_scale.y);
    hit.normal = normal;
    hit.time = 0.0f;
    debug_data.hits.push_back(hit);
}

void ArkanoidImpl::draw_bonuses(ImDrawList& draw_list) {
    for (const Bonus& bonus : bonuses) {
        if (!bonus.active) continue;
        ImVec2 center(bonus.pos.x * world_to_screen_scale.x, bonus.pos.y * world_to_screen_scale.y);
        float radius = bonus.radius * world_to_screen_scale.x;
        ImColor color = (bonus.type == MULTIPLY) ? colors["blue"] : colors["yellow"];

        draw_list.AddQuadFilled(
            ImVec2(center.x, center.y - radius), ImVec2(center.x + radius, center.y),
            ImVec2(center.x, center.y + radius), ImVec2(center.x - radius, center.y), color
        );
    }
}

void ArkanoidImpl::draw_carriage(ImDrawList& draw_list) {
    float half_width = carriage_width / 2.0f;
    float half_height = carriage_height / 2.0f;
    ImVec2 p_min((carriage_pos.x - half_width) * world_to_screen_scale.x, (carriage_pos.y - half_height) * world_to_screen_scale.y);
    ImVec2 p_max((carriage_pos.x + half_width) * world_to_screen_scale.x, (carriage_pos.y + half_height) * world_to_screen_scale.y);
    draw_list.AddRectFilled(p_min, p_max, colors["blue"]);
}

void ArkanoidImpl::draw_ui(ImGuiIO& io, ImDrawList& draw_list) {
    char score_text[32];
    sprintf(score_text, "Score: %d", score);
    ImVec2 text_size = ImGui::CalcTextSize(score_text);
    float text_x = io.DisplaySize.x - text_size.x - top_ui_text_padding;

    char time_text[32];
    sprintf(time_text, "Time: %.f s", game_time);

    draw_list.AddText(NULL, 0.0f, ImVec2(20.0f, top_ui_text_padding), colors["red"], time_text);
    draw_list.AddText(NULL, 0.0f, ImVec2(text_x, top_ui_text_padding), colors["red"], score_text);
}

void ArkanoidImpl::draw_menus(ImGuiIO& io, ImDrawList& draw_list) {
    if (state != PLAYING)
    {
        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(300, 220));
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

        if (state == PAUSE)
        {
            if (ImGui::Begin("PauseMenu", nullptr, flags))
            {
                ImGui::SetWindowFontScale(1.5f);
                const char* text = "PAUSED";
                float text_width = ImGui::CalcTextSize(text).x;
                ImGui::SetCursorPosX((ImGui::GetWindowSize().x - text_width) * 0.5f);
                ImGui::Text("%s", text);

                ImGui::SetWindowFontScale(1.0f);
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                if (ImGui::Button("CONTINUE", ImVec2(-1.0f, 40.0f))) state = PLAYING;
                ImGui::Spacing();
                if (ImGui::Button("EXIT", ImVec2(-1.0f, 40.0f))) std::exit(0);

                ImGui::End();
            }
        }
        else
        {

            if (ImGui::Begin("GameMenu", nullptr, flags)) {
                ImGui::SetWindowFontScale(1.5f);
                float win_width = ImGui::GetWindowSize().x;

                if (state == VICTORY) {
                    const char* text = "VICTORY!";
                    float text_width = ImGui::CalcTextSize(text).x;
                    ImGui::SetCursorPosX((win_width - text_width) * 0.5f);
                    ImGui::TextColored(colors["green"], "%s", text);
                }
                else if (state == GAME_OVER) {
                    const char* text = "GAME OVER";
                    float text_width = ImGui::CalcTextSize(text).x;
                    ImGui::SetCursorPosX((win_width - text_width) * 0.5f);
                    ImGui::TextColored(colors["red"], "%s", text);
                }

                ImGui::SetWindowFontScale(1.0f);
                ImGui::Spacing();

                char fin_score[64];
                sprintf(fin_score, "Final Score: %d\nTime: %.1f s", score, game_time);
                ImGui::TextWrapped("%s", fin_score);

                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                if (ImGui::Button("RESTART", ImVec2(-1.0f, 40.0f))) reset(saved_settings);
                ImGui::Spacing();
                if (ImGui::Button("EXIT GAME", ImVec2(-1.0f, 40.0f))) std::exit(0);

                ImGui::End();
            }
        }
    }
}

void ArkanoidImpl::reset(const ArkanoidSettings &settings)
{

    world_size = Vect(settings.world_size[0], settings.world_size[1]);

    balls.clear();
    bonuses.clear();
    bricks.clear();

    saved_settings = settings;
    state = PLAYING;
    score = 0;
    game_time = 0.0f;

    bonus_spawn_percentage = settings.bonus_spawn_percentage;
    bonus_initial_speed = settings.bonus_initial_speed;

    ball_initial_speed = settings.ball_speed;
    carriage_height = settings.carriage_height;
    carriage_width = settings.carriage_width;
    carriage_pos = Vect(world_size.x * 0.5f, world_size.y - carriage_offset);
    carriage_speed = settings.carriage_speed;

    Ball main_ball;
    ball_radius = settings.ball_radius;
    main_ball.pos = Vect(carriage_pos.x, carriage_pos.y - settings.ball_radius - 5.0f);
    main_ball.vel = Vect(settings.ball_speed, -settings.ball_speed);
    balls.push_back(main_ball);

    float total_padding_x = (settings.bricks_columns_count + 1) * settings.bricks_columns_padding;
    float brick_width = (world_size.x - total_padding_x) / settings.bricks_columns_count;

    for (int row = 0; row < settings.bricks_rows_count; ++row) {
        for (int col = 0; col < settings.bricks_columns_count; ++col) {
            Brick b;
            b.size = Vect(brick_width, brick_height);
            int random_number = std::rand() % 100;
            if (random_number < settings.bricks_count_percentage) {
                b.pos.x = settings.bricks_columns_padding + col * (brick_width + settings.bricks_columns_padding);
                b.pos.y = top_brick_padding + row * (brick_height + settings.bricks_rows_padding);
                b.destroyed = false;
                b.color = colors[color_names[std::rand() % 4]];
                bricks.push_back(b);
            }
        }
    }

}

void ArkanoidImpl::update(ImGuiIO& io, ArkanoidDebugData& debug_data, float elapsed)
{
    world_to_screen_scale = Vect(io.DisplaySize.x / world_size.x, io.DisplaySize.y / world_size.y);

    if (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE, false))
    {
        if (state == PLAYING) state = PAUSE;
        else if (state == PAUSE) state = PLAYING;
    }

    if (state != PLAYING) return;

    game_time += elapsed;

    update_carriage(io, elapsed);

    bool any_brick_alive = false;
    update_balls(elapsed, debug_data, any_brick_alive);
    balls.erase(std::remove_if(balls.begin(), balls.end(), [](const Ball& b){ return !b.active; }), balls.end());
    if (balls.empty()) {
        state = GAME_OVER;
        return;
    }

    update_bonuses(elapsed);
    bonuses.erase(std::remove_if(bonuses.begin(), bonuses.end(), [](const Bonus& b){ return !b.active; }), bonuses.end());
    if (!any_brick_alive && !bricks.empty()) {
        state = VICTORY;
    }
}

void ArkanoidImpl::draw(ImGuiIO& io, ImDrawList &draw_list)
{
    draw_bricks(draw_list);
    draw_bonuses(draw_list);
    draw_carriage(draw_list);
    draw_balls(draw_list);
    draw_ui(io, draw_list);
    draw_menus(io, draw_list);
}