/*////////////////////////////////////////////////////////////////////////////*/

Texture2D    u_texture;
SamplerState u_sampler;

cbuffer ImGui: register(b0)
{
    float4x4 u_projection;
};

struct VSInput
{
    float2 position : POSITION;
    float2 texcoord : TEXCOORD;
    float4 color    : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
    float4 color    : COLOR;
};

PSInput vs_main(VSInput input)
{
    PSInput output;
    output.position = mul(u_projection, float4(input.position,0,1));
    output.texcoord = input.texcoord;
    output.color = input.color;
    return output;
}

float4 ps_main(PSInput input) : SV_TARGET
{
    float4 frag_color = input.color * u_texture.Sample(u_sampler, input.texcoord);
    return frag_color;
}

/*////////////////////////////////////////////////////////////////////////////*/
