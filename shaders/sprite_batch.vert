#version 330 core

layout (location = 0) in vec3 a_vertex;         
layout (location = 1) in vec2 a_tex_coord;      
layout (location = 2) in vec4 a_color;        
layout (location = 3) in vec2 a_offset;                
layout (location = 4) in vec4 a_src_rect;  
layout (location = 5) in mat4 a_transform;      

out vec2 v_tex_coord;
uniform mat4 u_mvp;

void main()
{
    vec4 position = vec4(a_vertex.xy, 0.0, 1.0);
    
    // First apply the global transformation (u_mvp)
    // Then apply the local transformation (a_transform) and offset
    gl_Position = u_mvp * (a_transform * position + vec4(a_offset, 0.0, 0.0));

    v_tex_coord = a_tex_coord * a_src_rect.zw + a_src_rect.xy;
}
