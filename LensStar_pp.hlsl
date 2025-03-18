//--------------------------------------------------------------------------------------
// LightStreaks
//--------------------------------------------------------------------------------------
#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

Texture2D BrightPassTexture : register(t0);
SamplerState LinearSampler : register(s0);

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------

static const int numSamples = 6;

// Light streak parameters
static const float2 gStreakDirections[numSamples] =
{
    float2(1.0f, 0.0f), // Horizontal
    float2(0.0f, 1.0f), // Vertical
    float2(1.0f, 1.0f), // Diagonal 1
    float2(-1.0f, 1.0f), // Diagonal 2
    float2(2.0f, 1.0f), // Shallow Diagonal 1
    float2(-2.0f, 1.0f) // Shallow Diagonal 2
};

static const float3 gStreakColours[numSamples] =
{
    float3(1.0, 0.6, 0.3), // Warm horizontal
    float3(0.3, 1.0, 0.6), // Cool vertical
    float3(1.0, 0.8, 0.5), // Golden diagonal 1
    float3(0.5, 0.8, 1.0), // Cool diagonal 2
    float3(1.0, 0.4, 0.2), // Deep orange shallow 1
    float3(0.2, 0.4, 1.0) // Deep blue shallow 2
};

//--------------------------------------------------------------------------------------
// Shader Code
//--------------------------------------------------------------------------------------

float3 CalculateStreak(float2 direction, float3 colourTint, float2 uv)
{    
    float3 colour = float3(0.0f, 0.0f, 0.0f);
    float weightSum = 0.0f;
    float2 dir = normalize(direction) * gStepSize;
    
    for (int i = -numSamples; i <= numSamples; ++i)
    {
        if (i == 0) continue; // Skip center
        
        float2 offset = dir * i;
        float3 sampleColour = BrightPassTexture.Sample(LinearSampler, uv + offset).rgb;
        
        // Distance-based attenuation with color tinting
        float weight = pow(gAttenuation, abs(i));
        colour += sampleColour * weight * colourTint;
        weightSum += weight;
    }
    
    return colour / weightSum;
}

float4 main(PostProcessingInput input) : SV_Target
{
    float3 totalStreaks = float3(0.0f, 0.0f, 0.0f);
    
    // Generate streaks in all directions
    for (unsigned int i = 0; i < 6; i++)
    {
        totalStreaks += CalculateStreak(gStreakDirections[i], gStreakColours[i], input.sceneUV);
    }

    // Apply brightness compression
    float3 outputColour = totalStreaks / (totalStreaks + 1.0f);
    
    return float4(outputColour, 1.0f);
}