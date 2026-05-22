#version 430

struct MaterialInfo{
	vec3 Kd;
};

uniform MaterialInfo Material;

out vec4 vFragColor;

void main(void)
{
    vFragColor = vec4(Material.Kd, 1.0);
}
