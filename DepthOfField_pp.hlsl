//--------------------------------------------------------------------------------------
// Depth of Field Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
// Applies a depth of field effect: models at the focal distance are sharp, while nearer or further models are blurred

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------

static const float TAU = 6.28318530717958647692f;
static const float GAMMA = 2.2f;

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

Texture2D SceneTexture : register(t0); // Color texture of the rendered scene
SamplerState PointSample : register(s0); // Point sampling for color texture

Texture2D DepthTexture : register(t1); // Depth texture of the rendered scene
SamplerState LinearSample : register(s1); // Linear sampling for depth interpolation

//--------------------------------------------------------------------------------------
// Helper Functions
//--------------------------------------------------------------------------------------

// Correctly linearize depth from non-linear depth buffer to linear distance
float LinearizeDepth(float depth)
{
    return (gNearClip * gFarClip) / (gFarClip - depth * (gFarClip - gNearClip));
}

// Gaussian blur in linear colour space
float3 Blur(float2 uv, float blurRadius)
{
    int halfKernel = clamp(int(ceil(blurRadius * 2.0f)), 2, 8);
    float sigma = max(blurRadius, 0.001f);
    float3 colourAccum = float3(0.0f, 0.0f, 0.0f);
    float weightAccum = 0.0f;
    
    // Loop over a square region
    for (int y = -halfKernel; y <= halfKernel; ++y)
    {
        for (int x = -halfKernel; x <= halfKernel; ++x)
        {
            // Distance^2 from the center
            float dist2 = (float) (x * x + y * y);

            // Gaussian weight for this offset
            float w = exp(-dist2 / (2.0f * sigma * sigma));

            // Convert (x,y) to a UV offset in screen space
            // Multiplying offset by 'blurRadius' ensures that bigger radius
            // means sampling further out from the center.
            float2 offsetUV = float2(x, y) * gTexelSize * blurRadius;

            // Sample and accumulate
            float3 sampleColor = SceneTexture.Sample(LinearSample, uv + offsetUV).rgb;
            colourAccum += sampleColor * w;
            weightAccum += w;
        }
    }

    // Normalize by total weight
    return colourAccum / max(weightAccum, 1e-5);
}

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

float4 main(PostProcessingInput input) : SV_Target
{
    // Sample colour and depth
    float3 outputColour = SceneTexture.Sample(PointSample, input.sceneUV).rgb;
    float depth = DepthTexture.Sample(PointSample, input.sceneUV).r;
    float linearDepth = LinearizeDepth(depth);

    // Calculate blur radius based on depth difference and distance
    float depthDifference = abs(linearDepth - gFocalDistance);
    float blurAmount = (gAperture * depthDifference) / linearDepth;
    
    if (blurAmount > 0.01f)
    {
        outputColour = Blur(input.sceneUV, blurAmount * 0.5f);
    }

    return float4(outputColour, 1.0f);
}