//--------------------------------------------------------------------------------------
// Full-screen Blur Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
// Applies a simple box blur to the scene texture.
// The shader samples a 3x3 neighborhood around each pixel and averages the result.
//--------------------------------------------------------------------------------------

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// The rendered scene is stored in a texture.
Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0); // Use point sampling for sharp texel fetches.

//--------------------------------------------------------------------------------------
// Shader Code
//--------------------------------------------------------------------------------------

// The input structure (PostProcessingInput) is defined in Common.hlsli.
// It provides at least the sceneUV coordinates for sampling the scene texture.
float4 main(PostProcessingInput input) : SV_Target
{    
    // Initialize the output colour to zero.
    float3 blurredColour = float3(0.0f, 0.0f, 0.0f);
    
    // Sample a 3x3 grid around the current pixel.
    blurredColour += SceneTexture.Sample(PointSample, input.sceneUV + float2(-gTexelSize.x, -gTexelSize.y));
    blurredColour += SceneTexture.Sample(PointSample, input.sceneUV + float2(0.0f, -gTexelSize.y));
    blurredColour += SceneTexture.Sample(PointSample, input.sceneUV + float2(gTexelSize.x, -gTexelSize.y));
    
    blurredColour += SceneTexture.Sample(PointSample, input.sceneUV + float2(-gTexelSize.x, 0.0f));
    blurredColour += SceneTexture.Sample(PointSample, input.sceneUV);
    blurredColour += SceneTexture.Sample(PointSample, input.sceneUV + float2(gTexelSize.x, 0.0f));
    
    blurredColour += SceneTexture.Sample(PointSample, input.sceneUV + float2(-gTexelSize.x, gTexelSize.y));
    blurredColour += SceneTexture.Sample(PointSample, input.sceneUV + float2(0.0f, gTexelSize.y));
    blurredColour += SceneTexture.Sample(PointSample, input.sceneUV + float2(gTexelSize.x, gTexelSize.y));
    
    // Average the 9 samples.
    blurredColour /= 9.0;
        
    return float4(blurredColour, 1.0f);
}
