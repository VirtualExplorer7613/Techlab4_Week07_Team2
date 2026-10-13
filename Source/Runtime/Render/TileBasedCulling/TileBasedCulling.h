#pragma once

#include "Render/Buffer.h"
#include <d3d11.h>

class FTileBasedCulling
{
public:
	bool Init();

	bool Run();

private:
	ComPtr<ID3D11ComputeShader> TileBasedCullingCS;

	ComPtr<ID3D11Buffer> ViewCB;
	ComPtr<ID3D11Buffer> TileBasedConstantsCB;
};