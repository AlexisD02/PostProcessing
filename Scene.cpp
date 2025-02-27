//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#include "Scene.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "State.h"
#include "Shader.h"
#include "Input.h"
#include "Common.h"

#include "CVector2.h" 
#include "CVector3.h" 
#include "CMatrix4x4.h"
#include "MathHelpers.h"     // Helper functions for maths
#include "GraphicsHelpers.h" // Helper functions to unclutter the code here
#include "ColourRGBA.h" 

#include <array>
#include <sstream>
#include <memory>


//--------------------------------------------------------------------------------------
// Scene Data
//--------------------------------------------------------------------------------------

//********************
// Available post-processes
enum class PostProcess
{
	None,
	Copy,
	Tint,
	GreyNoise,
	Burn,
	Distort,
	Spiral,
	HeatHaze,
	VerticalGradient,
	Underwater,
	HueVerticalGradient,
	GaussianBlurHorizontal,
	GaussianBlurVertical,
	RetroGame
};

enum class PostProcessMode
{
	Fullscreen,
	Area,
	Polygon,
	WindowPolygon
};

PostProcess		gSelectedPostProcess		= PostProcess::None;
PostProcessMode gSelectedPostProcessMode    = PostProcessMode::Fullscreen;
PostProcess     gCurrentPostProcess			= PostProcess::None;
PostProcessMode gCurrentPostProcessMode		= PostProcessMode::Fullscreen;

std::vector<std::pair<PostProcess, PostProcessMode>> gActivePostProcesses;

//********************


// Constants controlling speed of movement/rotation (measured in units per second because we're using frame time)
const float ROTATION_SPEED = 1.5f;  // Radians per second for rotation
const float MOVEMENT_SPEED = 50.0f; // Units per second for movement (what a unit of length is depends on 3D model - i.e. an artist decision usually)

// Lock FPS to monitor refresh rate, which will typically set it to 60fps. Press 'p' to toggle to full fps
bool lockFPS = true;


// Meshes, models and cameras, same meaning as TL-Engine. Meshes prepared in InitGeometry function, Models & camera in InitScene
Mesh* gStarsMesh;
Mesh* gGroundMesh;
Mesh* gCubeMesh;
Mesh* gCrateMesh;
Mesh* gLightMesh;
Mesh* gWallMesh;
Mesh* gWall2Mesh;

Model* gStars;
Model* gGround;
Model* gCube;
Model* gCrate;
Model* gWall;
Model* gWall2;

Camera* gCamera;


// Store lights in an array in this exercise
const int NUM_LIGHTS = 2;
struct Light
{
	Model*   model;
	CVector3 colour;
	float    strength;
};
Light gLights[NUM_LIGHTS];

const int NUM_WINDOWS = 3;

// Additional light information
CVector3 gAmbientColour = { 0.3f, 0.3f, 0.4f }; // Background level of light (slightly bluish to match the far background, which is dark blue)
float    gSpecularPower = 256; // Specular power controls shininess - same for all models in this app

ColourRGBA gBackgroundColor = { 0.3f, 0.3f, 0.4f, 1.0f };

// Variables controlling light1's orbiting of the cube
const float gLightOrbitRadius = 20.0f;
const float gLightOrbitSpeed = 0.7f;



//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
// Variables sent over to the GPU each frame
// The structures are now in Common.h
// IMPORTANT: Any new data you add in C++ code (CPU-side) is not automatically available to the GPU
//            Anything the shaders need (per-frame or per-model) needs to be sent via a constant buffer

PerFrameConstants gPerFrameConstants;      // The constants (settings) that need to be sent to the GPU each frame (see common.h for structure)
ID3D11Buffer*     gPerFrameConstantBuffer; // The GPU buffer that will recieve the constants above

PerModelConstants gPerModelConstants;      // As above, but constants (settings) that change per-model (e.g. world matrix)
ID3D11Buffer*     gPerModelConstantBuffer; // --"--

//**************************
PostProcessingConstants gPostProcessingConstants;       // As above, but constants (settings) for each post-process
ID3D11Buffer*           gPostProcessingConstantBuffer; // --"--
//**************************


//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------

// DirectX objects controlling textures used in this lab
ID3D11Resource*           gStarsDiffuseSpecularMap = nullptr;
ID3D11ShaderResourceView* gStarsDiffuseSpecularMapSRV = nullptr;
ID3D11Resource*           gGroundDiffuseSpecularMap = nullptr;
ID3D11ShaderResourceView* gGroundDiffuseSpecularMapSRV = nullptr;
ID3D11Resource*           gCrateDiffuseSpecularMap = nullptr;
ID3D11ShaderResourceView* gCrateDiffuseSpecularMapSRV = nullptr;
ID3D11Resource*           gCubeDiffuseSpecularMap = nullptr;
ID3D11ShaderResourceView* gCubeDiffuseSpecularMapSRV = nullptr;
ID3D11Resource* gWallDifuseSpecularMap = nullptr;
ID3D11ShaderResourceView* gWallDifuseSpecularMapSRV = nullptr;

ID3D11Resource*           gLightDiffuseMap = nullptr;
ID3D11ShaderResourceView* gLightDiffuseMapSRV = nullptr;



//****************************
// Post processing textures

// This texture will have the scene renderered on it. Then the texture is then used for post-processing
ID3D11Texture2D*          gSceneTexture      = nullptr; // This object represents the memory used by the texture on the GPU
ID3D11RenderTargetView*   gSceneRenderTarget = nullptr; // This object is used when we want to render to the texture above
ID3D11ShaderResourceView* gSceneTextureSRV   = nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

// This texture will have the scene renderered on it. Then the texture is then used for post-processing
ID3D11Texture2D*		  gSceneTextureTwo		= nullptr; // This object represents the memory used by the texture on the GPU
ID3D11RenderTargetView*   gSceneRenderTargetTwo = nullptr; // This object is used when we want to render to the texture above
ID3D11ShaderResourceView* gSceneTextureSRVTwo	= nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

ID3D11Texture2D*		  gSceneCurrentTexture		= nullptr; // This object represents the memory used by the texture on the GPU
ID3D11RenderTargetView*	  gSceneCurrentRenderTarget = nullptr; // This object is used when we want to render to the texture above
ID3D11ShaderResourceView* gSceneCurrentTextureSRV	= nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

ID3D11Texture2D*		  gFeedbackTexture = nullptr;
ID3D11RenderTargetView*	  gFeedbackRTV	   = nullptr;
ID3D11ShaderResourceView* gFeedbackSRV	   = nullptr;

