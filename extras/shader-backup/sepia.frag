#version 450 core

layout(location = 0)out vec4 fragColor;

layout(location = 0)in VS_OUT
{
	vec4 Color;
	vec3 TexCoords;
} fs_in;

layout(binding = 0)uniform sampler2D texture0;

void main()
{
	vec4 color = texture(texture0, fs_in.TexCoords.xy);
	float r = color.r;
	float g = color.g;
	float b = color.b;
	
	color.r = min(1.0, (r * (1.0 - (0.607 * 1))) + (g * (0.769 * 1)) + (b * (0.189 * 1)));
	color.g = min(1.0, (r * 0.349 * 1) + (g * (1.0 - (0.314 * 1))) + (b * 0.168 * 1));
	color.b = min(1.0, (r * 0.272 * 1) + (g * 0.534 * 1) + (b * (1.0 - (0.869 * 1))));
	
	fragColor = color;
	
	//fragColor = texture(texture0, fs_in.TexCoords.xy) * fs_in.Color;
}