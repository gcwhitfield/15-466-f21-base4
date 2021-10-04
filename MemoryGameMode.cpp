#include "MemoryGameMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>
// The MemoryGameMode operates like a Finite State Machine (FSM). The game has 
// different states and the MemoryGameMode decides what to do based on curr_state.
// The states are:
//     INIT - displays the blue plus sign
//     PATTERN_DELIVERY - shows the player a pattern to memorize
//     PATTERN_RECALL - shows nothing, the player must correctly recall pattern
//     FINISH - final state for 'you win' screen. At the moment, the game never
//            enters into this state and instead the pattern gets harder forever
//
// Notice how the handle_input, update, and draw functions all case on curr_state.
// If the player presses the spacebar in the INIT state, then the game will 
// transition to the first PATTERN_DELIVERY state with difficulty = 1. After 
// the pattern is shown in the PATTERN_DELIVERY state, the game automatically
// transitions into PATTERN_RECALL. From PATTERN_RECALL, the game will 
// always transition back into PATTERN_DELIVERY, with the caveat that if the player
// makes a mistake in the recall then the difficulty gets reset back to 1.
MemoryGameMode::MemoryGameMode() {
	
	{ // set up the game mode
		_START_CALLED = false;
	}

	// ----- set up game state -----
	difficulty = 1;
	pattern =  MemoryPattern(difficulty);
	curr_state = INIT;
	next_state = INIT;
	
	// ----- allocate OpenGL resources -----
	{ // vertex buffer:
		glGenBuffers(1, &vertex_buffer);
		// for now, buffer will be un-filled.

		GL_ERRORS(); // PARANOIA: print out any OpenGL errors that may have happened
	}

	{ // vertex array mapping buffer for color_texture_program:
		// ask OpenGL to fill vertex_buffer_for_color_texture_program with the name of an unused vertex array object:
		glGenVertexArrays(1, &vertex_buffer_for_color_texture_program);

		// set vertex_buffer_for_color_texture_program as the current vertex array object:
		glBindVertexArray(vertex_buffer_for_color_texture_program);

		// set vertex_buffer as the source of glVertexAttribPointer() commands:
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		// set up the vertex array object to describe arrays of MemoryGameMode::Vertex:
		glVertexAttribPointer(
			color_texture_program.Position_vec4, // attribute
			3, // size
			GL_FLOAT, // type
			GL_FALSE, // normalized
			sizeof(Vertex), // stride
			(GLbyte *)0 + 0 // offset
		);
		glEnableVertexAttribArray(color_texture_program.Position_vec4);
		// [Note that it is okay to bind a vec3 input to a vec4 attribute -- the w component will be filled with 1.0 automatically]

		glVertexAttribPointer(
			color_texture_program.Color_vec4, // attribute
			4, // size
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
			GL_FALSE, // normalized
			sizeof(Vertex), // stride
			(GLbyte *)0 + 4*3 + 4*1 // offset
		);
		glEnableVertexAttribArray(color_texture_program.TexCoord_vec2);

		// done referring to vertex_buffer, so unbind it:
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// done setting up vertex array object, so unbind it:
		glBindVertexArray(0);

		GL_ERRORS(); // PARANOIA: print out any OpenGL errors that may have happened
	}

	{ // make a texture from FreeType

		// 1) Load font with Freetype
		// copied from https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
		FT_Library ft;
		if (FT_Init_FreeType(&ft))
		{
			std::cout << "ERROR::FREETYPE:: Could not init FreeType Library " << std::endl;
			exit(0);
		}

		const char* fontFile = &"fonts/quicksilver_3/Quicksilver.ttf"[0];
		FT_Face face;
		if (FT_New_Face(ft, fontFile, 0, &face))
		{
			std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
			exit(0);
		}

		// disable alignment since what we read from the face (font) is grey-scale. 
		// this line was copied from https://github.com/ChunanGang/TextBasedGame/blob/main/TextRenderer.cpp
    	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

		FT_Set_Pixel_Sizes(face, 0, 48);

		const char* text = &"Hello!"[0];

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
			exit(0);
		}

		// 3) Create a texture from glyph (should be 'X')
		// GLuint white_tex;
		
			glGenTextures(1, &white_tex);
			glBindTexture(GL_TEXTURE_2D, white_tex);
			glm::uvec2 size = glm::uvec2(face->glyph->bitmap.rows,face->glyph->bitmap.width);
			std::vector< glm::u8vec4 > data(size.x*size.y, glm::u8vec4(0xff, 0xff, 0xff, 0xff));
			for (size_t i = 0; i < size.y; i++)
			{
				for (size_t j = 0; j < size.x; j++)
				{
					size_t index = i * size.y + j;
					uint8_t val = face->glyph->bitmap.buffer[j * std::abs(face->glyph->bitmap.pitch) + i]; // copied from professor mccan's example code for printing bitmap buffer
					(void) val;
				
					data[index].x = val;
					data[index].y = val;
					data[index].z = val;
					data[index].w = val;
				}
			}
			glTexImage2D(
				GL_TEXTURE_2D,
				0, 
				GL_RGBA,
				size.x,
				size.y,
				0, 
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				//face->glyph->bitmap.buffer
				data.data()
			);

		// 	{
		// 		FT_Bitmap const &bitmap = face->glyph->bitmap;

		// 		std::cout << "Bitmap (" << bitmap.width << "x" << bitmap.rows << "):\n";
		// 		std::cout << "  pitch is " << bitmap.pitch << "\n";
		// 		std::cout << "  pixel_mode is " << int32_t(bitmap.pixel_mode) << "; num_grays is " << bitmap.num_grays << "\n";
		// 		if (bitmap.pixel_mode == FT_PIXEL_MODE_GRAY && bitmap.num_grays == 256 && bitmap.pitch >= 0) {
		// 			for (uint32_t row = 0; row < bitmap.rows; ++row) {
		// 				std::cout << "   ";
		// 				for (uint32_t col = 0; col < bitmap.width; ++col) {
		// 					uint8_t val = bitmap.buffer[row * std::abs(bitmap.pitch) + col];
		// 					if (val < 128) std::cout << '.';
		// 					else std::cout << '#';
		// 				}
		// 				std::cout << '\n';
		// 			}
		// 		} else {
		// 			std::cout << "  (bitmap is not FT_PIXEL_MODE_GRAY with 256 levels and upper-left origin, not dumping)" << "\n";
		// 		}
		// 		std::cout.flush();
		// 	}

		// 	// glBindTexture(GL_TEXTURE_2D, 0);
		

		// // ask OpenGL to fill white_tex with the name of an unused texture object:
		// glGenTextures(1, &white_tex);

		// // // bind that texture object as a GL_TEXTURE_2D-type texture:
		// glBindTexture(GL_TEXTURE_2D, white_tex);

		// // upload a 1x1 image of solid white to the texture:
		// glm::uvec2 size = glm::uvec2(face->glyph->bitmap.rows, face->glyph->bitmap.width);
		// // glm::u8vec4 start_color(0x00, 0x00, 0x00, 0x00);
		// // glm::u8vec4 end_color(0xff, 0xff, 0xff, 0xff);
		// std::vector< glm::u8vec4 > data(size.x*size.y, glm::u8vec4(0xff, 0xff, 0xff, 0xff));
		// for (int i = 0; i < size.y; i++)
		// {
		// 	for (int j = 0; j < size.x; j++)
		// 	{
		// 		int index = i * size.y + j;
		// 		// float t =  sqrt(pow((i / (float)size.y), 2) + pow((j / (float)size.x), 2)) / sqrt(2);
		// 		// data[index].x = start_color.x * t + end_color.x * (t - (float)1); 
		// 		// data[index].y = start_color.y * t + end_color.y * (t - (float)1); 
		// 		// data[index].z = start_color.z * t + end_color.z * (t - (float)1); 
		// 		// data[index].w = start_color.w * t + end_color.w * (t - (float)1); 
		// 		uint8_t val = face->glyph->bitmap.buffer[j * std::abs(face->glyph->bitmap.pitch) + i]; // copied from professor mccan's example code for printing bitmap buffer
		// 		data[index].x =val; 
		// 		data[index].y =val; 
		// 		data[index].z =val; 
		// 		data[index].w =val; 
		// 	}
		// }
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

		// set filtering and wrapping parameters:
		// (it's a bit silly to mipmap a 1x1 texture, but I'm doing it because you may want to use this code to load different sizes of texture)
		// parameters copied form https://github.com/ChunanGang/TextBasedGame/blob/main/TextRenderer.cpp
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// since texture uses a mipmap and we haven't uploaded one, instruct opengl to make one for us:
		glGenerateMipmap(GL_TEXTURE_2D);

		// Okay, texture uploaded, can unbind it:
		glBindTexture(GL_TEXTURE_2D, 0);

		GL_ERRORS(); // PARANOIA: print out any OpenGL errors that may have happened
	}
}

