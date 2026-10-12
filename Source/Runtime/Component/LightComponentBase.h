#pragma once

#include "SceneComponent.h"

class FLineBatcher;

class ULightComponentBase : public USceneComponent
{
	DECLARE_CLASS(ULightComponentBase, USceneComponent)
	REFLECT_START(ULightComponentBase)
		PROPERTY(Intensity)
		PROPERTY_TYPE(LightColor, Color)
		PROPERTY(bVisible)
	REFLECT_END()

public:
	ULightComponentBase() = default;
	virtual ~ULightComponentBase() override;

	virtual void InitializeComponent() override;

	// 에디터에서 선택됐을 때 라이트 범위·방향을 와이어프레임으로 그린다. 기본은 그릴 것 없음(Ambient).
	virtual void DrawDebug(FLineBatcher* LineBatcher) const {}

	void SetIntensity(float NewIntensity) { Intensity = NewIntensity; }
	float GetIntensity() const { return Intensity; }
	void SetLightColor(const FVector4& NewColor) { LightColor = NewColor; }
	FVector4 GetLightColor() const { return LightColor; }
	void SetVisible(bool bNewVisible) { bVisible = bNewVisible; }
	bool IsVisible() const { return bVisible; }

protected:
	float Intensity = 1.0f;
	FVector4 LightColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
	bool bVisible = true;
};
