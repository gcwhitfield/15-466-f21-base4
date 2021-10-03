// MemoryPattern.hpp
// George Whitfield gwhitfie@andrew.cmu.edu
// September 2, 2021

#include <vector>
#include <random>
#include <iostream>

#ifndef DRAW_HELPER
#define DRAW_HELPER
#include "DrawHelper.h"
#endif

// MemoryPattern is a class that stores information about 
// the current pattern being shown to the player. The game will
// create a new MemoryPattern at the beginning of each
// memory sequence, which randomly generates a sequence of 
// UP, DOWN, LEFT, and RIGHT directions. 
//
// ----- drawing -----
// The MemoryGameMode calls MemoryPattern.update every frame. 
// MemoryPattern.update stores information about 
class MemoryPattern {

    public:
        
        static const int NUM_DIRECTIONS = 4;

        enum Direction
        {
            NONE = -1,
            UP = 0,
            DOWN = 1,
            LEFT = 2,
            RIGHT = 3
        };

        std::vector<Direction> pattern;

        // randomly generates a new pattern of size 'len'
        MemoryPattern() { MemoryPattern(10); };
        MemoryPattern(int len);
        ~MemoryPattern() {};

        bool isDoneDrawing() { return _t > _drawDuration; };
        bool isDrawing() { return _t > 0.0f && _t < _drawDuration; };
        
        void begin_drawing();

        void update(float elapsed_time);
        void draw (glm::vec2 const &drawable_size);

        // ----- drawing -----
        std::vector<Vertex> vertices;

    private: 
        // t is the timestep for drawing each of the tiles. This variable is 
        // set by begin_drawing and MemoryPattern.update
        float _t;

        // the time in seconds it takes to draw the entire sequence from 
        // start to finish. This variable is set by the begin_drawing method
        float _drawDuration;

        float _timePerTile = 1.0f; // in seconds

        glm::u8vec4 _top_tile_color = HEX_TO_U8VEC4(0x31afd4ff);
        glm::u8vec4 _left_tile_color = HEX_TO_U8VEC4(0x004fffff);
        glm::u8vec4 _bottom_tile_color = HEX_TO_U8VEC4(0x902d41ff);
        glm::u8vec4 _right_tile_color = HEX_TO_U8VEC4(0xff007fff);
        glm::u8vec4 _deactivated_tile_color = HEX_TO_U8VEC4(0x797979ff);

};