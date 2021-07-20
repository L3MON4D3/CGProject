#version 330 core
layout (std140) uniform transforms {
	mat4 proj_view_model_mat;
	mat4 model_mat;
};

void main()
{
    gl_Position = proj_view_model_mat * vec4(0, 0, 0, 1.0);
}
