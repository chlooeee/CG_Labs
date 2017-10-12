#version 410

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 texcoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 binormal;

uniform mat4 vertex_model_to_world;
uniform mat4 vertex_world_to_clip;

out VERTEX {
	vec3 vertex_position;
	vec2 texture_coords;
	vec3 tangent;
	vec3 binormal;
	vec3 normal;
} vs_out;


void main()
{
	vs_out.texture_coords = vec2(texcoord.y, texcoord.x);
	vs_out.tangent = tangent;
	vs_out.binormal = binormal;
	vs_out.normal = normal;
	vs_out.vertex_position = (vertex_model_to_world * vec4(vertex, 1.0)).xyz;

	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vertex, 1.0);
}



