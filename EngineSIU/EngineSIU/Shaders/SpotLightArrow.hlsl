
cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 Model;
    row_major float4x4 View;
    row_major float4x4 Projection;
    float4 ConstantColor;
}

struct VS_INPUT
{
    float3 position : POSITION; 
    float4 color : COLOR; 
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};


PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT vsoutput;
    
    float4 Position = mul(float4(input.position, 1), Model);
    Position = mul(Position, View);
    Position = mul(Position, Projection);
    
    vsoutput.position = Position;
    vsoutput.color = input.color;
    
    return vsoutput;
}

float4 mainPS(PS_INPUT input) : SV_Target
{
    return ConstantColor;
    //return input.color;   
}
