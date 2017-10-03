#version 410

// uniform sampler2D diffuse_texture;

uniform vec3 camera_position;

in VS_OUT {
	vec3 world_position;

	vec2 texcoord;

	vec3 normal;
	vec3 binormal;
	vec3 tangent;
} fs_in;

out vec4 frag_color;

void main()
{
	vec4 color_deep = vec4(0.0, 0.0, 0.1, 1.0);
	vec4 color_shallow = vec4(0.0, 0.5, 0.5, 1.0);

	vec3 normal = normalize( fs_in.normal);
	vec3 binormal = normalize(fs_in.binormal);
	vec3 tangent = normalize(fs_in.tangent);

	vec3 view = normalize(camera_position - fs_in.world_position);

	float facing = 1- max(dot(view, normal), 0);

	frag_color = mix(color_deep, color_shallow, facing);
}
