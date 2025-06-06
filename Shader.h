//--------------------------------------------------------------------------------------
// Loading GPU shaders
// Creation of constant buffers to help send C++ values to shaders each frame
//--------------------------------------------------------------------------------------
#ifndef _SHADER_H_INCLUDED_
#define _SHADER_H_INCLUDED_

#include <d3d11.h>
#include <string>

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// Make global variables available to other files. "extern" means this variable is defined in another
// file somewhere. We should use classes and avoid use of globals, but done this way to keep code simpler
// so the DirectX content is clearer. However, try to architect your own code in a better way.

// Vertex, geometry and pixel shader DirectX objects
extern ID3D11VertexShader*   gBasicTransformVertexShader;
extern ID3D11VertexShader*   gPixelLightingVertexShader;
extern ID3D11PixelShader*    gTintedTexturePixelShader;
extern ID3D11PixelShader*    gPixelLightingPixelShader;

//*******************************
//**** Post-processing shader DirectX objects
extern ID3D11VertexShader* g2DQuadVertexShader;
extern ID3D11VertexShader* g2DPolygonVertexShader;
extern ID3D11PixelShader*  gCopyPostProcess;
extern ID3D11PixelShader*  gTintPostProcess;
extern ID3D11PixelShader*  gGreyNoisePostProcess;
extern ID3D11PixelShader*  gBurnPostProcess;
extern ID3D11PixelShader*  gDistortPostProcess;
extern ID3D11PixelShader*  gSpiralPostProcess;
extern ID3D11PixelShader*  gHeatHazePostProcess;
extern ID3D11PixelShader*  gVerticalGradientPostProcess;
extern ID3D11PixelShader*  gBlurPostProcess;
extern ID3D11PixelShader*  gUnderwaterPostProcess;
extern ID3D11PixelShader*  gGaussianHorizontalBlurPostProcess;
extern ID3D11PixelShader*  gGaussianVerticalBlurPostProcess;
extern ID3D11PixelShader*  gMotionBlurPostProcess;
extern ID3D11PixelShader*  gRetroGamePostProcess;
extern ID3D11PixelShader*  gBrightPassPostProcess;
extern ID3D11PixelShader*  gLensStarPostProcess;
extern ID3D11PixelShader*  gBloomPostProcess;
extern ID3D11PixelShader*  gDepthOfFieldPostProcess;
extern ID3D11PixelShader*  gWireframePostProcess;
extern ID3D11PixelShader*  gFogPostProcess;
extern ID3D11PixelShader*  gInvertPostProcess;
extern ID3D11PixelShader*  gNightVisionPostProcess;
extern ID3D11PixelShader*  gGameBoyPostProcess;
extern ID3D11PixelShader*  gSepiaPostProcess;
extern ID3D11PixelShader*  gChromaticDistortionPostProcess;
extern ID3D11PixelShader*  gDilationPostProcess;



//--------------------------------------------------------------------------------------
// Shader creation / destruction
//--------------------------------------------------------------------------------------

// Load shaders required for this app, returns true on success
bool LoadShaders();

// Release shaders used by the app
void ReleaseShaders();


//--------------------------------------------------------------------------------------
// Constant buffer creation / destruction
//--------------------------------------------------------------------------------------

// Create and return a constant buffer of the given size
// The returned pointer needs to be released before quitting. Returns nullptr on failure
ID3D11Buffer* CreateConstantBuffer(int size);


//--------------------------------------------------------------------------------------
// Helper functions
//--------------------------------------------------------------------------------------

// Load a shader, include the file in the project and pass the name (without the .hlsl extension)
// to this function. The returned pointer needs to be released before quitting. Returns nullptr on failure
ID3D11VertexShader*   LoadVertexShader  (std::string shaderName);
ID3D11GeometryShader* LoadGeometryShader(std::string shaderName);
ID3D11PixelShader*    LoadPixelShader   (std::string shaderName);

// Special method to load a geometry shader that can use the stream-out stage, Use like the other functions in this file except
// also pass the stream out declaration, number of entries in the declaration and the size of each output element. 
// The returned pointer needs to be released before quitting. Returns nullptr on failure. 
ID3D11GeometryShader* LoadStreamOutGeometryShader(std::string shaderName, D3D11_SO_DECLARATION_ENTRY* soDecl, unsigned int soNumEntries, unsigned int soStride);


// Helper function. Returns nullptr on failure.
ID3DBlob* CreateSignatureForVertexLayout(const D3D11_INPUT_ELEMENT_DESC vertexLayout[], int numElements);


#endif //_SHADER_H_INCLUDED_
