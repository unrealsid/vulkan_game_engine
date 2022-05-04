//glsl version 4.5
#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;

layout(set = 3, binding = 0) uniform GlobalData
{   
	vec4 time;
	vec4 Idx;
} globalData;

layout( set = 2, binding = 0) uniform sampler thisSampler;
layout( set = 2, binding = 1) uniform texture2D textures[8];

layout( push_constant ) uniform perObjectBlock 
{
  int imgID;
} PushConstant;

//output write
layout (location = 0) out vec4 outFragColor;

void main()
{
	//outFragColor = vec4(inColor, 1.0f);

	//vec3 color = texture(imgTexture, inUV).xyz + globalData.time.x;

	vec3 color = vec3(1.0f);
	outFragColor = texture(sampler2D(textures[int(globalData.Idx.x)], thisSampler), inUV);
}