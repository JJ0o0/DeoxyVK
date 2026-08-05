struct VertexInput {
    [vk::location(0)] float3 Position;
    [vk::location(1)] float4 Color;
    [vk::location(2)] float2 UV;
    [vk::location(3)] float3 Normal;
};

struct VertexOutput {
    float4 Position : SV_Position;

    [vk::location(0)] float4 Color;
    [vk::location(1)] float2 UV;
    [vk::location(2)] float3 Normal;
};

struct DirectionalLightUniformData {
    float3 Direction;
    float Intensity;
    float4 LightColor;
}

struct FrameUniformData {
    float4x4 View;
    float4x4 Projection;

    DirectionalLightUniformData DirectionalLight;
};

struct PushConstantsData {
    float4x4 ModelMatrix;
    float4 MaterialTint;
    float2 MaterialUVScale;
    float2 MaterialUVOffset;
};

[[vk::binding(0, 0)]] ConstantBuffer<FrameUniformData> FrameData;
[[vk::binding(0, 1)]] Sampler2D<float4> AlbedoTexture;

[[vk::push_constant]] ConstantBuffer<PushConstantsData> PushConstants;

[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;

    float4 localPosition = float4(input.Position, 1.0);
    float4 worldPosition = mul(PushConstants.ModelMatrix, localPosition);
    float4 viewPosition = mul(FrameData.View, worldPosition);

    output.Position = mul(FrameData.Projection, viewPosition);
    output.Color = input.Color;
    output.UV = input.UV;

    // TEMPORÁRIO!
    // Escalas não uniformes (tipo: {1.0f, 2.0f, 0.5f}) vão ficar com iluminação estranha!
    float3x3 modelRotScale = (float3x3)PushConstants.ModelMatrix;
    output.Normal = normalize(mul(modelRotScale, input.Normal));

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target {
    float2 scaledUV = input.UV * PushConstants.MaterialUVScale + PushConstants.MaterialUVOffset;
    float4 textureColor = AlbedoTexture.Sample(scaledUV);

    float3 normal = normalize(input.Normal);
    float3 lightDir = normalize(-FrameData.DirectionalLight.Direction);

    float3 ambient = float3(0.05);

    float3 diffuseStrength = FrameData.DirectionalLight.LightColor.rgb * FrameData.DirectionalLight.Intensity;
    float3 diffuse = max(dot(normal, lightDir), 0.0) * diffuseStrength;

    float3 lighting = ambient + diffuse;

    float4 albedo = textureColor * input.Color * PushConstants.MaterialTint;
    return float4(albedo.rgb * lighting, albedo.a);
}
