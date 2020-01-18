#version 330 core
uniform float u_time;
uniform sampler2D u_texture;

in vec2 texpos;
in vec3 normal;

out vec4 FragColor;

void main()
{
    vec3 color = normal;
    FragColor = vec4(color, 1.0f);
}
