
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

// copied from OpenGL tutorial about rendering text
// https://learnopengl.com/code_viewer_gh.php?code=src/7.in_practice/2.text_rendering/text.vs
uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}