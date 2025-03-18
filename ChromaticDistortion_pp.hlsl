//--------------------------------------------------------------------------------------
// Chromatic Aberration + Lens Distortion Pixel Shader
// No smoothing/clamping for off-screen UVs.
//--------------------------------------------------------------------------------------

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures & Samplers
//--------------------------------------------------------------------------------------

Texture2D SceneTexture : register(t0);
SamplerState Sampler : register(s0);


//--------------------------------------------------------------------------------------
// Helper: Safely sample the scene, returning black (or a custom color) if UV is out of range
//--------------------------------------------------------------------------------------

float3 SafeSampleScene(Texture2D tex, SamplerState samp, float2 uv)
{
    // If the UV is out of [0..1], return black (no smoothing/clamping)
    if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
    return tex.SampleLevel(samp, uv, 0.0f).rgb;
}

//--------------------------------------------------------------------------------------
// Main pixel shader
//--------------------------------------------------------------------------------------

float4 main(PostProcessingInput input) : SV_Target
{
    float2 uv = input.sceneUV;

    // Shift UV to center-based coordinates
    float2 offset = uv - gScreenCenter;
    float dist = length(offset);
    
    // Lens Distortion
    float distortedDist = dist * (1.0f + gDistortionAmount * dist * dist);
    float2 direction = normalize(offset);

    float2 uvDistorted = gScreenCenter + (direction * distortedDist);

    // Chromatic Aberration
    // Shift each channel differently (Distances are all from center, so the shift is outward/inward.)
    float2 uvR = uvDistorted + (direction * gChromAbAmount * dist);
    float2 uvG = uvDistorted;
    float2 uvB = uvDistorted - (direction * gChromAbAmount * dist);

    // Safe Sampling (No clamp or mirror)
    float3 colourR = SafeSampleScene(SceneTexture, Sampler, uvR);
    float3 colourG = SafeSampleScene(SceneTexture, Sampler, uvG);
    float3 colourB = SafeSampleScene(SceneTexture, Sampler, uvB);

    float3 outputColour = float3(colourR.r, colourG.g, colourB.b);

    return float4(outputColour, 1.0f);
}
