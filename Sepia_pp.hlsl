//--------------------------------------------------------------------------------------
// Sepia Post-Processing Shader
//--------------------------------------------------------------------------------------
// Applies a classic sepia color transform to the sampled scene.

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures & Samplers
//--------------------------------------------------------------------------------------

Texture2D SceneTexture : register(t0);
SamplerState LinearSampler : register(s0);

//--------------------------------------------------------------------------------------
// Main Shader Code
//--------------------------------------------------------------------------------------

float4 main(PostProcessingInput input) : SV_Target
{
    // Sample the scene texture
    float4 sampledColour = SceneTexture.Sample(LinearSampler, input.sceneUV);

    // Apply the sepia colour 
    float3 outputColour = float3(0.0f, 0.0f, 0.0f);
    outputColour.r = dot(sampledColour.rgb, float3(0.393f, 0.769f, 0.189f));
    outputColour.g = dot(sampledColour.rgb, float3(0.349f, 0.686f, 0.168f));
    outputColour.b = dot(sampledColour.rgb, float3(0.272f, 0.534f, 0.131f));

    // Return the final colour (keep alpha as 1.0 unless you want to preserve original alpha)
    return float4(outputColour, 1.0f);
}