// Additional textures used for specific post-processes
ID3D11Resource*           gNoiseMap = nullptr;
ID3D11ShaderResourceView* gNoiseMapSRV = nullptr;
ID3D11Resource*           gBurnMap = nullptr;
ID3D11ShaderResourceView* gBurnMapSRV = nullptr;
ID3D11Resource*           gDistortMap = nullptr;
ID3D11ShaderResourceView* gDistortMapSRV = nullptr;

// Unbind the input texture after processing
ID3D11ShaderResourceView* nullSRV = nullptr;

//****************************



//--------------------------------------------------------------------------------------
// Initialise scene geometry, constant buffers and states
//--------------------------------------------------------------------------------------

// Prepare the geometry required for the scene
// Returns true on success
bool InitGeometry()
{
	////--------------- Load meshes ---------------////

	// Load mesh geometry data, just like TL-Engine this doesn't create anything in the scene. Create a Model for that.
	try
	{
		gStarsMesh  = new Mesh("Media/Stars.x");
		gGroundMesh = new Mesh("Media/Hills.x");
		gCubeMesh   = new Mesh("Media/Cube.x");
		gCrateMesh  = new Mesh("Media/CargoContainer.x");
		gLightMesh  = new Mesh("Media/Light.x");
		gWallMesh  = new Mesh("Media/Wall1.x");
		gWall2Mesh = new Mesh("Media/Wall2.x");
	}
	catch (std::runtime_error e)  // Constructors cannot return error messages so use exceptions to catch mesh errors (fairly standard approach this)
	{
		gLastError = e.what(); // This picks up the error message put in the exception (see Mesh.cpp)
		return false;
	}


	////--------------- Load / prepare textures & GPU states ---------------////

	// Load textures and create DirectX objects for them
	// The LoadTexture function requires you to pass a ID3D11Resource* (e.g. &gCubeDiffuseMap), which manages the GPU memory for the
	// texture and also a ID3D11ShaderResourceView* (e.g. &gCubeDiffuseMapSRV), which allows us to use the texture in shaders
	// The function will fill in these pointers with usable data. The variables used here are globals found near the top of the file.
	if (!LoadTexture("Media/Stars.jpg",                &gStarsDiffuseSpecularMap,  &gStarsDiffuseSpecularMapSRV) ||
		!LoadTexture("Media/GrassDiffuseSpecular.dds", &gGroundDiffuseSpecularMap, &gGroundDiffuseSpecularMapSRV) ||
		!LoadTexture("Media/StoneDiffuseSpecular.dds", &gCubeDiffuseSpecularMap,   &gCubeDiffuseSpecularMapSRV) ||
		!LoadTexture("Media/CargoA.dds",               &gCrateDiffuseSpecularMap,  &gCrateDiffuseSpecularMapSRV) ||
		!LoadTexture("Media/brick_35.jpg",			   &gWallDifuseSpecularMap,	   &gWallDifuseSpecularMapSRV) ||
		!LoadTexture("Media/Flare.jpg",                &gLightDiffuseMap,          &gLightDiffuseMapSRV) ||
		!LoadTexture("Media/Noise.png",                &gNoiseMap,   &gNoiseMapSRV) ||
		!LoadTexture("Media/Burn.png",                 &gBurnMap,    &gBurnMapSRV) ||
		!LoadTexture("Media/Distort.png",              &gDistortMap, &gDistortMapSRV))
	{
		gLastError = "Error loading textures";
		return false;
	}


	// Create all filtering modes, blending modes etc. used by the app (see State.cpp/.h)
	if (!CreateStates())
	{
		gLastError = "Error creating states";
		return false;
	}


	////--------------- Prepare shaders and constant buffers to communicate with them ---------------////

	// Load the shaders required for the geometry we will use (see Shader.cpp / .h)
	if (!LoadShaders())
	{
		gLastError = "Error loading shaders";
		return false;
	}

	// Create GPU-side constant buffers to receive the gPerFrameConstants and gPerModelConstants structures above
	// These allow us to pass data from CPU to shaders such as lighting information or matrices
	// See the comments above where these variable are declared and also the UpdateScene function
	gPerFrameConstantBuffer       = CreateConstantBuffer(sizeof(gPerFrameConstants));
	gPerModelConstantBuffer       = CreateConstantBuffer(sizeof(gPerModelConstants));
	gPostProcessingConstantBuffer = CreateConstantBuffer(sizeof(gPostProcessingConstants));
	if (gPerFrameConstantBuffer == nullptr || gPerModelConstantBuffer == nullptr || gPostProcessingConstantBuffer == nullptr)
	{
		gLastError = "Error creating constant buffers";
		return false;
	}



	//********************************************
	//**** Create Scene Texture

	// We will render the scene to this texture instead of the back-buffer (screen), then we post-process the texture onto the screen
	// This is exactly the same code we used in the graphics module when we were rendering the scene onto a cube using a texture

	// Using a helper function to load textures from files above. Here we create the scene texture manually
	// as we are creating a special kind of texture (one that we can render to). Many settings to prepare:
	D3D11_TEXTURE2D_DESC sceneTextureDesc = {};
	sceneTextureDesc.Width = gViewportWidth;  // Full-screen post-processing - use full screen size for texture
	sceneTextureDesc.Height = gViewportHeight;
	sceneTextureDesc.MipLevels = 1; // No mip-maps when rendering to textures (or we would have to render every level)
	sceneTextureDesc.ArraySize = 1;
	sceneTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // RGBA texture (8-bits each)
	sceneTextureDesc.SampleDesc.Count = 1;
	sceneTextureDesc.SampleDesc.Quality = 0;
	sceneTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	sceneTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // IMPORTANT: Indicate we will use texture as render target, and pass it to shaders
	sceneTextureDesc.CPUAccessFlags = 0;
	sceneTextureDesc.MiscFlags = 0;
	if (FAILED(gD3DDevice->CreateTexture2D(&sceneTextureDesc, NULL, &gSceneTexture)))
	{
		gLastError = "Error creating scene texture";
		return false;
	}

	// We created the scene texture above, now we get a "view" of it as a render target, i.e. get a special pointer to the texture that
	// we use when rendering to it (see RenderScene function below)
	if (FAILED(gD3DDevice->CreateRenderTargetView(gSceneTexture, NULL, &gSceneRenderTarget)))
	{
		gLastError = "Error creating scene render target view";
		return false;
	}

	if (FAILED(gD3DDevice->CreateTexture2D(&sceneTextureDesc, NULL, &gSceneTextureTwo)))
	{
		gLastError = "Error creating scene texture two";
		return false;
	}

	if (FAILED(gD3DDevice->CreateRenderTargetView(gSceneTextureTwo, NULL, &gSceneRenderTargetTwo)))
	{
		gLastError = "Error creating scene render target view two";
		return false;
	}

	if (FAILED(gD3DDevice->CreateTexture2D(&sceneTextureDesc, NULL, &gFeedbackTexture)))
	{
		gLastError = "Error creating feedback texture";
		return false;
	}

	if (FAILED(gD3DDevice->CreateRenderTargetView(gFeedbackTexture, NULL, &gFeedbackRTV)))
	{
		gLastError = "Error creating feedback RTV";
		return false;
	}

	// We also need to send this texture (resource) to the shaders. To do that we must create a shader-resource "view"
	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc = {};
	srDesc.Format = sceneTextureDesc.Format;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;
	if (FAILED(gD3DDevice->CreateShaderResourceView(gSceneTexture, &srDesc, &gSceneTextureSRV)))
	{
		gLastError = "Error creating scene shader resource view";
		return false;
	}

	if (FAILED(gD3DDevice->CreateShaderResourceView(gSceneTextureTwo, &srDesc, &gSceneTextureSRVTwo)))
	{
		gLastError = "Error creating scene shader resource view two";
		return false;
	}

	if (FAILED(gD3DDevice->CreateShaderResourceView(gFeedbackTexture, &srDesc, &gFeedbackSRV)))
	{
		gLastError = "Error creating feedback SRV";
		return false;
	}

	return true;
}

