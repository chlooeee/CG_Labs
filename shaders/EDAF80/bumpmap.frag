#version 410

uniform sampler2D sample_texture_normals;
uniform sampler2D sample_texture;
uniform mat4 normal_model_to_world;

uniform vec3 light_position;
uniform vec3 camera_position;
uniform vec3 ambient;
//uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;

in VERTEX {
	vec3 vertex_position;
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

	// Determine the perturbed normal in world space
	vec3 normal_perturbed = texture(sample_texture_normals, fs_in.texture_coords).xyz;
	normal_perturbed = normal_perturbed * 2 - vec3(1.0, 1.0, 1.0);
	
	normal_perturbed = normal_perturbed.x * tangent + normal_perturbed.y * binormal + normal_perturbed.z * normal;
	vec3 normal_world = (normal_model_to_world * vec4(normal_perturbed, 0)).xyz;
	
	// Phong shading
	vec3 light = normalize(light_position - fs_in.vertex_position);
	normal_world = normalize(normal_world);
	
	vec3 diffuse = texture(sample_texture, fs_in.texture_coords).xyz;
	vec3 diffuse_color = max(diffuse*dot(light, normal_world), vec3(0,0,0));
	
	vec3 view_vector = normalize(camera_position - fs_in.vertex_position);
	
	vec3 specular_color = max(specular * pow(dot(reflect(-light, normal_world), view_vector), shininess),vec3(0,0,0));
	
	frag_color = vec4((ambient + diffuse_color + specular_color), 1.0f);
}
