

// ---------- TEST 1 ----------
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

#include "shaders/shader.h"
#include "DrawText.hpp"

#include "ColorTextureProgram.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

#define WINDOW_HEIGHT 528
#define WINDOW_WIDTH 1280

//This file exists to check that programs that use freetype / harfbuzz link properly in this base code.
//You probably shouldn't be looking here to learn to use either library.
#define FONT_SIZE 36
#define MARGIN (FONT_SIZE * .5)


int main(int argc, char **argv) {

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

	// ----- Harfbuzz and freetype -----
	// 1) Load font with Freetype
	// copied from https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE:: Could not init FreeType Library " << std::endl;
		return -1;
	}

	char* fontFile = argv[1];
	FT_Face face;
	if (FT_New_Face(ft, fontFile, 0, &face))
	{
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		return -1;
	}

	FT_Set_Pixel_Sizes(face, 0, 48);

	char* text = argv[2];

	// Create hb-ft font
	hb_font_t *hb_font;
	hb_font = hb_ft_font_create(face, NULL);
	
	// Create hb_buffer and populate
	hb_buffer_t *hb_buffer;
	hb_buffer = hb_buffer_create();
	hb_buffer_add_utf8(hb_buffer, text, -1, 0, -1 );
	hb_buffer_guess_segment_properties(hb_buffer);
	
	// shape it!
	hb_shape(hb_font, hb_buffer, NULL, 0);

	// get glyph information and positions out of the buffer
	unsigned int len = hb_buffer_get_length(hb_buffer);
	hb_glyph_info_t *info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
	hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);

	// print out the contents as is
	for (size_t i = 0; i < len; i++)
	{
		hb_codepoint_t gid = info[i].codepoint;
		unsigned int cluster = info[i].cluster;
		double x_advance = pos[i].x_advance / 64.;
		double y_advance = pos[i].y_advance / 64.;
		double x_offset = pos[i].x_offset / 64.;
		double y_offset = pos[i].y_offset / 64.;

		char glyphname[32];
		hb_font_get_glyph_name (hb_font, gid, glyphname, sizeof(glyphname));

		printf("glyph='%s' cluster=%d advance=(%g,%g) offset=(%g,%g)\n",
		glyphname, cluster, x_advance, y_advance, x_offset, y_offset);
	}

	// 2) load character with FreeType
	// Font::Character c;
	if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
	{
		std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
		return -1;
	}

	// 3) Create a texture from glyph (should be 'X')
	GLuint texture;
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0, 
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0, 
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// make a dummy white texture 
	// GLuint texture;
	// {
	// 	// ask OpenGL to fill texture with the name of an unused texture object
	// 	glGenTextures(1, &texture);

	// 	// bind that texture object as a GL_TEXTURE_2D-type texture
	// 	glBindTexture(GL_TEXTURE_2D, texture);

	// 	// upload a 1x1 image of a solid white texture
	// 	glm::uvec2 size = glm::uvec2(1,1);
	// 	std::vector< glm::u8vec4 > data(size.x * size.y, glm::u8vec4(0xff, 0xff, 0xff, 0xff));
	// 	glTexImage2D(
	// 		GL_TEXTURE_2D,
	// 		0, 
	// 		GL_RGBA, 
	// 		size.y, 
	// 		size.y,
	// 		0, 
	// 		GL_RGBA, 
	// 		GL_UNSIGNED_BYTE, 
	// 		data.data()
	// 	);

	// 	glBindTexture(GL_TEXTURE_2D, 0);
	// }
	

	GL_ERRORS();
	// ----- END harfbuzz and freetype -----

	// The vertex class was copied from the NEST framework
	// draw functions will work on vectors of vertices, defined as follows:
	struct Vertex {
		Vertex(glm::vec3 const &Position_, glm::u8vec4 const &Color_, glm::vec2 const &TexCoord_) :
			Position(Position_), Color(Color_), TexCoord(TexCoord_) { }
		glm::vec3 Position;
		glm::u8vec4 Color;
		glm::vec2 TexCoord;
	};
	std::vector < Vertex > vertices;

	// inline helper functions for drawing shapes. The triangles are being counter clockwise.
	// draw_rectangle copied from NEST framework
	auto draw_rectangle = [] (std::vector<Vertex> &verts, glm::vec2 const &center, glm::vec2 const &radius, glm::u8vec4 const &color) {
		verts.emplace_back(glm::vec3(center.x-radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.0f, 0.0f));
		verts.emplace_back(glm::vec3(center.x+radius.x, center.y-radius.y, 0.0f), color, glm::vec2(1.0f, 0.0f));
		verts.emplace_back(glm::vec3(center.x+radius.x, center.y+radius.y, 0.0f), color, glm::vec2(1.0f, 1.0f));

		verts.emplace_back(glm::vec3(center.x-radius.x, center.y-radius.y, 0.0f), color, glm::vec2(0.0f, 0.0f));
		verts.emplace_back(glm::vec3(center.x+radius.x, center.y+radius.y, 0.0f), color, glm::vec2(1.0f, 1.0f));
		verts.emplace_back(glm::vec3(center.x-radius.x, center.y+radius.y, 0.0f), color, glm::vec2(0.0f, 1.0f));
	};

	// 4) set up vertex array object and vertex buffer object
	GLuint vertex_buffer = 0;
	GLuint vertex_buffer_for_color_texture_program = 0;
	ColorTextureProgram color_texture_program;
	{ // vertex buffer: [copied from nest framework]
		glGenBuffers(1, &vertex_buffer);
		// for now, buffer will be un-filled.

		GL_ERRORS(); // PARANOIA: print out any OpenGL errors that may have happened
	}
	{ // vertex array mapping buffer for color_texture_program
		// ask OpenGL to fill vertex_buffer_for_color_texture_program with the name of an unsused vertex array object
		glGenVertexArrays(1, &vertex_buffer_for_color_texture_program);

		// set the vertex_buffer_for_color_texture_program as the current vertex array object
		glBindVertexArray(vertex_buffer_for_color_texture_program);

		// set vertex_buffer as the source of glVertexAttribPointer() commands
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		// set up the vertex array object to descrive arrays of Vertex
		glVertexAttribPointer(
			color_texture_program.Position_vec4,
			3,
			GL_FLOAT,
			GL_FALSE,
			sizeof(Vertex),
			(GLbyte *) 0 + 0
		);
		glEnableVertexAttribArray(color_texture_program.Position_vec4);
		// [Note that it is okay to bind a vec3 input to a vec4 attribute -- the w component will be filled with 1 automatically]

		glVertexAttribPointer(
			color_texture_program.Color_vec4, // attribute
			4, //size
			GL_UNSIGNED_BYTE, // type
			GL_TRUE, // normalized
			sizeof(Vertex), // stride
			(GLbyte *)0 + 4*3 // offset
		);
		glEnableVertexAttribArray(color_texture_program.Color_vec4);

		glVertexAttribPointer(
			color_texture_program.TexCoord_vec2, // attribute
			2, // size
			GL_FLOAT, // type
			GL_FALSE,
			sizeof(Vertex),
			(GLbyte *)0 + 4*3 + 4*1 // offset
		);
		glEnableVertexAttribArray(color_texture_program.TexCoord_vec2);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);

		GL_ERRORS();
	}

	GL_ERRORS();
	//------------ main loop ------------
	// copied from NEST framework
	//this inline function will be called whenever the window is resized,
	// and will update the window_size and drawable_size variables:
	glm::uvec2 window_size(WINDOW_WIDTH, WINDOW_WIDTH); //size of window (layout pixels)
	glm::uvec2 drawable_size(WINDOW_WIDTH, WINDOW_HEIGHT); //size of drawable (physical pixels)
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

	GL_ERRORS();


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
				// // ----- copied from game 0 BEGIN -----

				// compute window aspect ratio:
				float aspect = WINDOW_WIDTH / (float)WINDOW_WIDTH;
				// compute scale factor for court given that...
				float scale = std::min(
					(2.0f * aspect) / WINDOW_WIDTH, //... x must fit in [-aspect,aspect] ...
					(2.0f) / WINDOW_HEIGHT //... y must fit in [-1,1].
				);
				glm::vec2 center = 0.5f * glm::vec2(WINDOW_WIDTH, WINDOW_WIDTH);
				glm::mat3x2 clip_to_court(
					glm::vec2(aspect / scale, 0.0f),
					glm::vec2(0.0f, 1.0f / scale),
					glm::vec2(center.x, center.y)
				);
				// build matrix that scales and translates appropriately:
				glm::mat4 court_to_clip = glm::mat4(
					glm::vec4(scale / aspect, 0.0f, 0.0f, 0.0f),
					glm::vec4(0.0f, scale, 0.0f, 0.0f),
					glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
					glm::vec4(-center.x * (scale / aspect), -center.y * scale, 0.0f, 1.0f)
				);

				glm::u8vec4 bg_color(255, 255, 255, 255);

				glClearColor(bg_color.r / 255.0f, bg_color.g / 255.0f, bg_color.g / 255.0f, bg_color.a / 255.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				GL_ERRORS();
				// use alpha blending
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				// dont use the depth test
				glDisable(GL_DEPTH_TEST);

				{ // manipulate the vertex array
					draw_rectangle(vertices, glm::vec2(50, 50), glm::vec2(10, 20), glm::u8vec4(255, 255, 255, 255));
				}

				// upload vertices to the vertex buffer
				// assert(vertices.size() > 0);
				glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer); // set vertex_buffer as current
				glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STREAM_DRAW); // upload vertices array
				glBindBuffer(GL_ARRAY_BUFFER, 0);

				// set color_texture_program as current program:
				glUseProgram(color_texture_program.program);

				// upload OBJECT_TO_CLIP to the proper uniform location:
				glUniformMatrix4fv(color_texture_program.OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(court_to_clip));
				
				// use the mapping vertex_buffer_for_color_texture_program to fetch vertex data:
				glBindVertexArray(vertex_buffer_for_color_texture_program);

				// bind the solid white texture to location zero so things will be just drawn with their colors:
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture);

				// run the OpenGL pipeline
				glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertices.size()));

				// unbind the solid white texture
				glBindTexture(GL_TEXTURE_2D, 0);

				// reset the vertex array to none
				glBindVertexArray(0);
				
				// reset the current program to none:
				glUseProgram(0);
				// // ----- copied from game 0 END -----
			}

			vertices.clear();
		}

		//Wait until the recently-drawn frame is shown before doing it all again:
		SDL_GL_SwapWindow(window);
	}


	//------------  teardown ------------
	SDL_GL_DeleteContext(context);
	context = 0;

	SDL_DestroyWindow(window);
	window = NULL;

	// std::cout << "Hello " << std::endl;
	hb_buffer_t *buf = hb_buffer_create();
	hb_buffer_destroy(buf);
}