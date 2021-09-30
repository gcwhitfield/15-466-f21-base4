
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

// copied from OpenGL tutorial about rendering text
// https://learnopengl.com/code_viewer_gh.php?code=src/7.in_practice/2.text_rendering/text.fs
void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}