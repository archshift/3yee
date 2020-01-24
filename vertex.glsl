#version 300 es
layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec2 i_texpos;

precision mediump float;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform float u_time;

out vec2 texpos;
out vec3 normal;

vec3 fn(float u, float v)
{
    float t = u_time;
    __INCLUDE_XYZ__
    return vec3(x, y, z);
}

vec3 fn_normal(float u, float v)
{
    float eps = 0.01;
    vec3 df_du = fn(u + eps, v) - fn(u - eps, v);
    vec3 df_dv = fn(u, v + eps) - fn(u, v - eps);
    vec3 normal_unnormalized = cross(df_du, df_dv);
    return normalize(normal_unnormalized) * 0.5 + 0.5;
}

void main()
{
    mat4 mvp = u_proj * u_view * u_model;
    gl_Position = mvp * vec4(fn(i_pos.x, i_pos.z), 1.0);
    normal = fn_normal(i_pos.x, i_pos.z);
    texpos = i_texpos;
}
