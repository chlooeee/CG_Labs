#version 410

uniform samplerCube skybox_cube;

in vec3 normal;

out vec4 frag_color;

void main()
{
	// Normalize!
	normal = normalize(normal);

	frag_color = texture(skybox_cube, normal);
}
