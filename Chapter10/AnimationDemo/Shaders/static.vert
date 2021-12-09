#version 330 core
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 position;
in vec3 normal;
in vec2 texCoord;
in vec4 weights;// 额外的顶点属性1
in ivec4 joints;// 额外的顶点属性2

// 两个Uniform数组
uniform mat4 pose[120];	// 代表parent joint的world trans
uniform mat4 invBindPose[120];//代表转换到parent joint的local space的offset矩阵

// 这里是重点，对于Skinned Mesh，其ModelSpace下的顶点坐标和法向量都需要重新计算
// 因为这个Mesh变化了
out vec4 newModelPos;
out vec3 newNorm;
out vec2 uv;				// 注意，uv是不需要变化的(为啥？应该是Deformation只改顶点位置和法向量, 不改这些)

void main()
{
	mat m0 = pose[joints.x] * invBindPose[joints.x] * weights.x;
	mat m1 = pose[joints.y] * invBindPose[joints.y] * weights.y;
	mat m2 = pose[joints.z] * invBindPose[joints.z] * weights.z;
	mat m3 = pose[joints.w] * invBindPose[joints.w] * weights.w;
	mat4 pallete = m0 + m1 + m2 + m3;
	
	gl_Position = projection * view * model * pallete * position;// 算出屏幕上的样子

	// 计算真实的ModelSpace下的坐标，供后续进行着色和光照计算
	newModelPos =  (model * pallete * vec4(position, 1.0f)).xyz;
	newNorm = (model * pallete * vec4(normal, 0.0f)).xyz;

	// 注意，顶点即使进行了Deformation，但它对应贴图的TexCoord是不改变的
	uv =  texCoord;
}
