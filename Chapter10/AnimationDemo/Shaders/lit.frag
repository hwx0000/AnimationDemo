// lit.frag文件
// Some chapters will introduce new vertex shaders, but the fragment shader is always going to remain as this one.
#version 330 core

in vec3 norm;
in vec3 fragPos;
in vec2 uv;

uniform vec3 light;  
uniform sampler2D tex0;

out vec4 FragColor;

void main()
 {
	vec4 diffuseColor = texture(tex0, uv);

	vec3 n = normalize(norm);
	vec3 l = normalize(light);
	float diffuseIntensity = clamp(dot(n, l) + 0.1, 0, 1);

	// 这里的输出颜色只跟贴图中得到的光照颜色, 以及法线与光线的夹角有关
	// 没有环境光和高光
	FragColor = diffuseColor * diffuseIntensity;
}
