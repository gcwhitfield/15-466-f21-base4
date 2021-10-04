#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("hexapod.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("hexapod.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("dusty-floor.opus"));
});

PlayMode::PlayMode() : scene(*hexapod_scene) {
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		if (transform.name == "Hip.FL") hip = &transform;
		else if (transform.name == "UpperLeg.FL") upper_leg = &transform;
		else if (transform.name == "LowerLeg.FL") lower_leg = &transform;
	}
	if (hip == nullptr) throw std::runtime_error("Hip not found.");
	if (upper_leg == nullptr) throw std::runtime_error("Upper leg not found.");
	if (lower_leg == nullptr) throw std::runtime_error("Lower leg not found.");

	hip_base_rotation = hip->rotation;
	upper_leg_base_rotation = upper_leg->rotation;
	lower_leg_base_rotation = lower_leg->rotation;

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	//start music loop playing:
	// (note: position will be over-ridden in update())
	leg_tip_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, get_leg_tip_position(), 10.0f);

	//font initialization
	{
		{ // vertex buffer:
			glGenBuffers(1, &font_vertex_buffer);
			// for now, buffer will be un-filled.

			GL_ERRORS(); // PARANOIA: print out any OpenGL errors that may have happened
		}

		{ // vertex array mapping buffer for font_program:
			// ask OpenGL to fill font_vertex_attributes with the name of an unused vertex array object:
			glGenVertexArrays(1, &font_vertex_attributes);

			// set font_vertex_attributes as the current vertex array object:
			glBindVertexArray(font_vertex_attributes);

			// set font_vertex_buffer as the source of glVertexAttribPointer() commands:
			glBindBuffer(GL_ARRAY_BUFFER, font_vertex_buffer);

			// set up the vertex array object to describe arrays of MemoryGameMode::Vertex:
			glVertexAttribPointer(
				font_program.Position_vec4, // attribute
				3, // size
				GL_FLOAT, // type
				GL_FALSE, // normalized
				sizeof(Vertex), // stride
				(GLbyte *)0 + 0 // offset
			);
			glEnableVertexAttribArray(font_program.Position_vec4);
			// [Note that it is okay to bind a vec3 input to a vec4 attribute -- the w component will be filled with 1.0 automatically]

			glVertexAttribPointer(
				font_program.Color_vec4, // attribute
				4, // size
				GL_UNSIGNED_BYTE, // type
				GL_TRUE, // normalized
				sizeof(Vertex), // stride
				(GLbyte *)0 + 4*3 // offset
			);
			glEnableVertexAttribArray(font_program.Color_vec4);

			glVertexAttribPointer(
				font_program.TexCoord_vec2, // attribute
				2, // size
				GL_FLOAT, // type
				GL_FALSE, // normalized
				sizeof(Vertex), // stride
				(GLbyte *)0 + 4*3 + 4*1 // offset
			);
			glEnableVertexAttribArray(font_program.TexCoord_vec2);

			// done referring to font_vertex_buffer, so unbind it:
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// done setting up vertex array object, so unbind it:
			glBindVertexArray(0);

			GL_ERRORS(); // PARANOIA: print out any OpenGL errors that may have happened
		}

		// quicksilverFont.initialize(quicksilverFontFile);
		// quicksilverText = DrawText(
		// 	quicksilverFontFile,
		// 	font_program.program,
		// 	font_vertex_attributes,
		// 	font_vertex_buffer
		// );

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
				data.data()
			);

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
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	wobble += elapsed / 10.0f;
	wobble -= std::floor(wobble);

	hip->rotation = hip_base_rotation * glm::angleAxis(
		glm::radians(5.0f * std::sin(wobble * 2.0f * float(M_PI))),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	upper_leg->rotation = upper_leg_base_rotation * glm::angleAxis(
		glm::radians(7.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);
	lower_leg->rotation = lower_leg_base_rotation * glm::angleAxis(
		glm::radians(10.0f * std::sin(wobble * 3.0f * 2.0f * float(M_PI))),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);

	//move sound to follow leg tip position:
	leg_tip_loop->set_position(get_leg_tip_position(), 1.0f / 60.0f);

	//move camera:
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * forward;
	}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		glm::vec3 at = frame[3];
		Sound::listener.set_position_right(at, right, 1.0f / 60.0f);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ // draw text

		// compute window aspect ratio:
		float aspect = drawable_size.x / float(drawable_size.y);
		// we'll scale the x coordinate by 1.0 / aspect to make sure things stay square.

		glm::vec2 center = drawable_size;
		center.x *= 0.5;
		center.y *= 0.5;


		// build matrix that scales and translates appropriately:
		glm::mat4 court_to_clip = glm::mat4(
			glm::vec4(aspect, 0.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, aspect, 0.0f, 0.0f),
			glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
			glm::vec4(-center.x * aspect, -center.y * aspect, 0.0f, 1.0f)
		);

		// set color_texture_program as current program:
		glUseProgram(font_program.program);

		// upload OBJECT_TO_CLIP to the proper uniform location:
		glUniformMatrix4fv(font_program.OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(court_to_clip));

		// use the mapping font_vertex_attributes to fetch vertex data:
		glBindVertexArray(font_vertex_attributes);

		// DrawText.draw_text calls glDrawArrays
		// quicksilverText.draw_text(
		// 	"Hooga booga",
		// 	glm::ivec2(50, 50),
		// 	glm::vec2(1, 1),
		// 	glm::u8vec4(0x24, 0x24, 0x24, 0xff),
		// 	glm::vec2(50, 50)
		// );

		std::vector <Vertex> vertices;
		draw_rectangle(vertices, glm::vec2(50, 50), glm::vec2(50, 50), glm::u8vec4(0x22, 0x33, 0x44, 0xff));

		glBindBuffer(GL_ARRAY_BUFFER, font_vertex_buffer); // set font_vertex_buffer as current
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_DYNAMIC_DRAW); // upload vertices array
		glBindBuffer(GL_ARRAY_BUFFER, 0);

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
	}

	// {	// try to draw some text using Harfbuzz and FreeType

	// 	const char *fontfile = "./fonts/quicksilver_3/Quicksilver.ttf";
	// 	const char *text = "Avocados are tasty AF";

	// 	/* Initialize FreeType and create FreeType font face. */
	// 	FT_Library ft_library;
	// 	FT_Face ft_face;
	// 	FT_Error ft_error;

	// 	if ((ft_error = FT_Init_FreeType (&ft_library)))
	// 		abort();
	// 	if ((ft_error = FT_New_Face (ft_library, fontfile, 0, &ft_face)))
	// 		abort();
	// 	if ((ft_error = FT_Set_Char_Size (ft_face, FONT_SIZE*64, FONT_SIZE*64, 0, 0)))
	// 		abort();

	// 	/* Create hb-ft font. */
	// 	hb_font_t *hb_font;
	// 	hb_font = hb_ft_font_create (ft_face, NULL);

	// 	/* Create hb-buffer and populate. */
	// 	hb_buffer_t *hb_buffer;
	// 	hb_buffer = hb_buffer_create ();
	// 	hb_buffer_add_utf8 (hb_buffer, text, -1, 0, -1);
	// 	hb_buffer_guess_segment_properties (hb_buffer);

	// 	/* Shape it! */
	// 	hb_shape (hb_font, hb_buffer, NULL, 0);

	// 	/* Get glyph information and positions out of the buffer. */
	// 	unsigned int len = hb_buffer_get_length (hb_buffer);
	// 	hb_glyph_info_t *info = hb_buffer_get_glyph_infos (hb_buffer, NULL);
	// 	hb_glyph_position_t *pos = hb_buffer_get_glyph_positions (hb_buffer, NULL);

	// 	/* And converted to absolute positions. */
	// 	{
	// 		double current_x = 0;
	// 		double current_y = 0;
	// 		for (unsigned int i = 0; i < len; i++)
	// 		{
	// 		hb_codepoint_t gid   = info[i].codepoint;
	// 		// unsigned int cluster = info[i].cluster;
	// 		// double x_position = current_x + pos[i].x_offset / 64.;
	// 		// double y_position = current_y + pos[i].y_offset / 64.;


	// 		char glyphname[32];
	// 		hb_font_get_glyph_name (hb_font, gid, glyphname, sizeof (glyphname));

	// 		/*
	// 		printf ("glyph='%s'	cluster=%d	position=(%g,%g)\n",
	// 			glyphname, cluster, x_position, y_position);

	// 		*/
	// 		current_x += pos[i].x_advance / 64.;
	// 		current_y += pos[i].y_advance / 64.;
	// 		}
	// 	}

	// 	// example code copied from freetype documentation
	// 	FT_GlyphSlot  slot = ft_face->glyph;  /* a small shortcut */
	// 	// FT_UInt       glyph_index;
	// 	int           pen_x, pen_y, n;
	// 	size_t num_chars = strlen(text);

	// 	pen_x = 300;
	// 	pen_y = 200;

	// 	std::map<char, Font::Character> characters;

	// 	for ( n = 0; n < num_chars; n++ )
	// 	{

	// 		// float x = 50;
	// 		// float y = 50; 
	// 		float scale = 1;
	// 		/* load glyph image into the slot (erase previous one) */
	// 		FT_Error error = FT_Load_Char( ft_face, text[n], FT_LOAD_RENDER );
	// 		if ( error )
	// 			continue;  /* ignore errors */

	// 		/* now, draw to our target surface */
	// 		Font::Character ch = characters[text[n]];

	// 		float xpos = pen_x + ch.Bearing.x * scale;
	// 		float ypos = pen_y - (ch.Size.y - ch.Bearing.y) * scale;

	// 		float w = ch.Size.x * scale;
	// 		float h = ch.Size.y * scale;
	// 		// update VBO for each character
	// 		float vertices[6][4] = {
	// 			{ xpos,     ypos + h,   0.0f, 0.0f },            
	// 			{ xpos,     ypos,       0.0f, 1.0f },
	// 			{ xpos + w, ypos,       1.0f, 1.0f },

	// 			{ xpos,     ypos + h,   0.0f, 0.0f },
	// 			{ xpos + w, ypos,       1.0f, 1.0f },
	// 			{ xpos + w, ypos + h,   1.0f, 0.0f }           
	// 		};

	// 		unsigned int texture;
	// 		glGenTextures(1, &texture);
	// 		glBindTexture(GL_TEXTURE_2D, texture);
	// 		glTexImage2D(
	// 			GL_TEXTURE_2D,
	// 			0,
	// 			GL_RED,
	// 			ft_face->glyph->bitmap.width,
	// 			ft_face->glyph->bitmap.rows,
	// 			0,
	// 			GL_RED,
	// 			GL_UNSIGNED_BYTE,
	// 			ft_face->glyph->bitmap.buffer
	// 		);
	// 		// set texture options
	// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			
	// 		// now store character for later use
	// 		Font::Character character = {
	// 			texture, 
	// 			glm::ivec2(ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows),
	// 			glm::ivec2(ft_face->glyph->bitmap_left, ft_face->glyph->bitmap_top),
	// 			static_cast<unsigned int>(ft_face->glyph->advance.x)
	// 		};
	// 		if (characters.find(text[n]) == characters.end())
	// 		{
	// 			characters.insert(std::pair<char, Font::Character>(text[n], character));
	// 		}

	// 		// 'generate texutre', 'set texutre options', and 'store char for later use'
	// 		// sections were copied from Learn OpenGL tutorial on font rendering:
	// 		// https://learnopengl.com/In-Practice/Text-Rendering
	// 		// generate texture


	// 		// render glyph texture over quad
	// 		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
	// 		// update content of VBO memory
	// 		glBindBuffer(GL_ARRAY_BUFFER, hexapod_meshes_for_lit_color_texture_program);
	// 		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

	// 		glBindBuffer(GL_ARRAY_BUFFER, 0);
	// 		// render quad
	// 		glDrawArrays(GL_TRIANGLES, 0, 6);
	// 		/*
	// 		my_draw_bitmap( &slot->bitmap,
	// 						pen_x + slot->bitmap_left,
	// 						pen_y - slot->bitmap_top );
	// 		*/
	// 		/* increment pen position */

	// 		pen_x += slot->advance.x >> 6;

	// 		//glm::vec3 color(1, 0, 1);

	// 		// 'iterate through all characters' codeblock copied form opengl tutorial
	// 		// https://learnopengl.com/code_viewer_gh.php?code=src/7.in_practice/2.text_rendering/text_rendering.cpp
	// 		// iterate through all characters
	// 		// for (size_t i = 0; i < strlen(text); i++) 
	// 		// {
	// 		// 	// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
	// 		// 	x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	// 		// }

	// 		glBindVertexArray(0);
    // 		glBindTexture(GL_TEXTURE_2D, 0);

	// 	}
	// }


	GL_ERRORS();
}

glm::vec3 PlayMode::get_leg_tip_position() {
	//the vertex position here was read from the model in blender:
	return lower_leg->make_local_to_world() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
}
