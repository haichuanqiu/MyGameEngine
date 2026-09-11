#version 450 core

layout(location = 0) out vec4 FragColor;

uniform vec4 u_Color;

struct PointLight
{
    vec4 position;
    vec4 colorIntensity;
    vec4 params;  // x = range
};

struct SpotLight
{
    vec4 position;
    vec4 direction;
    vec4 colorIntensity;
    vec4 params;  // x=innerCone y=outerCone z=range
};

struct DirectionalLight
{
    vec4 direction;
    vec4 colorIntensity;
};

layout(std140, binding = 1) uniform LightBlock
{
    PointLight pointLights[10];
    SpotLight spotLights[10];
    DirectionalLight directionalLights[10];
    vec4 ambientColor;

    int pointLightCount;
    int spotLightCount;
    int directionalLightCount;
    int padding;
};

layout(std140, binding = 0) uniform CameraBlock
{
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_ViewProjection;
    vec4 u_CameraPosition;
};

in vec3 v_WorldPosition;

// ------------------------------------------------------------
// 点光源漫反射
// ------------------------------------------------------------
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos)
{
    vec3 lightDir = light.position.xyz - fragPos;
    float dist = length(lightDir);
    lightDir = normalize(lightDir);

    float NdotL = max(dot(normal, lightDir), 0.0);

    // 距离衰减（可以用 range 控制）
    float attenuation = clamp(1.0 - dist / light.params.x, 0.0, 1.0);
    attenuation *= attenuation;

    return light.colorIntensity.rgb * light.colorIntensity.a * NdotL * attenuation;
}

// ------------------------------------------------------------
// 聚光灯漫反射
// ------------------------------------------------------------
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos)
{
    vec3 lightDir = light.position.xyz - fragPos;
    float dist = length(lightDir);
    lightDir = normalize(lightDir);

    // 聚光灯锥角
    float theta = dot(lightDir, normalize(-light.direction.xyz));
    float epsilon = light.params.x - light.params.y;
    float intensity = clamp((theta - light.params.y) / epsilon, 0.0, 1.0);

    float NdotL = max(dot(normal, lightDir), 0.0);

    float attenuation = clamp(1.0 - dist / light.params.z, 0.0, 1.0);
    attenuation *= attenuation;

    return light.colorIntensity.rgb * light.colorIntensity.a * NdotL * attenuation * intensity;
}

// ------------------------------------------------------------
// 方向光漫反射
// ------------------------------------------------------------
vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal)
{
    vec3 lightDir = normalize(-light.direction.xyz);
    float NdotL = max(dot(normal, lightDir), 0.0);
    return light.colorIntensity.rgb * light.colorIntensity.a * NdotL;
}

void main()
{
    // 屏幕空间法线（简单方案，最好改用顶点法线）
    vec3 normal = normalize(
        cross(dFdx(v_WorldPosition), dFdy(v_WorldPosition))
    );

    // 环境光 × 物体颜色
    vec3 result = ambientColor.rgb * u_Color.rgb;

    // 所有点光源
    for (int i = 0; i < pointLightCount; i++)
    {
        result += CalcPointLight(pointLights[i], normal, v_WorldPosition) * u_Color.rgb;
    }

    // 所有聚光灯
    for (int i = 0; i < spotLightCount; i++)
    {
        result += CalcSpotLight(spotLights[i], normal, v_WorldPosition) * u_Color.rgb;
    }

    // 所有方向光
    for (int i = 0; i < directionalLightCount; i++)
    {
        result += CalcDirectionalLight(directionalLights[i], normal) * u_Color.rgb;
    }

    FragColor = vec4(result, 1.0);
}