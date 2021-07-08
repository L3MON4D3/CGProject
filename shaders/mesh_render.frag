#version 330 core

in vec4 interp_color;
in vec3 interp_normal;
in vec3 pos;

out vec4 frag_color;

layout (std140) uniform lighting {
	vec3 light_pos;
	vec3 cam_pos; 
};

layout (std140) uniform material {
	float roughness;
	float refractionIndex;
};

vec4 diffuse = interp_color;
vec4 specular = vec4(1,1,1,1);

const float pi = 3.14159265359;

vec3 v = normalize(cam_pos - pos);
vec3 light_dir = normalize(light_pos - pos);

float r_d_pi = 3/pi;

// Syntatic sugar. Make sure dot products only map to hemisphere
float cdot(vec3 a, vec3 b) {
	return clamp(dot(a,b), 0.0, 1.0);
}

// D
float beckmannDistribution(float dotNH) {
	float sigma2 = roughness * roughness;
	float alpha = acos(dotNH);
	return exp(-1*pow(tan(alpha)/roughness, 2))/(pi*sigma2*pow(dotNH, 4));
}

// F
float schlickApprox(float dotVH, float n1, float n2) {
	float r0 = pow((n1 - n2)/(n1 + n2), 2);
	return r0 + (1-r0)*pow((1-dotVH), 5);
}

// G
float geometricAttenuation(float dotNH, float dotVN, float dotVH, float dotNL) {
	float smaller = 2*dotNH/dotVH;
	if (dotVN < dotNL)
		smaller *= dotVN;
	else
		smaller *= dotNL;

	return smaller > 1 ? 1 : smaller;
}

float cooktorranceTerm(vec3 n, vec3 l) {
	vec3 h = normalize(l + v); // half-vector between V and L

	// precompute to avoid redundant computation
	float dotVN = cdot(v, n);
	float dotNL = cdot(n, l);
	float dotNH = cdot(n, h);
	float dotVH = cdot(v, h);

	float D = beckmannDistribution(dotNH);
	float F = schlickApprox(dotVH, 1.0, refractionIndex);
	float G = geometricAttenuation(dotNH, dotVN, dotVH, dotNL);

	return max(D * F * G / (4.0 * dotVN * dotNL), 0.0);
}

//Peform one iteration of Gram Schmidt.
vec3 gs1(vec3 a, vec3 b) {
	return normalize(a - cdot(a, b)*b);
}

float orennayarTerm(float lambert, vec3 n, vec3 l) {
	float sigma2 = roughness * roughness; // sigma^2

	float A = 1 - .5*sigma2/(sigma2+.33);
	float B = .45*sigma2/(sigma2+.09);

	// both are normalized.
	float theta_v = acos(cdot(n, v));
	float theta_l = acos(cdot(n, l));

	float alpha, beta;
	if (theta_v > theta_l) {
		alpha = theta_v;
		beta = theta_l;
	} else {
		alpha = theta_l;
		beta = theta_v;
	}

	// max realised with cdot.
	return lambert * (A + B*cdot(gs1(v, n),  gs1(l, n))*sin(alpha)*tan(beta));
}

void main() {
	float diffuseTerm = orennayarTerm(r_d_pi, interp_normal, light_dir);
	// lowest possbile value = ambient fake light term
	diffuseTerm = max(diffuseTerm, 0.1);
	// as specular part we compute the Cook-Torrance term
	float specularTerm = cooktorranceTerm(interp_normal, light_dir);
	// combine both terms (diffuse+specular) using our material properties (colors)
	frag_color = vec4(vec3(clamp(diffuse * diffuseTerm + specular * specularTerm, 0.0, 1.0)*cdot(interp_normal, light_dir)), 1);
}
