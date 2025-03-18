//--------------------------------------------------------------------------------------
// Bloom Shader
//--------------------------------------------------------------------------------------

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Scene (original)
Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0);

// Bloom-blurred texture
Texture2D BloomTexture : register(t1);
SamplerState LinearSample : register(s1);

// Lens Star texture
Texture2D StarTexture : register(t2);
SamplerState LinearSample2 : register(s2);

//--------------------------------------------------------------------------------------
// Shader Code
//--------------------------------------------------------------------------------------

float4 main(PostProcessingInput input) : SV_Target
{
    // Sample the original scene
    float3 sceneColour = SceneTexture.Sample(PointSample, input.sceneUV).rgb;

    // Sample the bloom texture
    float3 bloomColour = BloomTexture.Sample(LinearSample, input.sceneUV).rgb;

    // Sample the star effect texture
    float3 starColour = StarTexture.Sample(LinearSample2, input.sceneUV).rgb;

    // Combine bloom and star effect (Additive Blending)
    float3 outputColour = sceneColour + bloomColour * gBloomIntensity + starColour * gStarIntensity;

    return float4(outputColour, 1.0f);
}
