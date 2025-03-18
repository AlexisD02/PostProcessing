//--------------------------------------------------------------------------------------
// Game Boy Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
// Applies a pixelation effect and a limited color palette to simulate a Game Boy display.
//--------------------------------------------------------------------------------------

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures & Samplers
//--------------------------------------------------------------------------------------
Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0); // No filtering (point sampling for pixelated effect)

//--------------------------------------------------------------------------------------
// Helper Functions
//--------------------------------------------------------------------------------------
// Converts an RGB colour to grayscale using luminance factors
float ConvertToGrayscale(float3 colour)
{
    return 0.299f * colour.r + 0.587f * colour.g + 0.114f * colour.b;
}

//--------------------------------------------------------------------------------------
// Shader Code
//--------------------------------------------------------------------------------------

float4 main(PostProcessingInput input) : SV_Target
{
    // Compute pixelated UV coordinates
    float2 pixelatedUV;
    pixelatedUV.x = floor(input.sceneUV.x * gGameBoyPixelSize) / gGameBoyPixelSize;
    pixelatedUV.y = floor(input.sceneUV.y * gGameBoyPixelSize) / gGameBoyPixelSize;

    // Sample color from the scene texture
    float3 sampledColour = SceneTexture.Sample(PointSample, pixelatedUV).rgb;

    // Convert to grayscale with limited color depth
    float grayscale = ConvertToGrayscale(sampledColour);
    grayscale = round(grayscale * gGameBoyColourDepth) / gGameBoyColourDepth;

    // Apply the Game Boy color tint
    float3 outputColour = grayscale * gGameBoyColour;

    return float4(outputColour, 1.0f);
}
