#version 410

uniform samplerCube reflectioncube;
uniform sampler2D ripple_texture;

uniform vec3 camera_position;
uniform vec2 wind_direction;
uniform float time;
uniform float r_fresnel;

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
	float x_scale = 4;
	float y_scale = 2;

	vec2 texture_scale = vec2(x_scale, y_scale);
	float ripple_time = mod(time, 100.0);
	vec2 ripple_speed = 5 * wind_direction; //vec2(-0.05, 0);

	int num_ripples = 3;
	float d_x = x_scale/(num_ripples - 1), d_y = y_scale/(num_ripples - 1);

	vec2 sample_coords = fs_in.texcoord * texture_scale + ripple_time * ripple_speed;
	vec3 sum_ripplenormal = (texture(ripple_texture, sample_coords) * 2 - 1).xyz;

	for (int n = 1; n < num_ripples; ++n) {
		vec2 sample_coords = fs_in.texcoord * texture_scale * n * d_y + ripple_time * ripple_speed * n * d_x;
		sum_ripplenormal += (texture(ripple_texture, sample_coords) * 2 - 1).xyz;
	}

	sum_ripplenormal = normalize(sum_ripplenormal);
	vec3 ripple_normal_world = mat3(binormal, tangent, normal) * sum_ripplenormal;

	vec3 view = normalize(camera_position - fs_in.world_position);

	float facing = 1- max(dot(view, ripple_normal_world), 0);

	//Add reflection mapping.
	vec3 reflection_dir = reflect(-view, ripple_normal_world);

	//Add refraction mapping
	vec3 refraction_dir = refract(-view, ripple_normal_world, 1/1.33);

	// Add fresnel-type reflection/refraction
	float fresnel_factor = r_fresnel + (1 - r_fresnel) * pow((1 - dot(view, ripple_normal_world)), 5);

	vec4 reflection_color = texture(reflectioncube, reflection_dir);
	vec4 refraction_color = texture(reflectioncube, refraction_dir);
	vec4 wave_color = mix(color_deep, color_shallow, facing);

	frag_color = wave_color + reflection_color * fresnel_factor + refraction_color * (1 - fresnel_factor);
	//frag_color = wave_color;
}
