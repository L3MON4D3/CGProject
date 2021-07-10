#version 330 core

uniform sampler2D tex;

in vec2 tex_coord;
out vec4 frag_color;

void main()
{
    vec4 tex_color = texture(tex, tex_coord);
    frag_color = vec4(tex_color.xyz/tex_color.w, tex_color.w);
}
