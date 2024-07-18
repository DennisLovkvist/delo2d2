#version 330 core

layout (location = 0) in vec3 a_vertex;         
layout (location = 1) in vec2 a_tex_coord;      
layout (location = 2) in vec4 a_color;          
layout (location = 3) in mat4 a_transform;      
layout (location = 4) in vec2 a_offset;         
layout (location = 5) in vec4 a_src_rect;       

out vec2 v_tex_coord;
uniform mat4 u_mvp;

void main()
{
    vec3 position = a_vertex + vec3(a_offset,0);       
    gl_Position = vec4(position, 1.0)*u_mvp; 
    v_tex_coord = a_tex_coord * a_src_rect.zw + a_src_rect.xy;
}