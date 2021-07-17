#version 330 core
layout (location = 0) in vec3 vert_position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;
layout (location = 3) in vec3 asteroid_pos;
layout (location = 4) in vec3 rot_axis;
layout (location = 5) in float rot_amount;

layout (std140) uniform transforms {
	mat4 proj_view_mat;
	mat4 model_mat;
};

out vec4 interp_color;
out vec3 interp_normal;
out vec3 pos;

void main()
{
    mat4 m_mat = mat4(1);
    m_mat[3] = vec4(asteroid_pos, 1);

    gl_Position = proj_view_mat* m_mat * vec4(vert_position.x, vert_position.y, vert_position.z, 1.0);

	vec4 pos_ = m_mat * vec4(vert_position, 1);
	pos = vec3(pos_/pos_.w);

    interp_color = color;

    interp_normal = normalize((transpose(inverse(m_mat)) * vec4(normal, 0.0)).xyz);
}
