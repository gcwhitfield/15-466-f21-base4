

// ---------- TEST 1 ----------
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>
#include <iostream>
#include "GL.hpp"
#include <map>

//This file exists to check that programs that use freetype / harfbuzz link properly in this base code.
//You probably shouldn't be looking here to learn to use either library.
#define FONT_SIZE 36
#define MARGIN (FONT_SIZE * .5)
int main(int argc, char **argv) {
	// FT_Library library;
	// FT_Init_FreeType( &library );

	const char *fontfile;
	const char *text;

	if (argc < 3)
	{
		fprintf (stderr, "usage: hello-harfbuzz font-file.ttf text\n");
		exit (1);
	}

	fontfile = argv[1];
	text = argv[2];

	/* Initialize FreeType and create FreeType font face. */
	FT_Library ft_library;
	FT_Face ft_face;
	FT_Error ft_error;

	if ((ft_error = FT_Init_FreeType (&ft_library)))
		abort();
	if ((ft_error = FT_New_Face (ft_library, fontfile, 0, &ft_face)))
		abort();
	if ((ft_error = FT_Set_Char_Size (ft_face, FONT_SIZE*64, FONT_SIZE*64, 0, 0)))
		abort();

	/* Create hb-ft font. */
	hb_font_t *hb_font;
	hb_font = hb_ft_font_create (ft_face, NULL);

	/* Create hb-buffer and populate. */
	hb_buffer_t *hb_buffer;
	hb_buffer = hb_buffer_create ();
	hb_buffer_add_utf8 (hb_buffer, text, -1, 0, -1);
	hb_buffer_guess_segment_properties (hb_buffer);

	/* Shape it! */
	hb_shape (hb_font, hb_buffer, NULL, 0);

	/* Get glyph information and positions out of the buffer. */
	unsigned int len = hb_buffer_get_length (hb_buffer);
	hb_glyph_info_t *info = hb_buffer_get_glyph_infos (hb_buffer, NULL);
	hb_glyph_position_t *pos = hb_buffer_get_glyph_positions (hb_buffer, NULL);

	/* Print them out as is. */
	printf ("Raw buffer contents:\n");
	for (unsigned int i = 0; i < len; i++)
	{
		hb_codepoint_t gid   = info[i].codepoint;
		unsigned int cluster = info[i].cluster;
		double x_advance = pos[i].x_advance / 64.;
		double y_advance = pos[i].y_advance / 64.;
		double x_offset  = pos[i].x_offset / 64.;
		double y_offset  = pos[i].y_offset / 64.;

		char glyphname[32];
		hb_font_get_glyph_name (hb_font, gid, glyphname, sizeof (glyphname));

		printf ("glyph='%s'	cluster=%d	advance=(%g,%g)	offset=(%g,%g)\n",
				glyphname, cluster, x_advance, y_advance, x_offset, y_offset);
	}

	printf ("Converted to absolute positions:\n");
	/* And converted to absolute positions. */
	{
		double current_x = 0;
		double current_y = 0;
		for (unsigned int i = 0; i < len; i++)
		{
		hb_codepoint_t gid   = info[i].codepoint;
		unsigned int cluster = info[i].cluster;
		double x_position = current_x + pos[i].x_offset / 64.;
		double y_position = current_y + pos[i].y_offset / 64.;


		char glyphname[32];
		hb_font_get_glyph_name (hb_font, gid, glyphname, sizeof (glyphname));

		printf ("glyph='%s'	cluster=%d	position=(%g,%g)\n",
			glyphname, cluster, x_position, y_position);

		current_x += pos[i].x_advance / 64.;
		current_y += pos[i].y_advance / 64.;
		}
	}




	// example code copied from freetype documentation
	FT_GlyphSlot  slot = ft_face->glyph;  /* a small shortcut */
	// FT_UInt       glyph_index;
	int           pen_x, pen_y, n;
	size_t num_chars = strlen(text);

	pen_x = 300;
	pen_y = 200;

    
    // Character struct copied from Learn OpenGL tutorial on font rendering:
    // https://learnopengl.com/In-Practice/Text-Rendering
    struct Character {
        unsigned int TextureID; // ID handle the glyph texture
        glm::ivec2 Size; // Size of glyph
        glm::ivec2 Bearing; // Offset from baseline to left/top of the glyph
        unsigned int Advance; // Horizontal offest to advance to next glyph
    };
    std::map<char, Character> characters;

	for ( n = 0; n < num_chars; n++ )
	{
        /* load glyph image into the slot (erase previous one) */
        FT_Error error = FT_Load_Char( ft_face, text[n], FT_LOAD_RENDER );
        if ( error )
            continue;  /* ignore errors */

        /* now, draw to our target surface */
        /*
        my_draw_bitmap( &slot->bitmap,
                        pen_x + slot->bitmap_left,
                        pen_y - slot->bitmap_top );
        */
        /* increment pen position */

        // 'generate texutre', 'set texutre options', and 'store char for later use'
        // sections were copied from Learn OpenGL tutorial on font rendering:
        // https://learnopengl.com/In-Practice/Text-Rendering
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            ft_face->glyph->bitmap.width,
            ft_face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            ft_face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture, 
            glm::ivec2(ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows),
            glm::ivec2(ft_face->glyph->bitmap_left, ft_face->glyph->bitmap_top),
            static_cast<unsigned int>(ft_face->glyph->advance.x)
        };
        characters.insert(std::pair<char, Character>(c, character));

        pen_x += slot->advance.x >> 6;

	}


	std::cout << "Hello " << std::endl;
	hb_buffer_t *buf = hb_buffer_create();
	hb_buffer_destroy(buf);
}


// ---------- TEST 2 ----------
// code copied from OpenGL documentatoni about freetype and font rendering
// https://learnopengl.com/code_viewer_gh.php?code=src/7.in_practice/2.text_rendering/text_rendering.cpp

// #include <iostream>
// #include <map>
// #include <string>

// //#include <glad/glad.h>
// // #include <GLFW/glfw3.h>

// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/type_ptr.hpp>

// #include <ft2build.h>
// #include FR_FREETYPE_H

// //#include <learnopengl/shader.h>

// void framebuffer_size_callback(GLFWwindow* window, int width, int heigtht);
// void processInput(GLFWwindow *window);
// void RenderText Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color);

// // settings
// const unsigned int SCR_WIDTH = 800;
// const unsigned int SCR_HEIGHT = 600;
 
//  // Holds all state informatino relevant to a character as loaded using FreeType
//  struct Character {
// 	 unsigned int TextureID; // ID handle the glyph texture
// 	 glm:::ivec2 Size; // Size of glyph
// 	 glm::ivec2 Bearing; // Offset from baseline to left/top of the glyph
// 	 unsigned int Advance; // Horizontal offest to advance to next glyph
//  };


// int main(int argc, char **argv)
// {

// }