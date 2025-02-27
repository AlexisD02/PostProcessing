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
    const int numTaps = 5;
    const float weights[numTaps] = { 0.06136f, 0.24477f, 0.38774f, 0.24477f, 0.06136f };
    const float offsets[numTaps] = { -2.0f, -1.0f, 0.0f, 1.0f, 2.0f };

    float3 outputColour = float3(0.0f, 0.0f, 0.0f);

    // Loop through each tap
    for (unsigned int i = 0; i < numTaps; i++)
    {
        // We only move vertically in this pass.
        float2 offset = float2(0.0f, offsets[i] * gBlurStrength * gTexelSize.y);
        
        outputColour += SceneTexture.Sample(PointSample, input.sceneUV + offset).rgb * weights[i];
    }

    return float4(outputColour, 1.0f);
}