MemoryGameMode::~MemoryGameMode() {

	// ----- free OpenGL resources -----
	glDeleteBuffers(1, &vertex_buffer);
	vertex_buffer = 0;

	glDeleteVertexArrays(1, &vertex_buffer_for_color_texture_program);
	vertex_buffer_for_color_texture_program = 0;

	glDeleteTextures(1, &white_tex);
	white_tex = 0;

}

bool MemoryGameMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	// handle different types of input for each of the different states
	switch (curr_state)
	{
		case INIT:
			if (evt.key.keysym.sym == SDLK_SPACE)
			{
				pattern.begin_drawing();
				next_state = PATTERN_DELIVERY;
			}
			break;
		case PATTERN_DELIVERY:
			break;
		case PATTERN_RECALL:
			{
				auto check_input = [this](MemoryPattern::Direction given_dir)
				{
					MemoryPattern::Direction correct_dir = pattern.pattern[recall_tile_index];
					if (correct_dir != given_dir)
					{
						difficulty = 1;
						pattern = MemoryPattern(difficulty);
						pattern.begin_drawing();
						next_state = PATTERN_DELIVERY;
					} else {
						if (recall_tile_index >= pattern.pattern.size() - 1)
						{ // correct input, transition to next stage 
							next_state = PATTERN_DELIVERY;
							difficulty ++;
							pattern = MemoryPattern(difficulty);
							pattern.begin_drawing();
						} else {
							recall_tile_index ++;
						}
					}
					
					
					return;
				};
				if (evt.key.type == SDL_KEYDOWN)
				{
					if (evt.key.keysym.sym == SDLK_UP || evt.key.keysym.sym == SDLK_w)
					{
						check_input(MemoryPattern::UP);
					} else if (evt.key.keysym.sym == SDLK_DOWN || evt.key.keysym.sym == SDLK_s)
					{
						check_input(MemoryPattern::DOWN);
					} else if (evt.key.keysym.sym == SDLK_LEFT || evt.key.keysym.sym == SDLK_a)
					{
						check_input(MemoryPattern::LEFT);
					} else if (evt.key.keysym.sym == SDLK_RIGHT || evt.key.keysym.sym == SDLK_d)
					{
						check_input(MemoryPattern::RIGHT);
					}
				}
			}
			break;
		case FINISH:
			break;
		case NONE:
			break;
	}

	return false;
}

