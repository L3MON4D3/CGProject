#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;

layout (std140) uniform transforms {
	mat4 proj_view_model_mat;
	mat4 model_mat;
};

out vec4 interp_color;
out vec3 interp_normal;
out vec3 pos;

void main()
{
    gl_Position = proj_view_model_mat * vec4(position.x, position.y, position.z, 1.0);
	vec4 pos_ = model_mat * vec4(position, 1);
	pos = vec3(pos_/pos_.w);
    interp_color = color;
    interp_normal = normalize((transpose(inverse(model_mat)) * vec4(normal, 0.0)).xyz);
}
