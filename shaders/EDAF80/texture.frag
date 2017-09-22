#version 410

uniform sampler2D sample_texture;

in VS_OUT {
	vec2 texcoord;
} fs_in;

out vec4 frag_color;

void main()
{
	frag_color = texture(sample_texture, fs_in.texcoord);
	//frag_color = vec4(fs_in.texcoord.x, fs_in.texcoord.y, 0.0, 1.0);
	
}
