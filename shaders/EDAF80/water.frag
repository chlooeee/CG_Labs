#version 410

uniform samplerCube reflectioncube;
uniform sampler2D ripple_texture;

uniform vec3 camera_position;
uniform vec2 wind_direction;
uniform float time;

uniform mat4 normal_model_to_world;

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

	vec3 normal = normalize(fs_in.normal);
	vec3 binormal = normalize(fs_in.binormal);
	vec3 tangent = normalize(fs_in.tangent);

	//Construct a perturbed normal based on the sampling of waves from a texture
	vec2 texture_scale = vec2(4,2);
	float ripple_time = mod(time, 100.0);
	vec2 ripple_speed = 0.0005 * wind_direction;

	int num_ripples = 4;
	vec3 sum_ripplenormal;
	for (int n = 0; n < num_ripples; ++n) {
		vec2 sample_coords = fs_in.texcoord * texture_scale * pow(2,n) + ripple_time * ripple_speed * pow(2,n) * 2;
		sum_ripplenormal += (texture(ripple_texture, sample_coords) * 2 - 1).xyz;
	}

	sum_ripplenormal = normalize(sum_ripplenormal);
	vec3 ripple_normal_world = mat3(binormal, tangent, normal) * sum_ripplenormal;

	vec3 view = normalize(camera_position - fs_in.world_position);

	float facing = 1- max(dot(view, normal), 0);

	//Add reflection mapping.
	vec3 reflection_dir = reflect(-view, ripple_normal_world);

	vec4 reflection_color = texture(reflectioncube, reflection_dir);
	vec4 wave_color = mix(color_deep, color_shallow, facing);
	frag_color = wave_color + reflection_color;
}
