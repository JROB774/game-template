/*////////////////////////////////////////////////////////////////////////////*/

Texture2D    u_texture;
SamplerState u_sampler;

cbuffer Imm: register(b0)
{
    float4x4 u_projection;
    float4x4 u_view;
    float4x4 u_model;
    bool     u_usetex;
};

struct VSInput
{
    float4 position  : POSITION;
    float4 normal    : NORMAL;
    float4 color     : COLOR;
    float4 texcoord  : TEXCOORD;
    float4 userdata0 : USERDATA0;
    float4 userdata1 : USERDATA1;
    float4 userdata2 : USERDATA2;
    float4 userdata3 : USERDATA3;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
    float2 texcoord : TEXCOORD;
};

PSInput vs_main(VSInput input)
{
    PSInput output;
    output.position = mul(u_projection, mul(u_view, mul(u_model, input.position)));
    output.color = input.color;
    output.texcoord = input.texcoord.xy;
    return output;
}

float4 ps_main(PSInput input) : SV_TARGET
{
    float4 frag_color = input.color;
    if(u_usetex)
        frag_color *= u_texture.Sample(u_sampler, input.texcoord);
    return frag_color;
}

/*////////////////////////////////////////////////////////////////////////////*/
