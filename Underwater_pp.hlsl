//--------------------------------------------------------------------------------------
// Underwater Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
// Applies a full-screen underwater effect by tinting the scene blue and adding a slow,
// dynamic wobble (using sine waves) to the texture coordinates. The wobble parameters
// are updated over time to create a gentle animated distortion.
//--------------------------------------------------------------------------------------

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// The scene has been rendered to a texture. These variables allow access to that texture.
Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0);

//--------------------------------------------------------------------------------------
// Shader Code
//--------------------------------------------------------------------------------------

float4 main(PostProcessingInput input) : SV_Target
{
    float2 offset;
    offset.x = sin(input.areaUV.x * radians(360.0f) + gTimer * 3.0f);
    offset.y = cos(input.areaUV.y * radians(360.0f) + gTimer * 3.0f);
	
    float2 waterOffset = offset * gAmplitude;
    
    float3 sceneColour = SceneTexture.Sample(PointSample, input.sceneUV + waterOffset).rgb;
    
    float3 outputColour = sceneColour * float3(0.0f, 0.3f, 0.6f);

    return float4(outputColour, 1.0f);
}
