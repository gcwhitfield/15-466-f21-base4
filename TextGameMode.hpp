#include "ColorTextureProgram.hpp"

#include "Mode.hpp"
#include "GL.hpp"

#ifndef GLM
#define GLM
#include <glm/glm.hpp>
#endif

#include <vector>
#include <deque>
#include <map>
#include <string>

#ifndef DRAW_HELPER
#define DRAW_HELPER
#include "DrawHelper.h"
#endif


#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>
/*
 * TextGameMode is a game mode that implements a simple text game.
 */

struct TextGameMode : Mode {
	TextGameMode();
	virtual ~TextGameMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void start();
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----
	glm::vec2 court_radius = glm::vec2(7.0f, 5.0f);
	int difficulty; // the number of tiles to memorize per pattern

	// this variable is used in the PATTERN_RECALL state. It is the index into 
	// the std::vector<Direction> 'pattern.pattern' that corresponds to the 
	// correct direction that the player must press
	int recall_tile_index = 0; 

	enum GameState
	{
		NONE = -1,
		INIT = 0, 
		PATTERN_DELIVERY = 1,
		PATTERN_RECALL = 2,
		FINISH = 3
	};

	GameState curr_state;
	GameState next_state;

	// ----- draw -----
	void draw_init();

	static_assert(sizeof(Vertex) == 4*3 + 1*4 + 4*2, "TextGameMode::Vertex should be packed");

	//vertices will be accumulated into this list and then uploaded+drawn at the end of the 'draw' function:
	std::vector< Vertex > vertices;

	//----- opengl assets / helpers ------

	//Shader program that draws transformed, vertices tinted with vertex colors:
	ColorTextureProgram color_texture_program;

	//Buffer used to hold vertex data during drawing:
	GLuint vertex_buffer = 0;

	//Vertex Array Object that maps buffer locations to color_texture_program attribute locations:
	GLuint vertex_buffer_for_color_texture_program = 0;

	//Solid white texture:
	GLuint white_tex = 0;

	//matrix that maps from clip coordinates to court-space coordinates:
	glm::mat3x2 clip_to_court = glm::mat3x2(1.0f);
	// computed in draw() as the inverse of OBJECT_TO_CLIP
	// (stored here so that the mouse handling code can use it to position the paddle)

	bool _START_CALLED; // set to true once start has been called

	// ----- font rendering -----
	std::string textFontFile = "fonts/quicksilver_3/Quicksilver.ttf";

	// the Character struct and the characters map were inspired by the 
	// OpenGL documentation about text rendering: 
	// https://learnopengl.com/In-Practice/Text-Rendering
	struct Character {
		unsigned int TextureID;  // ID handle of the glyph texture
		glm::ivec2   Size;       // Size of glyph
		glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	};

	std::map<char, Character> characters;

	FT_Face face;
	hb_buffer_t *hb_buffer;
	hb_font_t *hb_font;
	hb_glyph_position_t *pos;
	hb_glyph_info_t *info;

	std::string text_to_display;
	std::string new_text_to_display;

	ColorTextureProgram font_program;
	GLuint font_vertex_attributes;
	GLuint font_vertex_buffer;

};