void AddPostProcessEffect(PostProcess effect, PostProcessMode mode)
{
	gActivePostProcesses.push_back(std::pair<PostProcess, PostProcessMode>(std::make_pair(effect, mode)));
}

void RemovePostProcessEffect()
{
	if (gActivePostProcesses.empty() || gActivePostProcesses.size() <= NUM_WINDOWS)
		return;

	// If last effect is the vertical part of a Gaussian blur pair.
	if (gActivePostProcesses.back().first == PostProcess::GaussianBlurVertical)
	{
		// Remove vertical component.
		gActivePostProcesses.pop_back();
		// If there's at least one effect left and it is GaussianBlurHorizontal, remove it too.
		if (!gActivePostProcesses.empty() &&
			gActivePostProcesses.back().first == PostProcess::GaussianBlurHorizontal)
		{
			gActivePostProcesses.pop_back();
		}
	}
	else
	{
		// Otherwise, just remove the last effect.
		gActivePostProcesses.pop_back();
	}
}

static std::vector<std::array<CVector3, 4>> gAllWallOpening =
{
	{
		CVector3(22.0f, 25.0f, -50.0f), CVector3(22.0f, 5.0f, -50.0f),
		CVector3(33.0f, 25.0f, -50.0f), CVector3(33.0f, 5.0f, -50.0f)
	},
	{
		CVector3(36.0f, 25.0f, -50.0f), CVector3(36.0f, 5.0f, -50.0f),
		CVector3(49.0f, 25.0f, -50.0f), CVector3(49.0f, 5.0f, -50.0f)
	},
	{
		CVector3(50.0f, 25.0f, -50.0f), CVector3(50.0f, 5.0f, -50.0f),
		CVector3(63.0f, 25.0f, -50.0f), CVector3(63.0f, 5.0f, -50.0f)
	}
};

std::array<CVector3, 4> GetWallOpeningCoords(int openingIndex)
{
	return gAllWallOpening.at(openingIndex);
}

void InitPolygonEffects()
{
	gCurrentPostProcessMode = PostProcessMode::WindowPolygon;
	AddPostProcessEffect(PostProcess::Underwater, gCurrentPostProcessMode);
	AddPostProcessEffect(PostProcess::Underwater, gCurrentPostProcessMode);
	AddPostProcessEffect(PostProcess::Underwater, gCurrentPostProcessMode);
}


// Prepare the scene
// Returns true on success
bool InitScene()
{
	////--------------- Set up scene ---------------////

	gStars  = new Model(gStarsMesh);
	gGround = new Model(gGroundMesh);
	gCube   = new Model(gCubeMesh);
	gCrate  = new Model(gCrateMesh);
	gWall  = new Model(gWallMesh);
	gWall2  = new Model(gWall2Mesh);

	// Initial positions
	gCube->SetPosition({ 42, 5, -10 });
	gCube->SetRotation({ 0.0f, ToRadians(-110.0f), 0.0f });
	gCube->SetScale(1.5f);
	gCrate->SetPosition({ -10, 0, 90 });
	gCrate->SetRotation({ 0.0f, ToRadians(40.0f), 0.0f });
	gCrate->SetScale(6.0f);
	gStars->SetScale(8000.0f);

	gWall->SetPosition({ 70, 0, -100 });
	gWall->SetRotation({ 180.0f, 0.0f, 0.0f });
	gWall->SetScale(CVector3( 50.0f, 150.0f, 50.0f ));

	gWall2->SetPosition({ 75, 0, -150 });
	gWall2->SetRotation({ 0.0f, 0.0f, 0.0f });
	gWall2->SetScale(50.0f);

	// Light set-up - using an array this time
	for (int i = 0; i < NUM_LIGHTS; ++i)
	{
		gLights[i].model = new Model(gLightMesh);
	}

	gLights[0].colour = { 0.8f, 0.8f, 1.0f };
	gLights[0].strength = 10;
	gLights[0].model->SetPosition({ 30, 10, 0 });
	gLights[0].model->SetScale(pow(gLights[0].strength, 1.0f)); // Convert light strength into a nice value for the scale of the light - equation is ad-hoc.

	gLights[1].colour = { 1.0f, 0.8f, 0.2f };
	gLights[1].strength = 40;
	gLights[1].model->SetPosition({ -70, 30, 100 });
	gLights[1].model->SetScale(pow(gLights[1].strength, 1.0f));


	////--------------- Set up camera ---------------////

	gCamera = new Camera();
	gCamera->SetPosition({ 25, 18, -45 });
	gCamera->SetRotation({ ToRadians(10.0f), ToRadians(7.0f), 0.0f });

	InitPolygonEffects();

	return true;
}


