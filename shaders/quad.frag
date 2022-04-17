//glsl version 4.5
#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;

layout(set = 0, binding = 1) uniform GlobalData
{   
	vec4 time;
} globalData;

layout( set = 2, binding = 0) uniform sampler2D imgTexture;

//output write
layout (location = 0) out vec4 outFragColor;

void main()
{
	//outFragColor = vec4(inColor, 1.0f);

	vec3 color = texture(imgTexture, inUV).xyz + globalData.time.x;
	outFragColor = vec4(color, 1.0f);
}