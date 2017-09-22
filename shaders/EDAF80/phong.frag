#version 410

uniform vec3 light_position;
uniform vec3 camera_position;
uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;


in VS_OUT {
	vec3 vertex;
	vec3 normal;
} fs_in;

out vec4 frag_color;

void main()
{
	vec3 light = normalize(light_position - fs_in.vertex);
	vec3 normal = normalize(fs_in.normal);
	
	vec3 diffuse_color = max(diffuse*dot(light, normal), vec3(0,0,0));
	
	vec3 view_vector = normalize(camera_position - fs_in.vertex);
	
	vec3 specular_color = max(specular * pow(dot(reflect(-light, normal), view_vector), shininess),vec3(0,0,0));
	
	frag_color = vec4((ambient + diffuse_color + specular_color), 1.0f);
}
