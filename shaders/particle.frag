#version 330 core

uniform sampler2D tex;

in vec4 color;
out vec4 frag_color;

void main()
{
    frag_color = color;
}
