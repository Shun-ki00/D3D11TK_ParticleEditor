#include "Particle.hlsli"

Texture2D tex : register(t0);
Texture2D tex2 : register(t1);
SamplerState samLinear : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
	//	‰æ‘œ•\¦
    float4 output = tex.Sample(samLinear, input.uv);

    float4 output2 = tex2.Sample(samLinear, input.uv);


	//	^‚Á”’‚È”Âƒ|ƒŠƒSƒ“
    float4 outputw = float4(1.0f, 1.0f, 1.0f, 1.0f);


    return outputw;
}
