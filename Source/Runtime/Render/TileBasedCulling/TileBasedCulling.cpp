#include "EnginePCH.h"
#include "TileBasedCulling.h"

#include "Render/Renderer.h"
#include "Render/RenderCommand.h"
#include "Core/Async/TaskPool.h"
#include <d3dcompiler.h>

namespace
{
	// 셰이더 폴더의 .hlsl은 전부 VS/PS로 자동 컴파일되므로 컴퓨트 셰이더는 소스를 여기에 둔다.
	constexpr char TileBasedCullingShaderSource[] = R"(
#pragma pack_matrix(row_major)
#define TILE_SIZE 16
#define MAX_LIGHTS_PER_TILE 64
#define SPOT_FLAG 0x80000000

//Tile Culling에 필요한 상수 버퍼
cbuffer TileCullConstants : register(b0)
{
    matrix View;
    matrix InvProjection;
    
    uint2 TileCount;
    uint2 ScreenSize;
    
    uint TilePointLightCount;
    uint TileSpotLightCount;
    uint2 Padding;
};



//StructuredBuffer (읽기용)
StructuredBuffer<FPointLightInfo> PointLights : register(t0);
StructuredBuffer<FSpotLightInfo>  SpotLights  : register(t1);
Texture2D<float>                  SceneDepth  : register(t2);

//RWSTructuredBuffer (쓰기용)
RWStructuredBuffer<uint2> TileHeader       : register(u0); // x = offset, y = count
RWStructuredBuffer<uint>  TileLightIndices : register(u1);

//groupshared...


[numthreads(TILE_SIZE, TILE_SIZE, 1)]
void TileCullCS(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint GI : SV_GroupIndex)
{
    if (GI == 0) { sMinDepth = 0x7F7FFFFF; sMaxDepth = 0; sCount = 0; }

    GroupMemoryBarrierWithGroupSync();

    // 1. 스레드마다 픽셀 깊이 기록
// 
	//여기에 작성

    //2. 같은 그룹 안의 스레드가 1번을 다할 때까지 대기
    GroupMemoryBarrierWithGroupSync();

    //3. 그룹마다 2번 결과물로 Frustum을 만들고 라이트마다 타일에 속하는지 검사

	//여기에 작성

    //4. 같은 그룹 안의 스레드가 끝날때까지 대기 후, 구조화된 버퍼에 기록
    GroupMemoryBarrierWithGroupSync();

   //여기에 작성
})";

	ComPtr<ID3D11ComputeShader> CompileComputeShader(ID3D11Device* Device, const char* EntryPoint)
	{
		const UINT Flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3;
		ComPtr<ID3DBlob> Code, Errors;
		const HRESULT Hr = D3DCompile(TileBasedCullingShaderSource, sizeof(TileBasedCullingShaderSource) - 1, "TileCullCS",
			nullptr, nullptr, EntryPoint, "cs_5_0", Flags, 0, Code.GetAddressOf(), Errors.GetAddressOf());
		if (FAILED(Hr))
		{
			if (Errors)
				OutputDebugStringA(static_cast<const char*>(Errors->GetBufferPointer()));
			HTR_LOG(Error, "[TileBasedCulling] compute shader {} compile failed", EntryPoint);
			return nullptr;
		}

		ComPtr<ID3D11ComputeShader> Shader;
		if (FAILED(Device->CreateComputeShader(Code->GetBufferPointer(), Code->GetBufferSize(), nullptr, Shader.GetAddressOf())))
			return nullptr;
		return Shader;
	}

	ComPtr<ID3D11Buffer> CreateDefaultConstantBuffer(ID3D11Device* Device, uint32 Size)
	{
		D3D11_BUFFER_DESC Desc{};
		Desc.ByteWidth = (Size + 15) & ~15u;
		Desc.Usage = D3D11_USAGE_DEFAULT;
		Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ComPtr<ID3D11Buffer> Buffer;
		Device->CreateBuffer(&Desc, nullptr, Buffer.GetAddressOf());
		return Buffer;
	}

	constexpr uint32 TileSize = 16;
	constexpr uint32 MaxLightsPerTile = 64;

	bool CreateRWStructuredBuffer(uint32 Stride, uint32 Count, ComPtr<ID3D11Buffer>& OutBuffer, ComPtr<ID3D11UnorderedAccessView>& OutUAV, ComPtr<ID3D11ShaderResourceView>& OutSRV)
	{
		ID3D11Device* Device = RenderCommand::GetDevice();

		D3D11_BUFFER_DESC Desc{};
		Desc.ByteWidth = Stride * Count;
		Desc.Usage = D3D11_USAGE_DEFAULT;
		Desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		Desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		Desc.StructureByteStride = Stride;

		D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc{};
		UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
		UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		UAVDesc.Buffer.NumElements = Count;

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		SRVDesc.Buffer.NumElements = Count;

		if (FAILED(Device->CreateBuffer(&Desc, nullptr, OutBuffer.GetAddressOf())) ||
			FAILED(Device->CreateUnorderedAccessView(OutBuffer.Get(), &UAVDesc, OutUAV.GetAddressOf())) ||
			FAILED(Device->CreateShaderResourceView(OutBuffer.Get(), &SRVDesc, OutSRV.GetAddressOf())))
			return false;
		return true;
	}

	struct alignas(16) FTileCullConstants
	{
		FMatrix View;
		FMatrix InvProjection;

		uint32 TileCount[2] = {}; // x,y
		uint32 ScreenSize[2] = {}; // 너비,높이

		uint32 TilePointLightCount = 0;
		uint32 TileSpotLightCount = 0;
		uint32 Padding[2] = {};
	};
	static_assert(sizeof(FTileCullConstants)  == 160, "TileCullConstants must be 16-byte aligned");
}

