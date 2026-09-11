#version 450 core

layout(location = 0) in vec3 a_Position;

// Camera UBO
layout(std140, binding = 0) uniform CameraBlock
{
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_ViewProjection;
    vec4 u_CameraPosition;
};

// 每个 Renderer / Object 自己设置
uniform mat4 u_Model;

out vec3 v_WorldPosition;

void main()
{
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);

    v_WorldPosition = worldPos.xyz;

    gl_Position = u_ViewProjection * worldPos;
}