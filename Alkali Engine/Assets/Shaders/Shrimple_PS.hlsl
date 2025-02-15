struct V_OUT
{    
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
};

float4 main(V_OUT input) : SV_Target
{
    return float4(abs(input.Normal), 1.0f);
}