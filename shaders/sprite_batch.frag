#version 330 core

uniform sampler2D texture;
layout(location = 0) out vec4 color;

in vec2 v_tex_coord;

void main()
{ 
    
    vec4 tex_color = texture2D(texture,v_tex_coord); 

    color = tex_color;
    
    if(color.a == 0)
    {
        gl_FragDepth = 1.0;
    }
    else
    {
        gl_FragDepth = gl_FragCoord.z;
    }
}