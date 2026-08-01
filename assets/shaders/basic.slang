struct VertexInput {
    [vk::location(0)]
    float3 Position;

    [vk::location(1)]
    float4 Color;
};

struct VertexOutput {
    float4 Position : SV_Position;

    [vk::location(0)]
    float4 Color;
};

struct CameraUniformData {
    float4x4 View;
    float4x4 Projection;
}

struct MeshPushConstants {
    float4x4 ModelMatrix;
};

[[vk::binding(0, 0)]]
ConstantBuffer<CameraUniformData> Camera;

[[vk::push_constant]]
ConstantBuffer<MeshPushConstants> PushConstants;

[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;

    float4 localPosition = float4(input.Position, 1.0);
    float4 worldPosition = mul(PushConstants.ModelMatrix, localPosition);
    float4 viewPosition = mul(Camera.View, worldPosition);
    output.Position = mul(Camera.Projection, viewPosition);
    output.Color = input.Color;

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target {
    return input.Color;
}
