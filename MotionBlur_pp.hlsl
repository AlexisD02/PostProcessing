//--------------------------------------------------------------------------------------
// Motion Blur Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
// Blends current frame with previous frame for motion blur effect

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

Texture2D SceneTexture : register(t0);
Texture2D PreviousFrameTexture : register(t1); // The previous frame's back buffer
SamplerState PointSample : register(s0); // Using point sampling for both textures

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

float4 main(PostProcessingInput input) : SV_Target
{
    // Sample the current frame
    float3 currentColour = SceneTexture.Sample(PointSample, input.sceneUV).rgb;
    
    // Sample the previous frame
    float3 previousColour = PreviousFrameTexture.Sample(PointSample, input.sceneUV).rgb;
    
    // Blend the current and previous frames with a blend factor
    float3 outputColour = lerp(currentColour, previousColour, gBlendFactor);
    
    return float4(outputColour, 1.0f);
}