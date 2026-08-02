struct VertexInput {
    [vk::location(0)] float3 Position;
    [vk::location(1)] float4 Color;
    [vk::location(2)] float2 UV;
};

struct VertexOutput {
    float4 Position : SV_Position;

    [vk::location(0)] float4 Color;
    [vk::location(1)] float2 UV;
};

struct CameraUniformData {
    float4x4 View;
    float4x4 Projection;
};

struct PushConstantsData {
    float4x4 ModelMatrix;
    float4 MaterialTint;
};

[[vk::binding(0, 0)]] ConstantBuffer<CameraUniformData> Camera;
[[vk::binding(0, 1)]] Sampler2D<float4> AlbedoTexture;

[[vk::push_constant]] ConstantBuffer<PushConstantsData> PushConstants;

[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;

    float4 localPosition = float4(input.Position, 1.0);
    float4 worldPosition = mul(PushConstants.ModelMatrix, localPosition);
    float4 viewPosition = mul(Camera.View, worldPosition);
    output.Position = mul(Camera.Projection, viewPosition);
    output.Color = input.Color;
    output.UV = input.UV;

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target {
    float4 textureColor = AlbedoTexture.Sample(input.UV);
    return textureColor * input.Color * PushConstants.MaterialTint;
}
