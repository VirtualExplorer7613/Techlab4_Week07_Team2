#pragma once

#include "Render/Buffer.h"
#include <d3d11.h>

class FTileBasedCulling
{
public:
	bool Init();

	bool Run();

	ID3D11ShaderResourceView* GetTileHeaderSRV() const { return TileHeaderSRV.Get(); }
	ID3D11ShaderResourceView* GetTileLightIndicesSRV() const { return TileLightIndicesSRV.Get(); }

private:
	ComPtr<ID3D11ComputeShader> TileBasedCullingCS;

	ComPtr<ID3D11Buffer> ViewCB;
	ComPtr<ID3D11Buffer> TileBasedConstantsCB;

	ComPtr<ID3D11Buffer> TileHeaderBuffer;
	ComPtr<ID3D11ShaderResourceView> TileHeaderSRV;
	ComPtr<ID3D11UnorderedAccessView> TileHeaderUAV;

	ComPtr<ID3D11Buffer> TileLightIndicesBuffer;
	ComPtr<ID3D11ShaderResourceView> TileLightIndicesSRV;
	ComPtr<ID3D11UnorderedAccessView> TileLightIndicesUAV;

	uint32 TileHeaderCapacity = 0;

	bool EnsureTileCapacity(uint32 TileCount);
};