// Release the geometry and scene resources created above
void ReleaseResources()
{
	ReleaseStates();

	if (gSceneTextureSRV)              gSceneTextureSRV->Release();
	if (gSceneRenderTarget)            gSceneRenderTarget->Release();
	if (gSceneTexture)                 gSceneTexture->Release();

	if (gSceneTextureSRVTwo)              gSceneTextureSRVTwo->Release();
	if (gSceneRenderTargetTwo)            gSceneRenderTargetTwo->Release();
	if (gSceneTextureTwo)                 gSceneTextureTwo->Release();

	if (gFeedbackSRV)            gFeedbackSRV->Release();
	if (gFeedbackRTV)            gFeedbackRTV->Release();
	if (gFeedbackTexture)        gFeedbackTexture->Release();

	if (gDistortMapSRV)                gDistortMapSRV->Release();
	if (gDistortMap)                   gDistortMap->Release();
	if (gBurnMapSRV)                   gBurnMapSRV->Release();
	if (gBurnMap)                      gBurnMap->Release();
	if (gNoiseMapSRV)                  gNoiseMapSRV->Release();
	if (gNoiseMap)                     gNoiseMap->Release();

	if (gLightDiffuseMapSRV)           gLightDiffuseMapSRV->Release();
	if (gLightDiffuseMap)              gLightDiffuseMap->Release();
	if (gCrateDiffuseSpecularMapSRV)   gCrateDiffuseSpecularMapSRV->Release();
	if (gCrateDiffuseSpecularMap)      gCrateDiffuseSpecularMap->Release();
	if (gCubeDiffuseSpecularMapSRV)    gCubeDiffuseSpecularMapSRV->Release();
	if (gCubeDiffuseSpecularMap)       gCubeDiffuseSpecularMap->Release();
	if (gWallDifuseSpecularMapSRV)     gWallDifuseSpecularMapSRV->Release();
	if (gWallDifuseSpecularMap)		   gWallDifuseSpecularMap->Release();
	if (gGroundDiffuseSpecularMapSRV)  gGroundDiffuseSpecularMapSRV->Release();
	if (gGroundDiffuseSpecularMap)     gGroundDiffuseSpecularMap->Release();
	if (gStarsDiffuseSpecularMapSRV)   gStarsDiffuseSpecularMapSRV->Release();
	if (gStarsDiffuseSpecularMap)      gStarsDiffuseSpecularMap->Release();

	if (gPostProcessingConstantBuffer)  gPostProcessingConstantBuffer->Release();
	if (gPerModelConstantBuffer)        gPerModelConstantBuffer->Release();
	if (gPerFrameConstantBuffer)        gPerFrameConstantBuffer->Release();

	ReleaseShaders();

	// See note in InitGeometry about why we're not using unique_ptr and having to manually delete
	for (int i = 0; i < NUM_LIGHTS; ++i)
	{
		delete gLights[i].model;  gLights[i].model = nullptr;
	}
	delete gCamera;  gCamera = nullptr;
	delete gCrate;   gCrate = nullptr;
	delete gCube;    gCube = nullptr;
	delete gGround;  gGround = nullptr;
	delete gStars;   gStars = nullptr;
	delete gWall;   gWall = nullptr;
	delete gWall2;   gWall2 = nullptr;

	delete gLightMesh;   gLightMesh = nullptr;
	delete gCrateMesh;   gCrateMesh = nullptr;
	delete gCubeMesh;    gCubeMesh = nullptr;
	delete gGroundMesh;  gGroundMesh = nullptr;
	delete gStarsMesh;   gStarsMesh = nullptr;
	delete gWallMesh;   gWallMesh = nullptr;
	delete gWall2Mesh;   gWall2Mesh = nullptr;
}



//--------------------------------------------------------------------------------------
// Scene Rendering
//--------------------------------------------------------------------------------------

// Render everything in the scene from the given camera
void RenderSceneFromCamera(Camera* camera)
{
	// Set camera matrices in the constant buffer and send over to GPU
	gPerFrameConstants.cameraMatrix = camera->WorldMatrix();
	gPerFrameConstants.viewMatrix = camera->ViewMatrix();
	gPerFrameConstants.projectionMatrix = camera->ProjectionMatrix();
	gPerFrameConstants.viewProjectionMatrix = camera->ViewProjectionMatrix();
	UpdateConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

	// Indicate that the constant buffer we just updated is for use in the vertex shader (VS), geometry shader (GS) and pixel shader (PS)
	gD3DContext->VSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer); // First parameter must match constant buffer number in the shader 
	gD3DContext->GSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);
	gD3DContext->PSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);

	gD3DContext->PSSetShader(gPixelLightingPixelShader, nullptr, 0);


	////--------------- Render ordinary models ---------------///

	// Select which shaders to use next
	gD3DContext->VSSetShader(gPixelLightingVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gPixelLightingPixelShader, nullptr, 0);
	gD3DContext->GSSetShader(nullptr, nullptr, 0);  // Switch off geometry shader when not using it (pass nullptr for first parameter)

	// States - no blending, normal depth buffer and back-face culling (standard set-up for opaque models)
	gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
	gD3DContext->RSSetState(gCullBackState);

	// Render lit models, only change textures for each onee
	gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

	gD3DContext->PSSetShaderResources(0, 1, &gGroundDiffuseSpecularMapSRV); // First parameter must match texture slot number in the shader
	gGround->Render();

	gD3DContext->PSSetShaderResources(0, 1, &gCrateDiffuseSpecularMapSRV); // First parameter must match texture slot number in the shader
	gCrate->Render();

	gD3DContext->PSSetShaderResources(0, 1, &gCubeDiffuseSpecularMapSRV); // First parameter must match texture slot number in the shader
	gCube->Render();

	gD3DContext->PSSetShaderResources(0, 1, &gWallDifuseSpecularMapSRV); // First parameter must match texture slot number in the shader
	gWall->Render();

	gD3DContext->PSSetShaderResources(0, 1, &gWallDifuseSpecularMapSRV); // First parameter must match texture slot number in the shader
	gWall2->Render();

	////--------------- Render sky ---------------////

	// Select which shaders to use next
	gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gTintedTexturePixelShader, nullptr, 0);

	// Using a pixel shader that tints the texture - don't need a tint on the sky so set it to white
	gPerModelConstants.objectColour = { 1, 1, 1 };

	// Stars point inwards
	gD3DContext->RSSetState(gCullNoneState);

	// Render sky
	gD3DContext->PSSetShaderResources(0, 1, &gStarsDiffuseSpecularMapSRV);
	gStars->Render();



	////--------------- Render lights ---------------////

	// Select which shaders to use next (actually same as before, so we could skip this)
	gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gTintedTexturePixelShader, nullptr, 0);

	// Select the texture and sampler to use in the pixel shader
	gD3DContext->PSSetShaderResources(0, 1, &gLightDiffuseMapSRV); // First parameter must match texture slot number in the shaer

	// States - additive blending, read-only depth buffer and no culling (standard set-up for blending)
	gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
	gD3DContext->RSSetState(gCullNoneState);

	// Render all the lights in the array
	for (int i = 0; i < NUM_LIGHTS; ++i)
	{
		gPerModelConstants.objectColour = gLights[i].colour; // Set any per-model constants apart from the world matrix just before calling render (light colour here)
		gLights[i].model->Render();
	}
}

