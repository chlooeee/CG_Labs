#version 410

uniform samplerCube reflection_cube;
uniform vec3 camera_position;

in VS_OUT{
	vec3 world_normal;
	vec3 world_position;
} fs_in;

out vec4 frag_color;

void main()
{
	// Normalize!
	vec3 normal = normalize(fs_in.world_normal);
	vec3 view = normalize(camera_position - fs_in.world_position);
	vec3 reflection_dir = reflect(-view, normal);

	frag_color = texture(reflection_cube, reflection_dir);
}
