#version 410

uniform Sampler2d Sampler

in VS_OUT {
	vec2 texcoord;
} fs_in;

out vec4 frag_color;

void main()
{
	frag_color = texture(Sampler, texcoord);
}