CVector3 HSLToRGB(float h, float s, float l) {
	// Normalize h to [0, 360)
	h = fmod(fmod(h, 360.0f) + 360.0f, 360.0f);

	const float t = h / 60.0f;
	const int sector = static_cast<int>(t) % 6;
	const float f = t - static_cast<int>(t);

	const float c = (1.0f - fabs(2.0f * l - 1.0f)) * s;
	const float m = l - c * 0.5f;

	// Compute x without branches based on sector parity
	const float x = c * ((sector % 2 == 0) ? f : (1.0f - f));

	// Lookup tables for RGB coefficients
	static const struct { float c; float x; } coeff[6][3] = {
		{{1,0}, {0,1}, {0,0}},
		{{0,1}, {1,0}, {0,0}},
		{{0,0}, {1,0}, {0,1}},
		{{0,0}, {0,1}, {1,0}},
		{{0,1}, {0,0}, {1,0}},
		{{1,0}, {0,0}, {0,1}},
	};

	CVector3 rgb;
	rgb.x = c * coeff[sector][0].c + x * coeff[sector][0].x + m;
	rgb.y = c * coeff[sector][1].c + x * coeff[sector][1].x + m;
	rgb.z = c * coeff[sector][2].c + x * coeff[sector][2].x + m;

	return rgb;
}

//**************************

// Select the appropriate shader plus any additional textures required for a given post-process
// Helper function shared by full-screen, area and polygon post-processing functions below
void SelectPostProcessShaderAndTextures(PostProcess postProcess, float frameTime)
{
	if (postProcess == PostProcess::Copy)
	{
		gD3DContext->PSSetShader(gCopyPostProcess, nullptr, 0);
	}

	else if (postProcess == PostProcess::Tint)
	{
		gD3DContext->PSSetShader(gTintPostProcess, nullptr, 0);
	}

	else if (postProcess == PostProcess::GreyNoise)
	{
		gD3DContext->PSSetShader(gGreyNoisePostProcess, nullptr, 0);

		// Give pixel shader access to the noise texture
		gD3DContext->PSSetShaderResources(1, 1, &gNoiseMapSRV);
		gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);
	}

	else if (postProcess == PostProcess::Burn)
	{
		gD3DContext->PSSetShader(gBurnPostProcess, nullptr, 0);

		// Give pixel shader access to the burn texture (basically a height map that the burn level ascends)
		gD3DContext->PSSetShaderResources(1, 1, &gBurnMapSRV);
		gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);
	}

	else if (postProcess == PostProcess::Distort)
	{
		gD3DContext->PSSetShader(gDistortPostProcess, nullptr, 0);

		// Give pixel shader access to the distortion texture (containts 2D vectors (in R & G) to shift the texture UVs to give a cut-glass impression)
		gD3DContext->PSSetShaderResources(1, 1, &gDistortMapSRV);
		gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);
	}

	else if (postProcess == PostProcess::Spiral)
	{
		gD3DContext->PSSetShader(gSpiralPostProcess, nullptr, 0);
	}

	else if (postProcess == PostProcess::HeatHaze)
	{
		gD3DContext->PSSetShader(gHeatHazePostProcess, nullptr, 0);
	}	
	
	else if (postProcess == PostProcess::VerticalGradient)
	{
		gD3DContext->PSSetShader(gVerticalGradientPostProcess, nullptr, 0);

		// Set the top and bottom colours of the gradient
		gPostProcessingConstants.topColour = { 0.0f, 0.0f, 1.0f }; // Blue at the top.
		gPostProcessingConstants.bottomColour = { 1.0f, 1.0f, 0.0f }; // Yellow at the bottom.
	}

	else if (postProcess == PostProcess::GaussianBlurHorizontal || postProcess == PostProcess::GaussianBlurVertical)
	{
		if (postProcess == PostProcess::GaussianBlurHorizontal)
			gD3DContext->PSSetShader(gGaussianHorizontalBlurPostProcess, nullptr, 0);
		else
			gD3DContext->PSSetShader(gGaussianVerticalBlurPostProcess, nullptr, 0);

		// Adjust texelSize based on render target resolution (example for half-res)
		gPostProcessingConstants.texelSize = {
			2.0f / static_cast<float>(gViewportWidth),
			2.0f / static_cast<float>(gViewportHeight)
		};
		gPostProcessingConstants.blurStrength = 1.5f;
		gPostProcessingConstants.feedbackAmount = 1.0f;
	}

	else if (postProcess == PostProcess::Underwater)
	{
		gD3DContext->PSSetShader(gUnderwaterPostProcess, nullptr, 0);

		gPostProcessingConstants.underWaterTimer += frameTime;
		gPostProcessingConstants.frequency = 2.0f;
		gPostProcessingConstants.amplitude = 0.005f;
	}

	else if (postProcess == PostProcess::HueVerticalGradient)
	{
		gD3DContext->PSSetShader(gVerticalGradientPostProcess, nullptr, 0);

		static float hue = 0.0f;

		hue += frameTime * 30.0f;
		if (hue > 360.0f) hue = 0.0f;

		// Set the top and bottom colours of the gradient
		gPostProcessingConstants.topColour = HSLToRGB(hue, 1.0f, 0.5f);
		gPostProcessingConstants.bottomColour = HSLToRGB(hue + 180.0f, 1.0f, 0.5f);
	}

	else if (postProcess == PostProcess::RetroGame)
	{
		gD3DContext->PSSetShader(gRetroGamePostProcess, nullptr, 0);

		gPostProcessingConstants.pixelSize = 200.0f;
		if(!gPostProcessingConstants.paletteSize) gPostProcessingConstants.paletteSize = Random(8, 25);
	}
}

