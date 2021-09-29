

// ---------- TEST 1 ----------
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>
#include <iostream>

#include "GL.hpp"

//for screenshots:
#include "load_save_png.hpp"

#include <map>

#include <SDL.h>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <algorithm>


//This file exists to check that programs that use freetype / harfbuzz link properly in this base code.
//You probably shouldn't be looking here to learn to use either library.
#define FONT_SIZE 36
#define MARGIN (FONT_SIZE * .5)

int main(int argc, char **argv) {
	// FT_Library library;
	// FT_Init_FreeType( &library );

	
	// const char *fontfile;
	// const char *text;

	// if (argc < 3)
	// {
	// 	fprintf (stderr, "usage: hello-harfbuzz font-file.ttf text\n");
	// 	exit (1);
	// }

	// fontfile = argv[1];
	// text = argv[2];

	// /* Initialize FreeType and create FreeType font face. */
	// FT_Library ft_library;
	// FT_Face ft_face;
	// FT_Error ft_error;

	// if ((ft_error = FT_Init_FreeType (&ft_library)))
	// 	abort();
	// if ((ft_error = FT_New_Face (ft_library, fontfile, 0, &ft_face)))
	// 	abort();
	// if ((ft_error = FT_Set_Char_Size (ft_face, FONT_SIZE*64, FONT_SIZE*64, 0, 0)))
	// 	abort();

	// /* Create hb-ft font. */
	// hb_font_t *hb_font;
	// hb_font = hb_ft_font_create (ft_face, NULL);

	// /* Create hb-buffer and populate. */
	// hb_buffer_t *hb_buffer;
	// hb_buffer = hb_buffer_create ();
	// hb_buffer_add_utf8 (hb_buffer, text, -1, 0, -1);
	// hb_buffer_guess_segment_properties (hb_buffer);

	// /* Shape it! */
	// hb_shape (hb_font, hb_buffer, NULL, 0);

	// /* Get glyph information and positions out of the buffer. */
	// unsigned int len = hb_buffer_get_length (hb_buffer);
	// hb_glyph_info_t *info = hb_buffer_get_glyph_infos (hb_buffer, NULL);
	// hb_glyph_position_t *pos = hb_buffer_get_glyph_positions (hb_buffer, NULL);

	// /* Print them out as is. */
	// printf ("Raw buffer contents:\n");
	// for (unsigned int i = 0; i < len; i++)
	// {
	// 	hb_codepoint_t gid   = info[i].codepoint;
	// 	unsigned int cluster = info[i].cluster;
	// 	double x_advance = pos[i].x_advance / 64.;
	// 	double y_advance = pos[i].y_advance / 64.;
	// 	double x_offset  = pos[i].x_offset / 64.;
	// 	double y_offset  = pos[i].y_offset / 64.;

	// 	char glyphname[32];
	// 	hb_font_get_glyph_name (hb_font, gid, glyphname, sizeof (glyphname));

	// 	printf ("glyph='%s'	cluster=%d	advance=(%g,%g)	offset=(%g,%g)\n",
	// 			glyphname, cluster, x_advance, y_advance, x_offset, y_offset);
	// }

	// printf ("Converted to absolute positions:\n");
	// /* And converted to absolute positions. */
	// {
	// 	double current_x = 0;
	// 	double current_y = 0;
	// 	for (unsigned int i = 0; i < len; i++)
	// 	{
	// 	hb_codepoint_t gid   = info[i].codepoint;
	// 	unsigned int cluster = info[i].cluster;
	// 	double x_position = current_x + pos[i].x_offset / 64.;
	// 	double y_position = current_y + pos[i].y_offset / 64.;


	// 	char glyphname[32];
	// 	hb_font_get_glyph_name (hb_font, gid, glyphname, sizeof (glyphname));

	// 	printf ("glyph='%s'	cluster=%d	position=(%g,%g)\n",
	// 		glyphname, cluster, x_position, y_position);

	// 	current_x += pos[i].x_advance / 64.;
	// 	current_y += pos[i].y_advance / 64.;
	// 	}
	// }




	// // example code copied from freetype documentation
	// FT_GlyphSlot  slot = ft_face->glyph;  /* a small shortcut */
	// // FT_UInt       glyph_index;
	// int           pen_x, pen_y, n;
	// size_t num_chars = strlen(text);

	// pen_x = 300;
	// pen_y = 200;

    
    // // Character struct copied from Learn OpenGL tutorial on font rendering:
    // // https://learnopengl.com/In-Practice/Text-Rendering
    // struct Character {
    //     unsigned int TextureID; // ID handle the glyph texture
    //     glm::ivec2 Size; // Size of glyph
    //     glm::ivec2 Bearing; // Offset from baseline to left/top of the glyph
    //     unsigned int Advance; // Horizontal offest to advance to next glyph
    // };
    // std::map<char, Character> characters;

	
	// ----- create a opengl window (copied from nest framework) -----
	//Initialize SDL library:
	SDL_Init(SDL_INIT_VIDEO);

	//Ask for an OpenGL context version 3.3, core profile, enable debug:
	SDL_GL_ResetAttributes();
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	//create window:
	SDL_Window *window = SDL_CreateWindow(
		"gp21 game4: choice-based game", //TODO: remember to set a title for your game!
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		1280, 720, //TODO: modify window size if you'd like
		SDL_WINDOW_OPENGL
		| SDL_WINDOW_RESIZABLE //uncomment to allow resizing
		| SDL_WINDOW_ALLOW_HIGHDPI //uncomment for full resolution on high-DPI screens
	);

	//prevent exceedingly tiny windows when resizing:
	SDL_SetWindowMinimumSize(window,100,100);

	if (!window) {
		std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
		return 1;
	}

	//Create OpenGL context:
	SDL_GLContext context = SDL_GL_CreateContext(window);

	if (!context) {
		SDL_DestroyWindow(window);
		std::cerr << "Error creating OpenGL context: " << SDL_GetError() << std::endl;
		return 1;
	}

	//On windows, load OpenGL entrypoints: (does nothing on other platforms)
	init_GL();

	//Set VSYNC + Late Swap (prevents crazy FPS):
	if (SDL_GL_SetSwapInterval(-1) != 0) {
		std::cerr << "NOTE: couldn't set vsync + late swap tearing (" << SDL_GetError() << ")." << std::endl;
		if (SDL_GL_SetSwapInterval(1) != 0) {
			std::cerr << "NOTE: couldn't set vsync (" << SDL_GetError() << ")." << std::endl;
		}
	}
	

	//------------ main loop ------------

	//this inline function will be called whenever the window is resized,
	// and will update the window_size and drawable_size variables:
	glm::uvec2 window_size; //size of window (layout pixels)
	glm::uvec2 drawable_size; //size of drawable (physical pixels)
	//On non-highDPI displays, window_size will always equal drawable_size.
	auto on_resize = [&](){
		int w,h;
		SDL_GetWindowSize(window, &w, &h);
		window_size = glm::uvec2(w, h);
		SDL_GL_GetDrawableSize(window, &w, &h);
		drawable_size = glm::uvec2(w, h);
		glViewport(0, 0, drawable_size.x, drawable_size.y);
	};
	on_resize();

	//This will loop until the current mode is set to null:
	bool activated = true;
	while (activated) {
		//every pass through the game loop creates one frame of output
		//  by performing three steps:

		{ //(1) process any events that are pending
			static SDL_Event evt;
			while (SDL_PollEvent(&evt) == 1) {
				//handle resizing:
				if (evt.type == SDL_WINDOWEVENT && evt.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
					on_resize();
				}
				//handle input:
				{
				
			
				} 
				if (evt.type == SDL_QUIT) {
					activated = false;
					break;
				} else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_PRINTSCREEN) {
					// --- screenshot key ---
					// std::string filename = "screenshot.png";
					// std::cout << "Saving screenshot to '" << filename << "'." << std::endl;
					// glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
					// glReadBuffer(GL_FRONT);
					// int w,h;
					// SDL_GL_GetDrawableSize(window, &w, &h);
					// std::vector< glm::u8vec4 > data(w*h);
					// glReadPixels(0,0,w,h, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
					// for (auto &px : data) {
					// 	px.a = 0xff;
					// }
					// save_png(filename, glm::uvec2(w,h), data.data(), LowerLeftOrigin);
				}
			}
			if (!activated) break;
		}

		{ //(2) call the current mode's "update" function to deal with elapsed time:
			auto current_time = std::chrono::high_resolution_clock::now();
			static auto previous_time = current_time;
			float elapsed = std::chrono::duration< float >(current_time - previous_time).count();
			previous_time = current_time;

			//if frames are taking a very long time to process,
			//lag to avoid spiral of death:
			elapsed = std::min(0.1f, elapsed);

			// update
			{

			}
			if (!activated) break;
		}

		{ //(3) call the current mode's "draw" function to produce output:
		
			// draw
			{
	

			}
		}

		//Wait until the recently-drawn frame is shown before doing it all again:
		SDL_GL_SwapWindow(window);
	}


	//------------  teardown ------------
	SDL_GL_DeleteContext(context);
	context = 0;

	SDL_DestroyWindow(window);
	window = NULL;

	// for ( n = 0; n < num_chars; n++ )
	// {
    //     /* load glyph image into the slot (erase previous one) */
    //     FT_Error error = FT_Load_Char( ft_face, text[n], FT_LOAD_RENDER );
    //     if ( error )
    //         continue;  /* ignore errors */

    //     /* now, draw to our target surface */
    //     /*
    //     my_draw_bitmap( &slot->bitmap,
    //                     pen_x + slot->bitmap_left,
    //                     pen_y - slot->bitmap_top );
    //     */
    //     /* increment pen position */

    //     // // 'generate texutre', 'set texutre options', and 'store char for later use'
    //     // // sections were copied from Learn OpenGL tutorial on font rendering:
    //     // // https://learnopengl.com/In-Practice/Text-Rendering
    //     // // generate texture
    //     unsigned int texture;
    //     glGenTextures(1, &texture);
	// 	std::cout << ft_face << std::endl;
    //     glBindTexture(GL_TEXTURE_2D, texture);
    //     glTexImage2D(
    //         GL_TEXTURE_2D,
    //         0,
    //         GL_RED,
    //         ft_face->glyph->bitmap.width,
    //         ft_face->glyph->bitmap.rows,
    //         0,
    //         GL_RED,
    //         GL_UNSIGNED_BYTE,
    //         ft_face->glyph->bitmap.buffer
    //     );
    //     // // set texture options
    //     // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //     // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //     // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //     // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //     // // now store character for later use
    //     // // Character character = {
    //     // //     texture, 
    //     // //     glm::ivec2(ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows),
    //     // //     glm::ivec2(ft_face->glyph->bitmap_left, ft_face->glyph->bitmap_top),
    //     // //     static_cast<unsigned int>(ft_face->glyph->advance.x)
    //     // // };
    //     // // characters.insert(std::pair<char, Character>(c, character));

    //     pen_x += slot->advance.x >> 6;

	// }


	// std::cout << "Hello " << std::endl;
	// hb_buffer_t *buf = hb_buffer_create();
	// hb_buffer_destroy(buf);
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