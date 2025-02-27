#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures & Samplers
//--------------------------------------------------------------------------------------
Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0);

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// Define limited color palette as an array of float3 values
static const float3 gColorPalette[] =
{
    float3(1.0f, 0.0f, 0.0f), // Red
    float3(0.0f, 1.0f, 0.0f), // Green
    float3(0.0f, 0.0f, 1.0f), // Blue
    float3(1.0f, 1.0f, 0.0f), // Yellow
    float3(0.0f, 1.0f, 1.0f), // Cyan
    float3(1.0f, 0.0f, 1.0f), // Magenta
    float3(1.0f, 1.0f, 1.0f), // White
    float3(0.75f, 0.75f, 0.75f), // Light Gray
    float3(1.0f, 0.65f, 0.0f), // Orange
    float3(0.75f, 0.5f, 0.75f), // Light Purple
    float3(0.5f, 0.75f, 0.75f), // Light Teal
    float3(0.75f, 0.75f, 0.5f), // Light Olive
    float3(1.0f, 0.87f, 0.68f), // Light Peach
    float3(0.68f, 0.85f, 0.9f), // Light Sky Blue
    float3(0.82f, 0.93f, 0.75f), // Light Green
    float3(1.0f, 0.8f, 0.8f), // Light Pink
    float3(0.5f, 0.0f, 0.0f), // Dark Red
    float3(0.0f, 0.5f, 0.0f), // Dark Green
    float3(0.0f, 0.0f, 0.5f), // Dark Blue
    float3(0.6f, 0.4f, 0.2f), // Brown
    float3(0.4f, 0.2f, 0.6f), // Deep Purple
    float3(0.2f, 0.6f, 0.4f), // Forest Green
    float3(0.9f, 0.4f, 0.6f), // Rose
    float3(0.95f, 0.9f, 0.1f), // Bright Yellow
    float3(0.1f, 0.7f, 0.9f) // Turquoise
};

//--------------------------------------------------------------------------------------
// Retro Game Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(PostProcessingInput input) : SV_Target
{
    // Snap the sceneUV to a coarse grid to produce the "big pixel" effect
    float2 blockUV = floor(input.sceneUV * gPixelSize) / gPixelSize;

    // Sample the scene using a point sampler to preserve crisp edges
    float3 sampledColour = SceneTexture.Sample(PointSample, blockUV).rgb;

    // Find the nearest colour in the palette
    float minDistance = 1e9;
    float3 nearestColour = float3(0.0f, 0.0f, 0.0f);
    for (unsigned int i = 0; i < gPaletteSize; i++)
    {
        float distance = length(sampledColour - gColorPalette[i]);
        if (distance < minDistance)
        {
            minDistance = distance;
            nearestColour = gColorPalette[i];
        }
    }

    return float4(nearestColour, 1.0f);
}