// Perform a full-screen post process from "scene texture" to back buffer
void FullScreenPostProcess(PostProcess postProcess, float frameTime, int ppIndex = 0)
{
	if (ppIndex % 2 == 0)
	{
		gD3DContext->OMSetRenderTargets(1, &gSceneRenderTargetTwo, gDepthStencil);
		gD3DContext->PSSetShaderResources(0, 1, &gSceneTextureSRV);
	}
	else
	{
		gD3DContext->OMSetRenderTargets(1, &gSceneRenderTarget, gDepthStencil);
		gD3DContext->PSSetShaderResources(0, 1, &gSceneTextureSRVTwo);
	}

	// Using special vertex shader that creates its own data for a 2D screen quad
	gD3DContext->VSSetShader(g2DQuadVertexShader, nullptr, 0);
	gD3DContext->GSSetShader(nullptr, nullptr, 0);  // Switch off geometry shader when not using it (pass nullptr for first parameter)


	// States - no blending, don't write to depth buffer and ignore back-face culling
	gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
	gD3DContext->RSSetState(gCullNoneState);


	// No need to set vertex/index buffer (see 2D quad vertex shader), just indicate that the quad will be created as a triangle strip
	gD3DContext->IASetInputLayout(NULL); // No vertex data
	gD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


	// Select shader and textures needed for the required post-processes (helper function above)
	SelectPostProcessShaderAndTextures(postProcess, frameTime);


	// Set 2D area for full-screen post-processing (coordinates in 0->1 range)
	gPostProcessingConstants.area2DTopLeft = { 0, 0 }; // Top-left of entire screen
	gPostProcessingConstants.area2DSize    = { 1, 1 }; // Full size of screen
	gPostProcessingConstants.area2DDepth   = 0;        // Depth buffer value for full screen is as close as possible


	// Pass over the above post-processing settings (also the per-process settings prepared in UpdateScene function below)
	UpdateConstantBuffer(gPostProcessingConstantBuffer, gPostProcessingConstants);
	gD3DContext->VSSetConstantBuffers(1, 1, &gPostProcessingConstantBuffer);
	gD3DContext->PSSetConstantBuffers(1, 1, &gPostProcessingConstantBuffer);


	// Draw a quad
	gD3DContext->Draw(4, 0);

	// Select the back buffer to use for rendering. Not going to clear the back-buffer because we're going to overwrite it all
	gD3DContext->OMSetRenderTargets(1, &gBackBufferRenderTarget, gDepthStencil);

	// Draw a quad
	gD3DContext->Draw(4, 0);

	// Comment
	gD3DContext->PSSetShaderResources(0, 1, &nullSRV);
}


// Perform an area post process from "scene texture" to back buffer at a given point in the world, with a given size (world units)
void AreaPostProcess(PostProcess postProcess, CVector3 worldPoint, CVector2 areaSize, float frameTime, int ppIndex)
{
	// First perform a full-screen copy of the scene to back-buffer
	FullScreenPostProcess(PostProcess::Copy, frameTime, ppIndex);

	if (ppIndex % 2 == 0)
	{
		gD3DContext->OMSetRenderTargets(1, &gSceneRenderTargetTwo, gDepthStencil);
		gD3DContext->PSSetShaderResources(0, 1, &gSceneTextureSRV);
	}
	else
	{
		gD3DContext->OMSetRenderTargets(1, &gSceneRenderTarget, gDepthStencil);
		gD3DContext->PSSetShaderResources(0, 1, &gSceneTextureSRVTwo);
	}

	gD3DContext->PSSetSamplers(0, 1, &gPointSampler);

	// Now perform a post-process of a portion of the scene to the back-buffer (overwriting some of the copy above)
	// Note: The following code relies on many of the settings that were prepared in the FullScreenPostProcess call above, it only
	//       updates a few things that need to be changed for an area process. If you tinker with the code structure you need to be
	//       aware of all the work that the above function did that was also preparation for this post-process area step

	// Select shader/textures needed for required post-process
	SelectPostProcessShaderAndTextures(postProcess, frameTime);

	// Enable alpha blending - area effects need to fade out at the edges or the hard edge of the area is visible
	// A couple of the shaders have been updated to put the effect into a soft circle
	// Alpha blending isn't enabled for fullscreen and polygon effects so it doesn't affect those (except heat-haze, which works a bit differently)
	gD3DContext->OMSetBlendState(gAlphaBlendingState, nullptr, 0xffffff);


	// Use picking methods to find the 2D position of the 3D point at the centre of the area effect
	auto worldPointTo2D = gCamera->PixelFromWorldPt(worldPoint, gViewportWidth, gViewportHeight);
	CVector2 area2DCentre = { worldPointTo2D.x, worldPointTo2D.y };
	float areaDistance = worldPointTo2D.z;

	// Nothing to do if given 3D point is behind the camera
	if (areaDistance < gCamera->NearClip())  return;

	// Convert pixel coordinates to 0->1 coordinates as used by the shader
	area2DCentre.x /= gViewportWidth;
	area2DCentre.y /= gViewportHeight;



	// Using new helper function here - it calculates the world space units covered by a pixel at a certain distance from the camera.
	// Use this to find the size of the 2D area we need to cover the world space size requested
	CVector2 pixelSizeAtPoint = gCamera->PixelSizeInWorldSpace(areaDistance, gViewportWidth, gViewportHeight);
	CVector2 area2DSize = { areaSize.x / pixelSizeAtPoint.x, areaSize.y / pixelSizeAtPoint.y };

	// Again convert the result in pixels to a result to 0->1 coordinates
	area2DSize.x /= gViewportWidth;
	area2DSize.y /= gViewportHeight;



	// Send the area top-left and size into the constant buffer - the 2DQuad vertex shader will use this to create a quad in the right place
	gPostProcessingConstants.area2DTopLeft = area2DCentre - 0.5f * area2DSize; // Top-left of area is centre - half the size
	gPostProcessingConstants.area2DSize = area2DSize;

	// Manually calculate depth buffer value from Z distance to the 3D point and camera near/far clip values. Result is 0->1 depth value
	// We've never seen this full calculation before, it's occasionally useful. It is derived from the material in the Picking lecture
	// Having the depth allows us to have area effects behind normal objects
	gPostProcessingConstants.area2DDepth = gCamera->FarClip() * (areaDistance - gCamera->NearClip()) / (gCamera->FarClip() - gCamera->NearClip());
	gPostProcessingConstants.area2DDepth /= areaDistance;

	// Pass over this post-processing area to shaders (also sends the per-process settings prepared in UpdateScene function below)
	UpdateConstantBuffer(gPostProcessingConstantBuffer, gPostProcessingConstants);
	gD3DContext->VSSetConstantBuffers(1, 1, &gPostProcessingConstantBuffer);
	gD3DContext->PSSetConstantBuffers(1, 1, &gPostProcessingConstantBuffer);

	// Draw a quad
	gD3DContext->Draw(4, 0);

	// Select the back buffer to use for rendering. Not going to clear the back-buffer because we're going to overwrite it all
	gD3DContext->OMSetRenderTargets(1, &gBackBufferRenderTarget, gDepthStencil);

	// Draw a quad
	gD3DContext->Draw(4, 0);
}