bool FTileBasedCulling::Init()
{
	ID3D11Device* Device = RenderCommand::GetDevice();

	TileBasedCullingCS = CompileComputeShader(Device, "TileCullCS");
	TileBasedConstantsCB = CreateDefaultConstantBuffer(Device, sizeof(FTileCullConstants));

	const bool bReady = TileBasedCullingCS && TileBasedConstantsCB;
	if (!bReady)
		HTR_LOG(Error, "[TileBasedCulling] init failed. GPU occlusion culling is unavailable.");
	return bReady;
}

bool FTileBasedCulling::Run(const FTileCullInputs& Inputs)
{
	if (!TileBasedCullingCS || !Inputs.DepthSRV || Inputs.Width == 0 || Inputs.Height == 0) return false;

	// 타일 수 -> 결과 버퍼 확보
	const uint32 TileCountX = (Inputs.Width + TileSize - 1) / TileSize;
	const uint32 TileCountY = (Inputs.Height + TileSize - 1) / TileSize;
	if (!EnsureTileCapacity(TileCountX * TileCountY))
		return false;

	// 상수 채우기 -> 버퍼 갱신
	FTileCullConstants Constants{};
	Constants.View = Inputs.View;
	Constants.InvProjection = Inputs.Projection.Inverse();
	Constants.TileCount[0] = TileCountX;
	Constants.TileCount[1] = TileCountY;
	Constants.ScreenSize[0] = Inputs.Width;
	Constants.ScreenSize[1] = Inputs.Height;
	Constants.TilePointLightCount = Inputs.PointLightCount;
	Constants.TileSpotLightCount = Inputs.SpotLightCount;

	ID3D11DeviceContext* Context = RenderCommand::GetContext();
	Context->UpdateSubresource(TileBasedConstantsCB.Get(), 0, nullptr, &Constants, 0, 0);

	// 바인딩
	ID3D11Buffer* ConstBuffers[1] = { TileBasedConstantsCB.Get() };
	ID3D11ShaderResourceView* SRVs[3] = { Inputs.PointLightSRV, Inputs.SpotLightSRV, Inputs.DepthSRV };
	ID3D11UnorderedAccessView* UAVs[2] = { TileHeaderUAV.Get(), TileLightIndicesUAV.Get() };
	Context->CSSetConstantBuffers(0, 1, ConstBuffers);
	Context->CSSetShaderResources(0, 3, SRVs);
	Context->CSSetUnorderedAccessViews(0, 2, UAVs, nullptr);
	Context->CSSetShader(TileBasedCullingCS.Get(), nullptr, 0);

	// 실행
	Context->Dispatch(TileCountX, TileCountY, 1);

	// 해제
	ID3D11ShaderResourceView* NullSRVs[3] = {};
	ID3D11UnorderedAccessView* NullUAVs[2] = {};
	Context->CSSetShaderResources(0, 3, NullSRVs);
	Context->CSSetUnorderedAccessViews(0, 2, NullUAVs, nullptr);
	Context->CSSetShader(nullptr, nullptr, 0);

	return true;
}

bool FTileBasedCulling::EnsureTileCapacity(uint32 TileCount)
{
	if (TileCount <= TileHeaderCapacity && TileHeaderBuffer && TileLightIndicesBuffer)
		return true;

	TileHeaderBuffer.Reset(); TileHeaderSRV.Reset(); TileHeaderUAV.Reset();
	TileLightIndicesBuffer.Reset(); TileLightIndicesSRV.Reset(); TileLightIndicesUAV.Reset();
	TileHeaderCapacity = 0;

	if (!CreateRWStructuredBuffer(sizeof(uint64), TileCount, TileHeaderBuffer, TileHeaderUAV, TileHeaderSRV) ||
		!CreateRWStructuredBuffer(sizeof(uint32), MaxLightsPerTile * TileCount, TileLightIndicesBuffer, TileLightIndicesUAV, TileLightIndicesSRV))
	{
		HTR_LOG(Error, "[TileBasedCulling] Failed to create RWStructuredBuffers for tile culling.");
		return false;
	}

	TileHeaderCapacity = TileCount;
	return true;
}


