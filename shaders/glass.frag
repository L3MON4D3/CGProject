#version 330 core
out vec4 frag_color;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    vec3 tex = vec3(texture(skybox, TexCoords));
    frag_color = vec4(tex, 1);
}