// Perform an post process from "scene texture" to back buffer within the given four-point polygon and a world matrix to position/rotate/scale the polygon
void PolygonPostProcess(PostProcess postProcess, const std::array<CVector3, 4>& points, const CMatrix4x4& worldMatrix, float frameTime, int ppIndex)
{
	// First perform a full-screen copy of the scene to back-buffer
	FullScreenPostProcess(PostProcess::Copy, frameTime, ppIndex);

	if (ppIndex % 2 == 0)
	{
		gD3DContext->OMSetRenderTargets(1, &gSceneRenderTargetTwo, gDepthStencil);
		gD3DContext->PSSetShaderResources(0, 1, &gSceneTextureSRV);
	}
	else
	{
		gD3DContext->OMSetRenderTargets(1, &gSceneRenderTarget, gDepthStencil);
		gD3DContext->PSSetShaderResources(0, 1, &gSceneTextureSRVTwo);
	}

	gD3DContext->PSSetSamplers(0, 1, &gPointSampler);

	// Now perform a post-process of a portion of the scene to the back-buffer (overwriting some of the copy above)
	// Note: The following code relies on many of the settings that were prepared in the FullScreenPostProcess call above, it only
	//       updates a few things that need to be changed for an area process. If you tinker with the code structure you need to be
	//       aware of all the work that the above function did that was also preparation for this post-process area step

	// Select shader/textures needed for required post-process
	SelectPostProcessShaderAndTextures(postProcess, frameTime);

	// Loop through the given points, transform each to 2D (this is what the vertex shader normally does in most labs)
	for (unsigned int i = 0; i < points.size(); ++i)
	{
		CVector4 modelPosition = CVector4(points[i], 1);
		CVector4 worldPosition = modelPosition * worldMatrix;
		CVector4 viewportPosition = worldPosition * gCamera->ViewProjectionMatrix();

		gPostProcessingConstants.polygon2DPoints[i] = viewportPosition;
	}

	// Pass over the polygon points to the shaders (also sends the per-process settings prepared in UpdateScene function below)
	UpdateConstantBuffer(gPostProcessingConstantBuffer, gPostProcessingConstants);
	gD3DContext->VSSetConstantBuffers(1, 1, &gPostProcessingConstantBuffer);
	gD3DContext->PSSetConstantBuffers(1, 1, &gPostProcessingConstantBuffer);

	// Select the special 2D polygon post-processing vertex shader and draw the polygon
	gD3DContext->VSSetShader(g2DPolygonVertexShader, nullptr, 0);

	// Draw a quad
	gD3DContext->Draw(4, 0);

	// Select the back buffer to use for rendering. Not going to clear the back-buffer because we're going to overwrite it all
	gD3DContext->OMSetRenderTargets(1, &gBackBufferRenderTarget, gDepthStencil);

	// Draw a quad
	gD3DContext->Draw(4, 0);
}


//**************************


