//--------------------------------------------------------------------------------------
// Vertical Blur Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
// The shader samples a set of texel offsets, weighted according to a Gaussian function.
//--------------------------------------------------------------------------------------

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// The rendered scene is stored in a texture.
Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0);

//--------------------------------------------------------------------------------------
// Shader Code
//--------------------------------------------------------------------------------------

float4 main(PostProcessingInput input) : SV_Target
{
    /*float3 outputColour = float3(0.0f, 0.0f, 0.0f);

    // Loop through each tap
    for (int i = 0; i < NUM_TAPS; i++)
    {
        // We only move vertically in this pass.
        float2 offset = float2(0.0, gOffsets[i] * gTexelSize.y);

        outputColour += SceneTexture.Sample(PointSample, input.sceneUV + offset) * gWeights[i];
    }

    return float4(outputColour, 1.0f);*/
    
    return float4(0.0f, 0.0f, 0.0f, 1.0f);
}
