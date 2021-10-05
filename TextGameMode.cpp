#include "TextGameMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

TextGameMode::TextGameMode() {
	
	{ // set up the game mode
		_START_CALLED = false;
	}

	// ----- set up game state -----
	difficulty = 1;
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

		// set up the vertex array object to describe arrays of TextGameMode::Vertex:
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

	{ // make map of glyph textures using FreeType

		// 1) Load font with Freetype
		// copied from https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
		FT_Library ft;
		if (FT_Init_FreeType(&ft))
		{
			std::cout << "ERROR::FREETYPE:: Could not init FreeType Library " << std::endl;
			exit(0);
		}

		const char* fontFile = textFontFile.c_str();
		if (FT_New_Face(ft, fontFile, 0, &face))
		{
			std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
			exit(0);
		}

		// disable alignment since what we read from the face (font) is grey-scale. 
		// this line was copied from https://github.com/ChunanGang/TextBasedGame/blob/main/TextRenderer.cpp
    	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

		FT_Set_Pixel_Sizes(face, 0, 48);

		// 2) characters with FreeType
		char LETTER_MIN = 32;
        char LETTER_MAX = 127;
        for (char c = LETTER_MIN; c < LETTER_MAX; c++)
        {
			if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
			{
				std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
				exit(0);
			}

			// 3) Create a texture from glyph (should be 'X')
			GLuint newTex = 0;
			glGenTextures(1, &newTex);
			glBindTexture(GL_TEXTURE_2D, newTex);
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
				data.data()
			);

			Character newChar = {
				newTex,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), // line copied from https://github.com/ChunanGang/TextBasedGame/blob/main/TextRenderer.cpp
            	glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), // line copied from https://github.com/ChunanGang/TextBasedGame/blob/main/TextRenderer.cpp
			};
			characters.insert(std::pair<char, Character>(c, newChar));
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

			// since texture uses a mipmap and we haven't uploaded one, instruct opengl to make one for us:
			glGenerateMipmap(GL_TEXTURE_2D);
			// set filtering and wrapping parameters:
			// (it's a bit silly to mipmap a 1x1 texture, but I'm doing it because you may want to use this code to load different sizes of texture)
			// parameters copied form https://github.com/ChunanGang/TextBasedGame/blob/main/TextRenderer.cpp
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


			// Okay, texture uploaded, can unbind it:
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		GL_ERRORS(); // PARANOIA: print out any OpenGL errors that may have happened
	}

	{ // use Harfbuzz to shape text
		const char* text = &"Hello!"[0];

		// Create hb-ft font
		hb_font = hb_ft_font_create(face, NULL);
		
		// Create hb_buffer and populate
		hb_buffer = hb_buffer_create();
		hb_buffer_add_utf8(hb_buffer, text, -1, 0, -1 );
		hb_buffer_guess_segment_properties(hb_buffer);
		
		// shape it!
		hb_shape(hb_font, hb_buffer, NULL, 0);

		// get glyph information and positions out of the buffer
		unsigned int len = hb_buffer_get_length(hb_buffer);
		info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
		pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);

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

	}
}

TextGameMode::~TextGameMode() {

	// ----- free OpenGL resources -----
	glDeleteBuffers(1, &vertex_buffer);
	vertex_buffer = 0;

	glDeleteVertexArrays(1, &vertex_buffer_for_color_texture_program);
	vertex_buffer_for_color_texture_program = 0;

	glDeleteTextures(1, &white_tex);
	white_tex = 0;

}

bool TextGameMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	// handle different types of input for each of the different states
	

	return false;
}

// start is called right before the first call to update is executed
void TextGameMode::start()
{
	curr_state = INIT;
}

void TextGameMode::update(float elapsed) {

	if (!_START_CALLED)
	{
		start();
		_START_CALLED = true;
	}

	// ----- next state logic -----

	curr_state = next_state;
}

void TextGameMode::draw(glm::uvec2 const &drawable_size) {

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
	glBindTexture(GL_TEXTURE_2D, characters['Q'].TextureID);

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

void TextGameMode::draw_init()
{
	// draw a blue plus-sign on the screen
	// glm::u8vec4 blue = HEX_TO_U8VEC4(0x4cc9f0ff);
	draw_rectangle(vertices, glm::vec2(0, 0), glm::vec2(court_radius.x / 16, court_radius.y / 2), fg_color);
	draw_rectangle(vertices, glm::vec2(0, 0), glm::vec2(court_radius.x / 2, court_radius.x / 16), fg_color);
}

void TextGameMode::draw_pattern_delivery()
{

	
}

void TextGameMode::draw_pattern_recall()
{
	// for now, draw nothing on the screen

	// TODO: add visual feedback for player input
}

void TextGameMode::draw_finish()
{
	// right now the game never enters the FINISH state so 
	// this function never gets called.
	
	// TODO: add a maximum level and display the FINISH screen if the player 
	// makes it to, say, level 10
}