// Rendering the scene
void RenderScene(float frameTime)
{
	//// Common settings ////

	// Set up the light information in the constant buffer
	// Don't send to the GPU yet, the function RenderSceneFromCamera will do that
	gPerFrameConstants.light1Colour   = gLights[0].colour * gLights[0].strength;
	gPerFrameConstants.light1Position = gLights[0].model->Position();
	gPerFrameConstants.light2Colour   = gLights[1].colour * gLights[1].strength;
	gPerFrameConstants.light2Position = gLights[1].model->Position();

	gPerFrameConstants.ambientColour  = gAmbientColour;
	gPerFrameConstants.specularPower  = gSpecularPower;
	gPerFrameConstants.cameraPosition = gCamera->Position();

	gPerFrameConstants.viewportWidth  = static_cast<float>(gViewportWidth);
	gPerFrameConstants.viewportHeight = static_cast<float>(gViewportHeight);



	////--------------- Main scene rendering ---------------////

	// Set the target for rendering and select the main depth buffer.
	// If using post-processing then render to the scene texture, otherwise to the usual back buffer
	// Also clear the render target to a fixed colour and the depth buffer to the far distance
	gD3DContext->OMSetRenderTargets(1, &gSceneRenderTarget, gDepthStencil);
	gD3DContext->ClearRenderTargetView(gSceneRenderTarget, &gBackgroundColor.r);
	gD3DContext->ClearDepthStencilView(gDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Setup the viewport to the size of the main window
	D3D11_VIEWPORT vp;
	vp.Width = static_cast<FLOAT>(gViewportWidth);
	vp.Height = static_cast<FLOAT>(gViewportHeight);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gD3DContext->RSSetViewports(1, &vp);

	// Render the scene from the main camera
	RenderSceneFromCamera(gCamera);


	////--------------- Scene completion ---------------////

    if (!gActivePostProcesses.empty())
    {
        bool useTextureOne = true;
		int ppIndex = 0;

		for (std::pair<PostProcess, PostProcessMode> postProcessAndMode : gActivePostProcesses)
		{
			gCurrentPostProcess = postProcessAndMode.first;
			gCurrentPostProcessMode = postProcessAndMode.second;

            if (gCurrentPostProcessMode == PostProcessMode::Fullscreen)
            {	
                FullScreenPostProcess(gCurrentPostProcess, frameTime, ppIndex++);
            }
            else if (gCurrentPostProcessMode == PostProcessMode::Area)
            {
				// Pass a 3D point for the centre of the affected area and the size of the (rectangular) area in world units
                AreaPostProcess(gCurrentPostProcess, gLights[0].model->Position(), { 10, 10 }, frameTime, ppIndex++);
            }
            else if (gCurrentPostProcessMode == PostProcessMode::Polygon)
            {
				// An array of four points in world space - a tapered square centred at the origin
				const std::array<CVector3, 4> points = { { {-3.0f, 5.0f, 0.0f}, {-5.0f, -5.0f, 0.0f}, 
					{3.0f, 5.0f, 0.0f}, {5.0f, -5.0f, 0.0f} } };
				// A rotating matrix placing the model above in the scene
                static CMatrix4x4 polyMatrix = MatrixTranslation({ 20.0f, 15.0f, 0.0f });
				polyMatrix = MatrixRotationY(ToRadians(0.2f)) * polyMatrix;

				// Pass an array of 4 points and a matrix. Only supports 4 points.
                PolygonPostProcess(gCurrentPostProcess, points, polyMatrix, frameTime, ppIndex++);
            }
			else if (gCurrentPostProcessMode == PostProcessMode::WindowPolygon)
			{
				// A rotating matrix placing the model above in the scene
				static CMatrix4x4 polyMatrix = MatrixTranslation({ 0.0f, 0.0f, 0.0f });
				PolygonPostProcess(gCurrentPostProcess, GetWallOpeningCoords(ppIndex), polyMatrix, frameTime, ppIndex++);
			}

            ID3D11ShaderResourceView* nullSRV = nullptr;
            gD3DContext->PSSetShaderResources(0, 1, &nullSRV);

            useTextureOne = !useTextureOne;
        }
    }
	else
	{
		//Use a simple FullScreenPostProcess
		FullScreenPostProcess(PostProcess::Copy, frameTime);
	}

	// When drawing to the off-screen back buffer is complete, we "present" the image to the front buffer (the screen)
	// Set first parameter to 1 to lock to vsync
	gSwapChain->Present(lockFPS ? 1 : 0, 0);
}


//--------------------------------------------------------------------------------------
// Scene Update
//--------------------------------------------------------------------------------------


// Update models and camera. frameTime is the time passed since the last frame
void UpdateScene(float frameTime)
{
	//***********

	// Select post process on keys
	if (KeyHit(Key_F1))  gSelectedPostProcessMode = PostProcessMode::Fullscreen;
	if (KeyHit(Key_F2))  gSelectedPostProcessMode = PostProcessMode::Area;
	if (KeyHit(Key_F3))  gSelectedPostProcessMode = PostProcessMode::Polygon;


	// Toggle effects with current mode
	if (KeyHit(Key_1)) AddPostProcessEffect(PostProcess::VerticalGradient, gSelectedPostProcessMode);
	if (KeyHit(Key_2))
	{
		AddPostProcessEffect(PostProcess::GaussianBlurHorizontal, gSelectedPostProcessMode);
		AddPostProcessEffect(PostProcess::GaussianBlurVertical, gSelectedPostProcessMode);
	}
	if (KeyHit(Key_3)) AddPostProcessEffect(PostProcess::Underwater, gSelectedPostProcessMode);
	if (KeyHit(Key_5)) AddPostProcessEffect(PostProcess::HueVerticalGradient, gSelectedPostProcessMode);
	if (KeyHit(Key_6)) AddPostProcessEffect(PostProcess::RetroGame, gSelectedPostProcessMode);

	if (KeyHit(Key_Back)) RemovePostProcessEffect();
	if (KeyHit(Key_0))
	{
		gActivePostProcesses.clear();
		InitPolygonEffects();
	}

	//if (KeyHit(Key_1))   gCurrentPostProcess = PostProcess::Tint;
	//if (KeyHit(Key_2))   gCurrentPostProcess = PostProcess::GreyNoise;
	//if (KeyHit(Key_3))   gCurrentPostProcess = PostProcess::Burn;
	//if (KeyHit(Key_4))   gCurrentPostProcess = PostProcess::Distort;
	//if (KeyHit(Key_5))   gCurrentPostProcess = PostProcess::Spiral;
	//if (KeyHit(Key_6))   gCurrentPostProcess = PostProcess::HeatHaze;
	//if (KeyHit(Key_9))   gCurrentPostProcess = PostProcess::Copy;
	//if (KeyHit(Key_0))   gCurrentPostProcess = PostProcess::None;

	// Post processing settings - all data for post-processes is updated every frame whether in use or not (minimal cost)
	
	// Colour for tint shader
	gPostProcessingConstants.tintColour = { 1, 0, 0 };

	// Noise scaling adjusts how fine the grey noise is.
	const float grainSize = 140; // Fineness of the noise grain
	gPostProcessingConstants.noiseScale  = { gViewportWidth / grainSize, gViewportHeight / grainSize };

	// The noise offset is randomised to give a constantly changing noise effect (like tv static)
	gPostProcessingConstants.noiseOffset = { Random(0.0f, 1.0f), Random(0.0f, 1.0f) };

	// Set and increase the burn level (cycling back to 0 when it reaches 1.0f)
	const float burnSpeed = 0.2f;
	gPostProcessingConstants.burnHeight = fmod(gPostProcessingConstants.burnHeight + burnSpeed * frameTime, 1.0f);

	// Set the level of distortion
	gPostProcessingConstants.distortLevel = 0.03f;

	// Set and increase the amount of spiral - use a tweaked cos wave to animate
	static float wiggle = 0.0f;
	const float wiggleSpeed = 1.0f;
	gPostProcessingConstants.spiralLevel = ((1.0f - cos(wiggle)) * 4.0f );
	wiggle += wiggleSpeed * frameTime;

	// Update heat haze timer
	gPostProcessingConstants.heatHazeTimer += frameTime;

	//***********


	// Orbit one light - a bit of a cheat with the static variable [ask the tutor if you want to know what this is]
	static float lightRotate = 0.0f;
	static bool go = true;
	gLights[0].model->SetPosition({ 20 + cos(lightRotate) * gLightOrbitRadius, 10, 20 + sin(lightRotate) * gLightOrbitRadius });
	if (go)  lightRotate -= gLightOrbitSpeed * frameTime;
	if (KeyHit(Key_L))  go = !go;

	// Control of camera
	gCamera->Control(frameTime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D);

	// Toggle FPS limiting
	if (KeyHit(Key_P))  lockFPS = !lockFPS;

	// Show frame time / FPS in the window title //
	const float fpsUpdateTime = 0.5f; // How long between updates (in seconds)
	static float totalFrameTime = 0;
	static int frameCount = 0;
	totalFrameTime += frameTime;
	++frameCount;
	if (totalFrameTime > fpsUpdateTime)
	{
		// Displays FPS rounded to nearest int, and frame time (more useful for developers) in milliseconds to 2 decimal places
		float avgFrameTime = totalFrameTime / frameCount;
		std::ostringstream frameTimeMs;
		frameTimeMs.precision(2);
		frameTimeMs << std::fixed << avgFrameTime * 1000;
		std::string windowTitle = "CO3303 Week 14: Area Post Processing - Frame Time: " + frameTimeMs.str() +
			"ms, FPS: " + std::to_string(static_cast<int>(1 / avgFrameTime + 0.5f));
		SetWindowTextA(gHWnd, windowTitle.c_str());
		totalFrameTime = 0;
		frameCount = 0;
	}
}
