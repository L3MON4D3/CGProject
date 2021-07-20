#version 330 core
layout (location = 0) in vec2 vert_position;
layout (location = 1) in vec3 instance_position;
layout (location = 2) in vec4 instance_color;

out vec4 color;

layout (std140) uniform transforms {
	mat4 proj_view_model_mat;
	mat4 model_mat;
};

void main()
{
	color = instance_color;
    mat4 pvm = proj_view_model_mat
    	* model_mat
    	* mat4(
    		1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			instance_position, 1);

	float scale = length(vert_position);
	pvm[0] = vec4(scale,0,0,0);
	pvm[1] = vec4(0,scale,0,0);
	pvm[2] = vec4(0,0,scale,0);

    gl_Position = pvm * vec4(vert_position.x, vert_position.y, 0, 1.0);
}
