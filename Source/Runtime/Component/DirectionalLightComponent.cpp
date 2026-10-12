#include "EnginePCH.h"
#include "DirectionalLightComponent.h"

#include "Render/LineBatcher.h"

namespace
{
	constexpr float ARROW_LENGTH = 3.0f;
	constexpr float ARROW_HEAD_LENGTH = 0.6f;
	constexpr float ARROW_HEAD_WIDTH = 0.3f;
}

void UDirectionalLightComponent::DrawDebug(FLineBatcher* LineBatcher) const
{
	if (LineBatcher == nullptr)
	{
		return;
	}

	FTransform WorldTransform;
	WorldTransform.Location = GetWorldLocation();
	WorldTransform.Rotation = GetWorldRotation();

	// Forward = 빛이 나아가는 방향. Right/Up은 화살촉을 벌릴 수직 축이다.
	const FVector Start = WorldTransform.Location;
	const FVector Forward = WorldTransform.GetForward();
	const FVector Right = WorldTransform.GetRight();
	const FVector Up = WorldTransform.GetUp();

	const FVector Tip = Start + Forward * ARROW_LENGTH;
	const FVector HeadBase = Tip - Forward * ARROW_HEAD_LENGTH;

	// 몸통
	LineBatcher->AddLine(Start, Tip, LightColor);

	// 화살촉: 네 방향으로 벌린 선
	LineBatcher->AddLine(Tip, HeadBase + Right * ARROW_HEAD_WIDTH, LightColor);
	LineBatcher->AddLine(Tip, HeadBase - Right * ARROW_HEAD_WIDTH, LightColor);
	LineBatcher->AddLine(Tip, HeadBase + Up * ARROW_HEAD_WIDTH, LightColor);
	LineBatcher->AddLine(Tip, HeadBase - Up * ARROW_HEAD_WIDTH, LightColor);
}
