#version 410

uniform samplerCube skybox_cube;

in VS_OUT{
	vec3 world_normal;
} fs_in;

out vec4 frag_color;

void main()
{
	// Normalize!
	vec3 normal = normalize(fs_in.world_normal);

	frag_color = texture(skybox_cube, normal);
}
