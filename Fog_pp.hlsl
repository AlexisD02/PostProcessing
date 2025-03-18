//--------------------------------------------------------------------------------------
// Fog Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
// Applies an atmospheric fog effect based on depth & height
#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------
Texture2D SceneTexture : register(t0);
SamplerState BilinearSample : register(s0);

Texture2D DepthTexture : register(t1);
SamplerState PointSample : register(s1);

//--------------------------------------------------------------------------------------
// Helper Functions
//--------------------------------------------------------------------------------------
// Convert depth from non-linear to linear space
float LinearizeDepth(float depth)
{
    return (gNearClip * gFarClip) / (gFarClip - depth * (gFarClip - gNearClip));
}

//--------------------------------------------------------------------------------------
// Main Shader Code
//--------------------------------------------------------------------------------------
float4 main(PostProcessingInput input) : SV_Target
{    
    // Sample scene texture
    float3 sampledColour = SceneTexture.Sample(BilinearSample, input.sceneUV).rgb;

    // Sample depth buffer
    float depth = DepthTexture.Sample(PointSample, input.sceneUV).r;
    float linearDepth = LinearizeDepth(depth);

    // Compute fog factor based on distance
    float fogFactor = exp(-linearDepth * gFogDensity);
    float heightFog = exp(-(linearDepth - gFogHeightStart) * gFogHeightDensity);
    heightFog = saturate(heightFog);

    // Combine distance fog and height fog
    float totalFog = saturate(1.0f - fogFactor * heightFog);

    // Blend fog color with scene color
    float3 outputColour = lerp(sampledColour, gFogColour, totalFog);

    return float4(outputColour, 1.0f);
}