// start is called right before the first call to update is executed
void MemoryGameMode::start()
{
	curr_state = INIT;
}

void MemoryGameMode::update(float elapsed) {

	if (!_START_CALLED)
	{
		start();
		_START_CALLED = true;
	}

	// ----- next state logic -----
	switch (curr_state)
	{
		case INIT:
			// INIT will transition into PATTERN_DELIVERY when the spacebar is 
			// pressed
			break;
		case PATTERN_DELIVERY:
			if (pattern.isDoneDrawing())
			{
				next_state = PATTERN_RECALL;
				recall_tile_index = 0;
			} else {
				pattern.update(elapsed);
			}
			break;
		case PATTERN_RECALL: // next state logic defined in handle_input	
			break;
		case FINISH:
			break;
		case NONE:
			break;
	}

	curr_state = next_state;
}

void MemoryGameMode::draw(glm::uvec2 const &drawable_size) {

	// verices must be cleared every time the screen is drawn. Otherwise, the 
	// the previous frame will be drawn below the curretn frame, which causes 
	// smeary artifacts
	vertices.clear();

	// other useful drawing constants:
	const float wall_radius = 0.05f;
	const float padding = 0.14f; //padding between outside of walls and edge of window

	// ----- draw items based on the current game state -----
	switch (curr_state)
	{
		case INIT:
			draw_init();
			break;
		case PATTERN_RECALL:
			draw_pattern_recall();
			break;
		case PATTERN_DELIVERY:
			draw_pattern_delivery();
			break;
		case FINISH:
			draw_finish();
			break;
		case NONE:
			std::cerr << "Game has entered NONE state, exiting... " << std::endl;
			exit(0);
			break;
	}

	// walls:
	draw_rectangle(vertices, glm::vec2(-court_radius.x-wall_radius, 0.0f), glm::vec2(wall_radius, court_radius.y + 2.0f * wall_radius), fg_color);
	draw_rectangle(vertices, glm::vec2( court_radius.x+wall_radius, 0.0f), glm::vec2(wall_radius, court_radius.y + 2.0f * wall_radius), fg_color);
	draw_rectangle(vertices, glm::vec2( 0.0f,-court_radius.y-wall_radius), glm::vec2(court_radius.x, wall_radius), fg_color);
	draw_rectangle(vertices, glm::vec2( 0.0f, court_radius.y+wall_radius), glm::vec2(court_radius.x, wall_radius), fg_color);

	// scores:
	glm::vec2 score_radius = glm::vec2(0.1f, 0.1f);

	// ------ compute court-to-window transform ------

	// compute area that should be visible:
	glm::vec2 scene_min = glm::vec2(
		-court_radius.x - 2.0f * wall_radius - padding,
		-court_radius.y - 2.0f * wall_radius - padding
	);
	glm::vec2 scene_max = glm::vec2(
		court_radius.x + 2.0f * wall_radius + padding,
		court_radius.y + 2.0f * wall_radius + 3.0f * score_radius.y + padding
	);

	// compute window aspect ratio:
	float aspect = drawable_size.x / float(drawable_size.y);
	// we'll scale the x coordinate by 1.0 / aspect to make sure things stay square.

	// compute scale factor for court given that...
	float scale = std::min(
		(2.0f * aspect) / (scene_max.x - scene_min.x), //... x must fit in [-aspect,aspect] ...
		(2.0f) / (scene_max.y - scene_min.y) //... y must fit in [-1,1].
	);

	glm::vec2 center = 0.5f * (scene_max + scene_min);

	// build matrix that scales and translates appropriately:
	glm::mat4 court_to_clip = glm::mat4(
		glm::vec4(scale / aspect, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, scale, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(-center.x * (scale / aspect), -center.y * scale, 0.0f, 1.0f)
	);
	// NOTE: glm matrices are specified in *Column-Major* order,
	// so each line above is specifying a *column* of the matrix(!)

	// also build the matrix that takes clip coordinates to court coordinates (used for mouse handling):
	clip_to_court = glm::mat3x2(
		glm::vec2(aspect / scale, 0.0f),
		glm::vec2(0.0f, 1.0f / scale),
		glm::vec2(center.x, center.y)
	);

	// ---- actual drawing ----
	// clear the color buffer:
	glClearColor(bg_color.r / 255.0f, bg_color.g / 255.0f, bg_color.b / 255.0f, bg_color.a / 255.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// use alpha blending:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// don't use the depth test:
	glDisable(GL_DEPTH_TEST);

	// upload vertices to vertex_buffer:
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer); // set vertex_buffer as current
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_DYNAMIC_DRAW); // upload vertices array
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// set color_texture_program as current program:
	glUseProgram(color_texture_program.program);

	// upload OBJECT_TO_CLIP to the proper uniform location:
	glUniformMatrix4fv(color_texture_program.OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(court_to_clip));

	// use the mapping vertex_buffer_for_color_texture_program to fetch vertex data:
	glBindVertexArray(vertex_buffer_for_color_texture_program);

	// bind the solid white texture to location zero so things will be drawn just with their colors:
	// if you want to use more than 1 testure, use glUniform1i
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, white_tex);

	// run the OpenGL pipeline:
	glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertices.size()));

	// unbind the solid white texture:
	glBindTexture(GL_TEXTURE_2D, 0);

	// reset vertex array to none:
	glBindVertexArray(0);

	// reset current program to none:
	glUseProgram(0);
	
	GL_ERRORS(); // PARANOIA: print errors just in case we did something wrong.
}

void MemoryGameMode::draw_init()
{
	// draw a blue plus-sign on the screen
	// glm::u8vec4 blue = HEX_TO_U8VEC4(0x4cc9f0ff);
	draw_rectangle(vertices, glm::vec2(0, 0), glm::vec2(court_radius.x / 16, court_radius.y / 2), fg_color);
	draw_rectangle(vertices, glm::vec2(0, 0), glm::vec2(court_radius.x / 2, court_radius.x / 16), fg_color);
}

void MemoryGameMode::draw_pattern_delivery()
{
	pattern.draw(court_radius);

	{ // Add vertex data from MemoryPattern
		for (auto v = pattern.vertices.begin(); v < pattern.vertices.end(); v++)
		{
			vertices.emplace_back(*v);
		}
	}
}

void MemoryGameMode::draw_pattern_recall()
{
	// for now, draw nothing on the screen

	// TODO: add visual feedback for player input
}

void MemoryGameMode::draw_finish()
{
	// right now the game never enters the FINISH state so 
	// this function never gets called.
	
	// TODO: add a maximum level and display the FINISH screen if the player 
	// makes it to, say, level 10
}