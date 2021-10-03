// George Whitfield
// gwhitfie@andrew.cmu.edu
// September 2, 2021

#include "MemoryPattern.hpp"

MemoryPattern::MemoryPattern(int len)
{
    static std::mt19937 rand; 
    pattern = std::vector<Direction>(len);
    for (int i = 0; i < len; i++)
    {
        // the following line was inspired from enum tutorial on static casting
        // https://riptutorial.com/cplusplus/example/18751/enum-conversions
        pattern[i] = static_cast<Direction>(rand() % NUM_DIRECTIONS);
    }
    _t = -0.01f;
    _drawDuration = pattern.size() * _timePerTile;
}

void MemoryPattern::update(float elapsed_time)
{
    if (!isDoneDrawing() && isDrawing())
        _t += elapsed_time;
}

// This function is called from inside of MemoryGameMode.draw(). To draw the
// memory sequence, you must first call begin_drawing(). If you do not call 
// begin_drawing() then this function will draw the 4 tiles deactivated.
// After the sequence has finished drawing, the tiles will be drawn 
// in deactivated once again
void MemoryPattern::draw(glm::vec2 const &drawable_size)
{
    vertices.clear();
    // 1) calculate which tile is supposed to be activated
    int tile = (unsigned)std::floor(_t / _timePerTile);

    Direction dir = isDoneDrawing() ? static_cast<Direction>(-1) : pattern[tile];

    // 2) draw the tiles
    glm::u8vec4 top_col    = dir == UP ? _top_tile_color : _deactivated_tile_color;
    glm::u8vec4 left_col   = dir == LEFT ? _left_tile_color : _deactivated_tile_color;
    glm::u8vec4 right_col  = dir == RIGHT ?_right_tile_color : _deactivated_tile_color;
    glm::u8vec4 bottom_col = dir == DOWN ? _bottom_tile_color : _deactivated_tile_color;

    glm::vec2 top_tile_location = glm::vec2(0, drawable_size.y / 4.0f);
    glm::vec2 bottom_tile_location = glm::vec2(0, -drawable_size.y / 4.0f);
    glm::vec2 left_tile_location = glm::vec2(-drawable_size.x / 4.0f, 0);
    glm::vec2 right_tile_location = glm::vec2(drawable_size.x / 4.0f, 0);

    draw_rectangle(vertices, left_tile_location, glm::vec2(0.3f, 0.4f), left_col);
    draw_rectangle(vertices, right_tile_location, glm::vec2(0.3f, 0.4f), right_col);
    draw_rectangle(vertices, top_tile_location, glm::vec2(0.3f, 0.4f), top_col);
    draw_rectangle(vertices, bottom_tile_location, glm::vec2(0.3f, 0.4f), bottom_col);
}

void MemoryPattern::begin_drawing()
{
    _t = 0.01f;
}
