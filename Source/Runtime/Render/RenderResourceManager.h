#pragma once

#include "Shader.h"

class FRenderResourceManager
{
public:
	static FRenderResourceManager& Get()
	{
		static FRenderResourceManager* Instance = new FRenderResourceManager();
		return *Instance;
	}

	static void Init()
	{
		Get().ScanShaders("Resources/Shader");
	}

	void ScanShaders(const fs::path& ShaderRoot);

	static void Shutdown();

	static FShaderProgram* GetShaderProgram(const FString& InPath, const TArray<FShaderDefine>& Defines = {});
private:
	void LoadOrCompileShader(const FString& Path, const TArray<FShaderDefine>& Defines = {});

	TMap<FString, TUniquePtr<FVertexShader>> VertexShaderMap;
	TMap<FString, TUniquePtr<FPixelShader>>  PixelShaderMap;
	TMap<FString, TUniquePtr<FShaderProgram>> ShaderProgramMap;
}; 