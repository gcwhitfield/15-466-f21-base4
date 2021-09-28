// George Whitfield
// September 28, 2021

#pragma once

#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <map>

struct Font
{
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
};

struct DrawText
{
	DrawText() {};
	~DrawText() {};

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
	void draw_text(std::string text, glm::vec2 pos, glm::vec2 bbox_size, 
		glm::vec2 txt_size, Font &font);
};