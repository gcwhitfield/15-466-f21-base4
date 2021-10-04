// George Whitfield
// September 28, 2021

#pragma once

#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <iostream>

#ifndef DRAW_HELPER
#define DRAW_HELPER
#include "DrawHelper.h"
#endif


//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#ifndef GLM
#define GLM
#include <glm/glm.hpp>
#endif

#include "GL.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

class Font
{
	public:
		Font()  {};
		~Font() {};
		void initialize(std::string fontFileName);

		// the Character struct and the characters map were copied from the 
		// OpenGL documentation about text rendering: 
		// https://learnopengl.com/In-Practice/Text-Rendering
		struct Character {
			unsigned int TextureID;  // ID handle of the glyph texture
			glm::ivec2   Size;       // Size of glyph
			glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
			unsigned int Advance;    // Offset to advance to next glyph
		};

		std::map<char, Character> characters;

		FT_Face face;
	
};

class DrawText
{
	public:
		DrawText() {};
		// DrawText(Font &font_) {font = font_;};
		DrawText(
			std::string fontFileName, 
			GLuint program,
			GLuint vertex_attribtues,
			GLuint vertex_buffer
		);
		~DrawText() {};

		GLuint program;
		GLuint vertex_attributes;
		GLuint vertex_buffer;
		
		Font font;
		FT_Face face;

		GLuint tex;

		// the Character struct and the characters map were copied from the 
		// OpenGL documentation about text rendering: 
		// https://learnopengl.com/In-Practice/Text-Rendering
		struct Character {
			unsigned int TextureID;  // ID handle of the glyph texture
			glm::ivec2   Size;       // Size of glyph
			glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
			unsigned int Advance;    // Offset to advance to next glyph
		};

		std::map<char, Character> characters;

		std::string text_to_display;
		/*
			text:       the text tp display
			pos:        the position of the bottom left corner of the first character to 
						display
			bbox_size:  the size of the text box. If the text goes over the bounds 
						of the text box then the text will get wrapped to the next 
						line
			txt_size:   the width and height of each glyph. If the width is 0 then 
						the width will be automatically calcualted by FreeType based
						on the height
			font:		A reference to a font to lookup the glyphs from
		*/
		void draw_text(std::string text, glm::ivec2 pos, glm::vec2 bbox_size, 
			glm::u8vec4 const &color, glm::vec2 txt_size);

	private:
		hb_buffer_t *hb_buffer;
		hb_font_t *hb_font;
		hb_glyph_position_t *pos;
		hb_glyph_info_t *info;
		std::vector<Vertex> vertices;

		//void updateTextData();
};