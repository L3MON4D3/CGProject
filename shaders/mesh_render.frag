#version 330 core

in vec4 interp_color;
in vec3 interp_normal;
uniform vec3 light_dir;

out vec4 frag_color;

float pi = 3.14159;
float r_d_pi = 2/pi;
float r_s = .8;
float m = 20;

float cdot(vec3 a, vec3 b) {
	return clamp(dot(a,b), 0.0, 1.0);
}

float brdf(vec3 l, vec3 n, vec3 e) {
	vec3 h = normalize(l+e);
	return r_d_pi+r_s*pow(cdot(h, n), m)/cdot(l,n);
}

void main()
{
	frag_color = brdf(light_dir, interp_normal, vec3(0,0,1))*interp_color*cdot(interp_normal, light_dir);
}
