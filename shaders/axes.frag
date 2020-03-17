#version 300 es
precision mediump float;

uniform sampler2D u_texture;
// uniform vec3 u_color;

in vec2 texpos;

out vec4 FragColor;

void main()
{
    FragColor = vec4(0.2f, 0.2f, 0.2f, 1.0f);
}
