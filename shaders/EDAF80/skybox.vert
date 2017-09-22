#version 410

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;

uniform mat4 vertex_model_to_world;
uniform mat4 vertex_world_to_clip;
uniform mat4 normal_model_to_world;


out vec3 normal;


void main()
{
	normal = (normal_model_to_world * vec4(normal, 0.0)).xyz;
	vec4 vertex_position = vertex_model_to_world * vec4(vertex, 1.0);

	gl_Position = vertex_world_to_clip * vertex_position;
}

