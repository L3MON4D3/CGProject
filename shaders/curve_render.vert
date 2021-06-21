#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 proj_view_mat;

void main()
{
    gl_Position = proj_view_mat * vec4(position.x, position.y, position.z, 1.0);
}
