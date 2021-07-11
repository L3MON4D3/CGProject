#version 330 core

uniform sampler2D tex;

in vec2 tex_coord;
out vec4 frag_color;

void main()
{
    frag_color = vec4(1,.2,0,.3);
}
