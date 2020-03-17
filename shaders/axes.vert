#version 300 es
layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec2 i_texpos;

precision mediump float;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec2 texpos;

void main()
{
    mat4 mvp = u_proj * u_view * u_model;
    gl_Position = mvp * vec4(i_pos, 1.0f);
    texpos = i_texpos;
}
