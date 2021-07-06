#version 330 core
layout (location = 0) in vec2 position;

uniform mat4 pvm;

void main()
{
    gl_Position = pvm * vec4(position.x, position.y, 0, 1.0);
}
