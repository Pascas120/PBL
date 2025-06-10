#version 330 core

uniform sampler2D colorTexture;
uniform sampler2D ssaoTexture;


uniform int width;
uniform int height;

in vec2 texCoord;
out vec4 fragColor;

void main()
{
	vec4 color = texture(colorTexture, texCoord);
	vec4 ssao = texture(ssaoTexture, texCoord);
	fragColor = vec4(color.rgb * ssao.r, color.a);
	//fragColor = vec4(ssao.rrr, 1.0); // Uncomment to see SSAO effect alone
	//fragColor = vec4(color.rgb, 1.0f);
}