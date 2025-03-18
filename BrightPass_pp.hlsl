//--------------------------------------------------------------------------------------
// BrightPass.hlsl
//--------------------------------------------------------------------------------------
#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

Texture2D gSceneTexture : register(t0);
SamplerState gLinearSampler : register(s0);

//--------------------------------------------------------------------------------------
// Shader Code
//--------------------------------------------------------------------------------------

float4 main(PostProcessingInput input) : SV_Target
{    
    // Read and adjust scene color with exposure
    float3 sampleColour = gSceneTexture.Sample(gLinearSampler, input.sceneUV).rgb;
    sampleColour *= gExposure;
    
    // Calculate perceptual luminance
    float luminance = dot(sampleColour, float3(0.2126f, 0.7152f, 0.0722f));
    
    // Smooth threshold with soft knee transition
    float soft = smoothstep(gBloomThreshold, gBloomThreshold + gBloomKnee, luminance);
    float3 outputColour = sampleColour * soft;
    
    return float4(outputColour, 1.0f);
}