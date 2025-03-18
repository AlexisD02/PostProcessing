//--------------------------------------------------------------------------------------
// Combined Night Vision Shader
//--------------------------------------------------------------------------------------
// Night vision approach accumulates color & applies a simple green tint,
// then adds noise, flicker, and a vignette effect.
//--------------------------------------------------------------------------------------

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures & Samplers
//--------------------------------------------------------------------------------------
Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0);
SamplerState LinearSample : register(s1);

//--------------------------------------------------------------------------------------
// Helper Functions
//--------------------------------------------------------------------------------------

// Computes average luminance
float ComputeLuminance(float3 colour)
{
    return (colour.r + colour.g + colour.b) / 3.0f;
}

// Random noise hash
float Hash(float n)
{
    return frac(sin(n) * 43758.5453123f);
}

//--------------------------------------------------------------------------------------
// Shader Code
//--------------------------------------------------------------------------------------

float4 main(PostProcessingInput input) : SV_Target
{
    // Accumulate color from multiple offsets
    float3 sampledColour = SceneTexture.Sample(PointSample, input.sceneUV).rgb;
    sampledColour += SceneTexture.Sample(PointSample, input.sceneUV + float2(0.001f, 0.001f)).rgb;
    sampledColour += SceneTexture.Sample(PointSample, input.sceneUV + float2(-0.002f, -0.002f)).rgb;
    sampledColour += SceneTexture.Sample(PointSample, input.sceneUV + float2(0.003f, -0.003f)).rgb;

    // Normalize accumulated color if below threshold
    if (ComputeLuminance(sampledColour) < gLuminanceThreshold)
    {
        sampledColour /= 4.0f;
    }

    // Apply brightness boost
    float3 stageOneColour = sampledColour + gBrightnessBoost;

    // Convert to a simple green-tinted night vision
    float luminance = ComputeLuminance(stageOneColour);
    stageOneColour = float3(0.0f, luminance, 0.0f);

    // Apply intensity factor from the first snippet
    stageOneColour *= gIntensity;

    // Now apply second snippet’s noise/flicker/vignette
    float3 combinedColour = stageOneColour;

    // Noise & flicker
    float noise = Hash((Hash(input.sceneUV.x) + input.sceneUV.y) * gTimer) * gNoiseIntensity;
    float flicker = sin(Hash(gTimer)) * gFlickerIntensity;

    // Apply noise & flicker
    combinedColour += noise + flicker;

    // Transform UV to [-1,1] and account for aspect ratio
    float2 coord = (input.sceneUV * 2.0f) - 1.0f;
    coord *= float2(gViewportWidth / gViewportHeight, 1.0f);

    // The vignette uses 'smoothstep'; tweak parameters as needed
    float vignette = smoothstep(
        length(coord * coord * coord * float2(0.075f, gVignetteIntensity)),
        1.0f,
        0.4f
    );

    // Combine these effects
    combinedColour *= vignette;

    // Convert to grayscale for a second green tint pass
    float finalLuminance = dot(combinedColour, float3(0.2126f, 0.7152f, 0.0722f));

    // Apply second snippet's green tint (gNightVisionTint), minus a bit of random noise to break uniform color
    float3 outputColour = finalLuminance * (gNightVisionTint - Hash(gTimer) * 0.1f);

    return float4(outputColour, 1.0f);
}
