//--------------------------------------------------------------------------------------
// Vertical Gradient Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
// This shader applies a full-screen vertical color gradient tint to the scene texture.
//--------------------------------------------------------------------------------------

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures and Sampler
//--------------------------------------------------------------------------------------

// The scene texture is the render target that contains the rendered scene.
Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0); // Use point sampling for post-processing.

//--------------------------------------------------------------------------------------
// Main Shader Function
//--------------------------------------------------------------------------------------
float4 main(PostProcessingInput input) : SV_Target
{
    // Sample the original scene texture at the provided UV coordinate.
    float3 sceneColour = SceneTexture.Sample(PointSample, input.sceneUV).rgb;
    
    // Use the vertical coordinate from areaUV (0 at top, 1 at bottom).
    float3 gradientTint = lerp(gTopColour, gBottomColour, saturate(input.areaUV.y));
    
    // Apply the gradient tint by multiplying it with the scene colour.
    float3 outputColour = sceneColour * gradientTint;
    
    return float4(outputColour, 1.0f);
}
