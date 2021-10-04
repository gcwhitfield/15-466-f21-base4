#include "DrawText.hpp"

// initialization of freetype and harfbuzz based on: 
// https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
// https://www.freetype.org/freetype2/docs/tutorial/step1.html

void Font::initialize(std::string fontFileName)
{
     // make a texture from FreeType

    // 1) Load font with Freetype
    // copied from https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE:: Could not init FreeType Library " << std::endl;
        exit(0);
    }

    const char* fontFile = fontFileName.c_str();

    if (FT_New_Face(ft, fontFile, 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        exit(0);
    }

    // disable alignment since what we read from the face (font) is grey-scale. 
    // this line was copied from https://github.com/ChunanGang/TextBasedGame/blob/main/TextRenderer.cpp
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    FT_Set_Pixel_Sizes(face, 0, 48);

    // 2) load characters into map with FreeType + Harfbuzz
    // Font::Character c;
    for (char c = 0; c < std::numeric_limits<char>().max(); c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
            exit(0);
        }

        // 2.0) Create a texture from glyph
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

        // 2.1) Use harfbuzz to extract Size, Bearing, and Advance data of each 
        // character
        // Create hb-ft font
        hb_font_t *hb_font;
        hb_font = hb_ft_font_create(face, NULL);
        
        // Create hb_buffer and populate
        hb_buffer_t *hb_buffer;
        hb_buffer = hb_buffer_create();
        hb_buffer_add_utf8(hb_buffer, &c, -1, 0, -1 );
        hb_buffer_guess_segment_properties(hb_buffer);
        
        // shape it!
        hb_shape(hb_font, hb_buffer, NULL, 0);

        // get glyph information and positions out of the buffer
        // unsigned int len = hb_buffer_get_length(hb_buffer);
        // hb_glyph_info_t *info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
        hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);

        Character newCharObj;
        newCharObj.TextureID = newTex;     
        newCharObj.Size = glm::ivec2(20, 20); 
        newCharObj.Bearing = glm::ivec2((int)(pos[0].x_offset / 64.),  (int)(pos[0].y_offset / 64.));
        newCharObj.Advance = (int)(pos[0].x_advance / 64.);

        characters.insert(std::pair<char, Character>(c, newCharObj));
        
        // // print out the contents as is
        // for (size_t i = 0; i < len; i++)
        // {
        //     hb_codepoint_t gid = info[i].codepoint;
        //     unsigned int cluster = info[i].cluster;
        //     double x_advance = pos[i].x_advance / 64.;
        //     double y_advance = pos[i].y_advance / 64.;
        //     double x_offset = pos[i].x_offset / 64.;
        //     double y_offset = pos[i].y_offset / 64.;

        //     char glyphname[32];
        //     hb_font_get_glyph_name (hb_font, gid, glyphname, sizeof(glyphname));

        //     printf("glyph='%s' cluster=%d advance=(%g,%g) offset=(%g,%g)\n",
        //     glyphname, cluster, x_advance, y_advance, x_offset, y_offset);
        // }

        // set filtering and wrapping parameters:
        // parameters copied form https://github.com/ChunanGang/TextBasedGame/blob/main/TextRenderer.cpp
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

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
    

    GL_ERRORS(); // PARANOIA: print out any OpenGL errors that may have happened

}


// function body copied from https://learnopengl.com/code_viewer_gh.php?code=src/7.in_practice/2.text_rendering/text_rendering.cpp
void DrawText::draw_text(std::string text, glm::ivec2 pos, glm::vec2 bbox_size, 
		glm::u8vec4 const &color, glm::vec2 txt_size)        
{
    if (text != text_to_display)
    { // reformat harfbuzz to handle new text

        
    }
    
    glm::ivec2 currAdvance(0, 0);

    for (int i = 0; i < text.length(); i++)
    {
        char c = text[i];
        Font::Character cObj = font.characters[c];
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cObj.TextureID);

        vertices.clear();

        draw_rectangle(vertices, pos + currAdvance + cObj.Bearing, txt_size, color);

        currAdvance.x += cObj.Advance;

        // run the OpenGL pipeline:
        glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertices.size()));

        // unbind the solid white texture:
        glBindTexture(GL_TEXTURE_2D, 0);
    }
	
}