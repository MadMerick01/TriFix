struct VertexInput
{
    float2 position : POSITION;
};

float4 main(VertexInput input) : SV_POSITION
{
    return float4(input.position, 0.0f, 1.0f);
}
