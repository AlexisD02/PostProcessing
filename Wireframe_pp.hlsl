//--------------------------------------------------------------------------------------
// Wireframe Post-Processing Pixel Shader (Sobel Edge Detection)
//--------------------------------------------------------------------------------------
// Detects edges using Sobel operator and draws white wireframe lines on black background

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------

Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0);

//--------------------------------------------------------------------------------------
// Main Shader code
//--------------------------------------------------------------------------------------

float3 SampleGray(float2 uv)
{
    float3 color = SceneTexture.Sample(PointSample, uv).rgb;
    return dot(color, float3(0.299f, 0.587f, 0.114f)); // Luminance conversion
}

float SobelEdgeDetect(float2 uv)
{
    // Sobel kernels
    const int numKernel = 9;
    const float kernelX[numKernel] = { -1.0f, 0.0f, 1.0f, -2.0f, 0.0f, 2.0f, -1.0f, 0.0f, 1.0f };
    const float kernelY[numKernel] = { -1.0f, -2.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 1.0f };
    
    float gradientX = 0.0f;
    float gradientY = 0.0f;
    int index = 0;
    
    // Sample 3x3 grid
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float2 offset = float2(x, y) * gTexelSize;
            float gray = SampleGray(uv + offset);
            gradientX += gray * kernelX[index];
            gradientY += gray * kernelY[index];
            index++;
        }
    }
    
    // Calculate edge strength
    return sqrt(gradientX * gradientX + gradientY * gradientY);
}

float4 main(PostProcessingInput input) : SV_Target
{
    // Calculate edge strength
    float edge = SobelEdgeDetect(input.sceneUV);
    
    // Enhance and threshold edges
    edge = saturate(pow(edge, gEdgePower) * (1.0f / gEdgeThreshold));
    
    // Create white-on-black wireframe effect
    float3 outputColour = lerp(float3(0.0f, 0.0f, 0.0f), float3(1.0f, 1.0f, 1.0f), edge);
    
    return float4(outputColour, 1.0f);
}