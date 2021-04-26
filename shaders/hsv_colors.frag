#version 330 core

in vec4 interp_color;

out vec4 frag_color;

vec4 hsva2rgba(vec4 hsva) {
    float H = hsva.x * 360;
    int hi = int(H/60);
    float f = H/60 - hi;
    float p = hsva.z * (1 - hsva.y);
    float q = hsva.z * (1 - hsva.y * f);
    float t = hsva.z * (1 - hsva.y * (1 - f));
    if (hi == 0 || hi == 6)
		return vec4(hsva.z, t, p, 1);
	else if (hi == 1)
		return vec4(q, hsva.z, p, 1);
	else if (hi == 2)
		return vec4(p, hsva.z, t, 1);
	else if (hi == 3)
		return vec4(p, q, hsva.z, 1);
	else if (hi == 4)
		return vec4(t, p, hsva.z, 1);
	else if (hi == 5)
		return vec4(hsva.z, p, q, 1);

}

void main()
{
    frag_color = hsva2rgba(interp_color);
}
