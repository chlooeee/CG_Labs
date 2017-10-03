#version 410

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 texcoord;

uniform mat4 vertex_model_to_world;
uniform mat4 vertex_world_to_clip;

uniform vec4 amplitudes;
uniform vec4 angular_freqs;
uniform vec4 wavenumbers;
uniform float power;
uniform vec4 directions_x;
uniform vec4 directions_z;
uniform float time;

out VS_OUT {
	vec3 world_position;

	vec2 texcoord;

	vec3 normal;
	vec3 binormal;
	vec3 tangent;
} vs_out;


void main()
{
	vs_out.texcoord = vec2(texcoord.x, texcoord.y);

	vec4 waves = amplitudes * pow((sin((directions_x * vertex.x + directions_z * vertex.z) * wavenumbers + time * angular_freqs) * 0.5 + 0.5), power * vec4(1.0));
	vec4 waves_diff_base = 0.5 * power * wavenumbers * amplitudes * pow(sin((directions_x * vertex.x + directions_z * vertex.z) * wavenumbers + time * angular_freqs) * 0.5 + 0.5, (power-1) * vec4(1.0)) *
												cos((directions_x * vertex.x + directions_z * vertex.z) * wavenumbers + time * angular_freqs);
	vec4 waves_diff_x = waves_diff_base * directions_x;
	vec4 waves_diff_z = waves_diff_base * directions_z;

	float diff_x = dot(waves_diff_x, vec4(1));
	float diff_z = dot(waves_diff_z, vec4(1));
	float wave = dot(waves, vec4(1));

	vs_out.normal = vec3(-diff_x, 1, -diff_z);
	vs_out.binormal = vec3(1, diff_x, 0);
	vs_out.tangent = vec3(0, diff_z, 1);

	vs_out.world_position = (vertex_model_to_world * vec4(vertex.x, wave, vertex.z, 1.0)).xyz;
	gl_Position = vertex_world_to_clip * vec4(vs_out.world_position, 1.0);
}
