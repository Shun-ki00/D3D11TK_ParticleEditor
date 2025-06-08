#include "Particle.hlsli"

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;

    output.positionCS = input.positionOS;

    output.color = input.color;
    
    output.uv = input.uv;
    
    return output;
}