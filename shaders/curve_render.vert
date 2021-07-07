#version 330 core
layout (location = 0) in vec3 position;

layout (std140) uniform transforms {
	mat4 proj_view_model_mat;
	mat4 model_mat;
};

void main()
{
    gl_Position = proj_view_model_mat * vec4(position.x, position.y, position.z, 1.0);
}
