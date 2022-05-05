//glsl version 4.5
#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;

layout( set = 2, binding = 0) uniform sampler thisSampler;
layout( set = 2, binding = 1) uniform texture2D textures[8];

layout(set = 3, binding = 0) uniform GlobalData
{   
	vec4 time;
	int idx;
	
} globalData;

layout( push_constant ) uniform perObjectBlock 
{
  int imgID;
} PushConstant;

//output write
layout (location = 0) out vec4 outFragColor;

void main()
{
	vec3 color = vec3(1.0f);
	float x = globalData.time.x;
	outFragColor = texture(sampler2D(textures[globalData.idx], thisSampler), inUV);
}