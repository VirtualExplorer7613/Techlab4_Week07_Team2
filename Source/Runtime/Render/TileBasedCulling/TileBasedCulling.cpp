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
#define TILE_SIZE 16
#define MAX_LIGHTS_PER_TILE 64
#define SPOT_FLAG 0x80000000

//Tile Culling에 필요한 상수 버퍼
cbuffer TileCullConstants : register(b0)
{
   //...
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

	struct FTileCullConstants
	{
		//...
	};
}

bool FTileBasedCulling::Init()
{
	ID3D11Device* Device = RenderCommand::GetDevice();

	TileBasedCullingCS = CompileComputeShader(Device, "BuildHiZ");
	TileBasedConstantsCB = CreateDefaultConstantBuffer(Device, sizeof(FTileCullConstants));
	ViewCB = CreateDefaultConstantBuffer(Device, sizeof(FViewConstants));

	const bool bReady = TileBasedCullingCS && TileBasedConstantsCB && ViewCB &&
		RenderCommand::SupportsConstantBufferOffsets();
	if (!bReady)
		HTR_LOG(Error, "[GPUOcclusion] init failed. GPU occlusion culling is unavailable.");
	return bReady;
}

bool FTileBasedCulling::Run()
{
    return false;


	//상수버퍼 바인딩
	//읽을 구조화된 버퍼 바인딩
	//Write할 구조화된 버퍼 바인딩
}
