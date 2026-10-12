#include "EnginePCH.h"
#include "RenderResourceManager.h"

#include "RenderCommand.h"

#include "RenderUtil.h"

namespace
{
	// ViewMode를 HLSL의 전처리 매크로 이름으로 변환한다.
	const char* GetLightingModelDefine(EViewMode Mode)
	{
		switch (Mode)
		{
		case EViewMode::Lit_Gouraud:
			return "LIGHTING_MODEL_GOURAUD";

		case EViewMode::Lit_Lambert:
			return "LIGHTING_MODEL_LAMBERT";

		case EViewMode::Lit_Phong:
			return "LIGHTING_MODEL_PHONG";

		case EViewMode::Unlit:
			return "LIGHTING_MODEL_UNLIT";

		default:
			return nullptr;
		}
	}

	FString MakeShaderVariantKey(
		const FString& Path,
		const char* DefineName)
	{
		return Path + "#" + DefineName;
	}
}

void FRenderResourceManager::ScanShaders(const fs::path& ShaderRoot)
{
	std::error_code ErrorCode;

	if (!fs::exists(ShaderRoot, ErrorCode))
	{
		HTR_LOG(Error, "[Shader] Scan Root Not Found : {}", ShaderRoot.generic_string());
		return;
	}

	for (const fs::directory_entry& Entry : fs::recursive_directory_iterator(ShaderRoot))
	{
		if (!Entry.is_regular_file()) continue;
		if (Entry.path().extension() != ".hlsl") continue;

		FString Path = Entry.path().generic_string();

		if (Entry.path().filename() == "UberLit.hlsl")
		{
			for (int32 Index = 0; Index < static_cast<int32>(EViewMode::Count);	++Index)
			{
				LoadShaderVariant(Path,	static_cast<EViewMode>(Index));
			}
		}
		else
		{
			LoadOrCompileShader(Path);
		}
	}
}

void FRenderResourceManager::Shutdown()
{
	Get().VertexShaderMap.Empty();
	Get().PixelShaderMap.Empty();
	Get().ShaderProgramMap.Empty();
}

FShaderProgram* FRenderResourceManager::GetShaderProgram(const FString& InPath)
{
	if (TUniquePtr<FShaderProgram>* Found = Get().ShaderProgramMap.Find(InPath))
		return Found->get();

	HTR_LOG(Error, "[Shader] not found: {}", InPath);

	if (TUniquePtr<FShaderProgram>* Fallback =
		Get().ShaderProgramMap.Find("Resources/Shader/DefaultShader.hlsl"))
		return Fallback->get();

	return nullptr;
}

FShaderProgram* FRenderResourceManager::GetShaderProgram(const FString& Path, EViewMode Mode)
{
	const char* DefineName = GetLightingModelDefine(Mode);

	if (DefineName)
	{
		const FString ProgramKey = MakeShaderVariantKey(Path, DefineName);

		if (TUniquePtr<FShaderProgram>* Found =	Get().ShaderProgramMap.Find(ProgramKey))
		{
			return Found->get();
		}

		HTR_LOG(Error, "[Shader] variant not found: {}", ProgramKey);
	}
	else
	{
		HTR_LOG(Error, "[Shader] invalid lighting mode");
	}

	// 실패 시 기존 기본 셰이더 조회 경로를 사용한다.
	return GetShaderProgram("Resources/Shader/DefaultShader.hlsl");
}

void FRenderResourceManager::LoadOrCompileShader(const FString& Path)
{ 
	FString VSCSOPath;
	FString PSCSOPath;
	FShaderByteCode VSCode = RenderUtil::GetOrCompile(Path, "mainVS", EShaderType::Vertex, VSCSOPath);
	FShaderByteCode PSCode = RenderUtil::GetOrCompile(Path, "mainPS", EShaderType::Pixel, PSCSOPath);

	if (!VSCode.IsValid() || !PSCode.IsValid())
	{
		HTR_LOG(Error, "[Shader] compile failed: {}", Path);
		return;
	}

	TUniquePtr<FVertexShader> Vs = RenderCommand::CreateVertexShader(VSCode);
	TUniquePtr<FPixelShader>  Ps = RenderCommand::CreatePixelShader(PSCode);

	if (!Vs || !Vs->IsValid() || !Ps || !Ps->IsValid())
	{
		HTR_LOG(Error, "[Shader] device create failed: {}", Path);
		return;
	}

	FVertexShader* VsRaw = Vs.get();
	FPixelShader* PsRaw = Ps.get();

	VertexShaderMap[VSCSOPath] = std::move(Vs);
	PixelShaderMap[PSCSOPath] = std::move(Ps);
	ShaderProgramMap[Path] = MakeUnique<FShaderProgram>(VsRaw, PsRaw);

	HTR_LOG(Info, "[Shader] loaded: {}", Path);

}

void FRenderResourceManager::LoadShaderVariant(const FString& Path, EViewMode Mode)
{
	const char* DefineName = GetLightingModelDefine(Mode);

	if (!DefineName)
	{
		HTR_LOG(Error, "[Shader] invalid lighting mode");
		return;
	}

	const FString ProgramKey = MakeShaderVariantKey(Path, DefineName);

	if (ShaderProgramMap.Contains(ProgramKey))
		return;

	D3D_SHADER_MACRO Defines[] =
	{
		{ DefineName, "1" },
		{nullptr, nullptr}
	};

	FShaderByteCode VSCode = RenderUtil::CompileDefineShader(Path, "mainVS", EShaderType::Vertex, Defines);
	FShaderByteCode PSCode = RenderUtil::CompileDefineShader(Path, "mainPS", EShaderType::Pixel, Defines);

	if (!VSCode.IsValid() || !PSCode.IsValid())
	{
		HTR_LOG(Error, "[Shader] variant compile failed: {}", ProgramKey);

		return;
	}

	TUniquePtr<FVertexShader> VS = RenderCommand::CreateVertexShader(VSCode);
	TUniquePtr<FPixelShader> PS = RenderCommand::CreatePixelShader(PSCode);

	if (!VS || !VS->IsValid() || !PS || !PS->IsValid())
	{
		HTR_LOG(Error,"[Shader] variant device create failed: {}", ProgramKey);

		return;
	}

	// 소유권을 이동하기 전에 실제 객체의 주소를 보관한다.
	FVertexShader* VSRaw = VS.get();
	FPixelShader* PSRaw = PS.get();

	const FString VSKey = ProgramKey + "#VS";
	const FString PSKey = ProgramKey + "#PS";

	VertexShaderMap[VSKey] = std::move(VS);
	PixelShaderMap[PSKey] = std::move(PS);

	ShaderProgramMap[ProgramKey] = MakeUnique<FShaderProgram>(VSRaw, PSRaw);

	HTR_LOG(Info, "[Shader] variant loaded: {}", ProgramKey);
}
