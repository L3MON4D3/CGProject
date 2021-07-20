#version 330 core

uniform sampler2D tex;

out vec4 frag_color;

void main()
{
    vec4 color = texture(tex, gl_PointCoord);
    if (color.w == 0) discard;
    frag_color = color;
}
