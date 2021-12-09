// static.vert文件
// This shader can be used to display static geometry or CPU skinned meshes
#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 position;
in vec3 normal;
in vec2 texCoord;

out vec3 norm;
out vec3 fragPos;
out vec2 uv;

void main() 
{
	// 注意, 这里的mvp是以uniform的形式存在的, 每个顶点的MVP是一样的
    gl_Position = projection * view * model * vec4(position, 1.0);
    
    fragPos = vec3(model * vec4(position, 1.0));
    norm = vec3(model * vec4(normal, 0.0f));
    uv = texCoord;
}


