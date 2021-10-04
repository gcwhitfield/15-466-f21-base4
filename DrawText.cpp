#include "DrawText.hpp"

// initialization of freetype and harfbuzz based on: 
// https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
// https://www.freetype.org/freetype2/docs/tutorial/step1.html

void Font::initialize(std::string fontFileName)
{
    FT_Library ft;
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

    // 2) load characters into map with FreeType
    // Font::Character c;
    char LETTER_MIN = 32;
    char LETTER_MAX = 127;
    for (char c = LETTER_MIN; c < LETTER_MAX; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
            exit(0);
        }

        assert(&(this->face->glyph) != NULL);
        assert(&(this->face->glyph->bitmap) != NULL);
        // 2.0) Create a texture from glyph
        GLuint newTex = 0;
        glGenTextures(1, &newTex);
        glBindTexture(GL_TEXTURE_2D, newTex);
        glm::uvec2 size = glm::uvec2(this->face->glyph->bitmap.rows,this->face->glyph->bitmap.width);
        std::vector< glm::u8vec4 > data(size.x*size.y, glm::u8vec4(0xff, 0xff, 0xff, 0xff));
        for (size_t i = 0; i < size.y; i++)
        {
            for (size_t j = 0; j < size.x; j++)
            {
                size_t index = i * size.y + j;
                (void) index;
                // uint8_t val = this->face->glyph->bitmap.buffer[j * std::abs(this->face->glyph->bitmap.pitch) + i]; // copied from professor mccan's example code for printing bitmap buffer                
                // (void) val;
                // data[index].x = val;
                // data[index].y = val;
                // data[index].z = val;
                // data[index].w = val;
            }
        }
        // glTexImage2D(
        //     GL_TEXTURE_2D,
        //     0, 
        //     GL_RGBA,
        //     size.x,
        //     size.y,
        //     0, 
        //     GL_RGBA,
        //     GL_UNSIGNED_BYTE,
        //     data.data()
        // );

        // Character newCharObj;
        // newCharObj.TextureID = newTex;     
        // newCharObj.Size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows); 
        // newCharObj.Bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
        // // newCharObj.Advance = (int)(pos[0].x_advance / 64.);

        // characters.insert(std::pair<char, Character>(c, newCharObj));
        
        // set filtering and wrapping parameters:
        // parameters copied form https://github.com/ChunanGang/TextBasedGame/blob/main/TextRenderer.cpp
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    GL_ERRORS(); // PARANOIA: print out any OpenGL errors that may have happened
}


void DrawText::updateTextData()
{
    hb_font_t *hb_font;
    hb_font = hb_ft_font_create(font.face, NULL);

    // this body of this function was mostly copied from 
    // https://github.com/ChunanGang/TextBasedGame/blob/main/TextRenderer.cpp
    
    // free previous resources
    if(hb_buffer)
        hb_buffer_destroy(hb_buffer);
    if(hb_font)
        hb_font_destroy(hb_font);

    // recreate hb resources
    hb_font = hb_ft_font_create (font.face, NULL);
    /* Create hb-buffer and populate. */
    hb_buffer = hb_buffer_create ();

    // reshape
    hb_buffer_add_utf8 (hb_buffer, text_to_display.c_str(), -1, 0, -1);
    hb_buffer_guess_segment_properties (hb_buffer);
    hb_shape (hb_font, hb_buffer, NULL, 0);

    /* Get glyph information and positions out of the buffer. */
    info = hb_buffer_get_glyph_infos (hb_buffer, NULL);
    pos = hb_buffer_get_glyph_positions (hb_buffer, NULL);
}

// function body inspried by
// https://learnopengl.com/code_viewer_gh.php?code=src/7.in_practice/2.text_rendering/text_rendering.cpp
void DrawText::draw_text(std::string text, glm::ivec2 pos, glm::vec2 bbox_size, 
		glm::u8vec4 const &color, glm::vec2 txt_size)        
{
    if (text != text_to_display)
    { // reformat harfbuzz to handle new text
        text_to_display = text;
        updateTextData();
    }
    
    glm::ivec2 currAdvance(0, 0);

    for (int i = 0; i < text.length(); i++)
    {
        char c = text[i];
        Font::Character cObj = font.characters[c];
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cObj.TextureID);

        vertices.clear();

        draw_rectangle(vertices, pos, txt_size, color);

        currAdvance.x += (this->pos[i]).x_advance;// / 64.0f;
        currAdvance.y += (this->pos[i]).y_advance;// / 64.0f;


        // run the OpenGL pipeline:
        glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertices.size()));

        // unbind the solid white texture:
        glBindTexture(GL_TEXTURE_2D, 0);
    }
	
}