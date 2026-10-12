#include "EnginePCH.h"
#include "RenderResourceManager.h"

#include "RenderCommand.h"

#include "RenderUtil.h"

namespace 
{
	FString MakeShaderKey(const FString& Path, const TArray<FShaderDefine>& Defines)
	{
		FString Key = Path;
		for (const FShaderDefine& Define : Defines)
		{
			Key += "_" + Define.Name + "=" + Define.Value;
		}
		return Key;
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
		LoadOrCompileShader(Path);
	}
}

void FRenderResourceManager::Shutdown()
{
	Get().VertexShaderMap.Empty();
	Get().PixelShaderMap.Empty();
	Get().ShaderProgramMap.Empty();
}

FShaderProgram* FRenderResourceManager::GetShaderProgram(const FString& InPath, const TArray<FShaderDefine>& Defines)
{
	if (TUniquePtr<FShaderProgram>* Found = Get().ShaderProgramMap.Find(MakeShaderKey(InPath, Defines)))
		return Found->get();

	if (!Defines.IsEmpty())
	{
		Get().LoadOrCompileShader(InPath, Defines);
		if (TUniquePtr<FShaderProgram>* Found = Get().ShaderProgramMap.Find(MakeShaderKey(InPath, Defines)))
			return Found->get();

		HTR_LOG(Warning, "[Shader] not found: {} with defines", InPath);
		for (const FShaderDefine& Define : Defines)
			HTR_LOG(Warning, "  {}={}", Define.Name, Define.Value);
	}

	if (TUniquePtr<FShaderProgram>* Fallback =
		Get().ShaderProgramMap.Find("Resources/Shader/DefaultShader.hlsl"))
		return Fallback->get();

	return nullptr;
}

void FRenderResourceManager::LoadOrCompileShader(const FString& Path, const TArray<FShaderDefine>& Defines)
{ 
	FString VSCSOPath;
	FString PSCSOPath;
	FShaderByteCode VSCode = RenderUtil::GetOrCompile(Path, "mainVS", EShaderType::Vertex, VSCSOPath, Defines);
	FShaderByteCode PSCode = RenderUtil::GetOrCompile(Path, "mainPS", EShaderType::Pixel, PSCSOPath, Defines);

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
	ShaderProgramMap[MakeShaderKey(Path, Defines)] = MakeUnique<FShaderProgram>(VsRaw, PsRaw);

	HTR_LOG(Info, "[Shader] loaded: {}", Path);
}
