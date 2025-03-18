//--------------------------------------------------------------------------------------
// Dilation Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
// Expands bright areas by finding the maximum color in a neighborhood (a naive approach).
//--------------------------------------------------------------------------------------

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures & Samplers
//--------------------------------------------------------------------------------------

// Scene has been rendered to a texture; these variables access that texture.
// Matches the style in the provided colour tint example.
Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0);

//--------------------------------------------------------------------------------------
// Main Shader Code
//--------------------------------------------------------------------------------------
float4 main(PostProcessingInput input) : SV_Target
{
    // The size in UV space for a single pixel.
    float2 pixelSize = float2(1.0f / gViewportWidth, 1.0f / gViewportHeight);

    float3 outputColour = float3(0.0f, 0.0f, 0.0f);

    // Loop over a square region around the current pixel
    for (int y = -gKernelRadius; y <= gKernelRadius; y++)
    {
        for (int x = -gKernelRadius; x <= gKernelRadius; x++)
        {
            // Offset in UV space
            float2 offset = float2(x, y) * pixelSize;
            float2 sampleUV = input.sceneUV + offset;

            // Sample the scene texture
            float3 sampleColour = SceneTexture.Sample(PointSample, sampleUV).rgb;

            // Track the maximum color found
            outputColour = max(outputColour, sampleColour);
        }
    }

    // Return the brightest color found in the neighborhood
    return float4(outputColour, 1.0f);
}
