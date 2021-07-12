#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

layout (std140) uniform transforms {
	mat4 proj_view_model_mat;
	mat4 model_mat;
};

void main()
{
    TexCoords = aPos;
    vec4 pos = proj_view_model_mat * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  
