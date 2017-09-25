#version 410

uniform samplerCube reflection_cube;
uniform sampler2D bumpmap;

uniform vec3 camera_position;

uniform mat4 normal_model_to_world;

in VS_OUT{
	vec3 world_position;
	vec2 texture_coords;
	vec3 tangent;
	vec3 binormal;
	vec3 normal;
} fs_in;

out vec4 frag_color;

void main()
{
	// Normalize!
	vec3 tangent = normalize(fs_in.tangent);
	vec3 binormal = normalize(fs_in.binormal);
	vec3 normal = normalize(fs_in.normal);
	
	// Calculate perturbed normal and transform to world space
	vec3 perturbed_normal_tangent = texture(bumpmap, fs_in.texture_coords).xyz;
	perturbed_normal_tangent = perturbed_normal_tangent * 2 - 1.0;
	
	vec3 perturbed_normal_model = perturbed_normal_tangent.x*tangent + perturbed_normal_tangent.y * binormal + perturbed_normal_tangent.z*normal;

	vec3 perturbed_normal_world = (normal_model_to_world * vec4(perturbed_normal_model, 0)).xyz;
	
	// Sample the reflection
	vec3 view = normalize(camera_position - fs_in.world_position);
	vec3 reflection_dir = reflect(-view, perturbed_normal_world);

	frag_color = texture(reflection_cube, reflection_dir);
}
