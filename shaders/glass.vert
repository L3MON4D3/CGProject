#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;

out vec3 TexCoords;

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
    vec3 model_normal = normalize((transpose(inverse(model_mat)) * vec4(normal, 0.0)).xyz);
    vec4 world_pos = proj_view_model_mat * vec4(pos, 1.0);
	vec3 v = normalize(cam_pos - vec3(world_pos));
	TexCoords = reflect(v, model_normal);
    gl_Position = world_pos;
}  
