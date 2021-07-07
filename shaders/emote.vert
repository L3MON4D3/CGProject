#version 330 core
layout (location = 0) in vec2 position;

out vec2 tex_coord;

layout (std140) uniform transforms {
	mat4 proj_view_model_mat;
	mat4 model_mat;
};

void main()
{
	tex_coord = vec2(position.x+0.5, position.y+0.5);
    gl_Position = proj_view_model_mat * vec4(position.x, position.y, 0, 1.0);
}
