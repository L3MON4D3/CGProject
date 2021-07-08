#version 330 core
layout (location = 0) in float _;

layout (std140) uniform transforms {
	mat4 proj_view_model_mat;
	mat4 model_mat;
};

layout (std140) uniform lighting {
	vec3 light_pos;
	vec3 cam_pos; 
};

void main()
{
    gl_Position = proj_view_model_mat * vec4(light_pos, 1);
}
