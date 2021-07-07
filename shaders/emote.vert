#version 330 core
layout (location = 0) in vec2 position;
out vec2 tex_coord;

uniform mat4 pvm;

void main()
{
	tex_coord = vec2(position.x+0.5, position.y+0.5);
    gl_Position = pvm * vec4(position.x, position.y, 0, 1.0);
}
