//--------------------------------------------------------------------------------------
// Invert Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------

#include "Common.hlsli"

// Scene texture and sampler
Texture2D SceneTexture : register(t0);
SamplerState LinearSample : register(s0);

// Shader main function
float4 main(PostProcessingInput input) : SV_Target
{
    // Sample the current pixel colour
    float3 colour = SceneTexture.Sample(LinearSample, input.sceneUV).rgb;

    // Invert colours
    float3 outputColour = 1.0f - colour;

    return float4(outputColour, 1.0f);
}
