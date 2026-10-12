#include "EnginePCH.h"
#include "PointLightComponent.h"
#include "Engine/World.h"
#include "Render/LineBatcher.h"

// 디버깅용
namespace
{
	constexpr int32 SPHERE_LATITUDE_COUNT = 12;	// 구의 가로선
	constexpr int32 SPHERE_LONGITUDE_COUNT = 24;	// 구의 세로선

	void AddWireSphere(
		FLineBatcher* LineBatcher,
		const FVector& Center,
		float Radius, 
		const FVector4& Color)
	{
		for (int32 i = 0; i < SPHERE_LATITUDE_COUNT; ++i)
		{
			const float Latitude = -PI / 2.0f + PI * static_cast<float>(i) / SPHERE_LATITUDE_COUNT;

			const float RingRadius = Radius * cos(Latitude);
			const float Z = Radius * sin(Latitude);

			FVector PreviousPoint;
			FVector FirstPoint;

			for (int32 j = 0; j < SPHERE_LONGITUDE_COUNT; ++j)
			{
				const float Angle = 2.0f * PI * static_cast<float>(j) / SPHERE_LONGITUDE_COUNT;

				const FVector Point = Center + FVector(RingRadius * cos(Angle), RingRadius * sin(Angle), Z);

				if (j == 0)
				{
					FirstPoint = Point;
				}
				else
				{
					LineBatcher->AddLine(PreviousPoint, Point, Color);
				}
				PreviousPoint = Point;
			}
			// 마지막 점과 첫 점을 이어 원을 닫는다 
			LineBatcher->AddLine(PreviousPoint, FirstPoint, Color);


			for (int32 i = 0; i < SPHERE_LONGITUDE_COUNT; ++i)
			{
				const float Angle = 2.0f * PI * static_cast<float>(i) / SPHERE_LONGITUDE_COUNT;

				FVector PreviousPoint;
				FVector FirstPoint;

				for (int32 j = 0; j <= SPHERE_LATITUDE_COUNT; ++j)
				{
					const float Latitude = -PI / 2.0f + PI * static_cast<float>(j) / SPHERE_LATITUDE_COUNT;

					const FVector Point = Center +
						FVector(Radius * cos(Latitude) * cos(Angle),
							Radius * cos(Latitude) * sin(Angle),
							Radius * sin(Latitude));

					if (j == 0)
					{
						FirstPoint = Point;
					}
					else
					{
						LineBatcher->AddLine(PreviousPoint, Point, Color);
					}
					PreviousPoint = Point;
				}
			}
		
		}
	}
}

UPointLightComponent::~UPointLightComponent()
{
    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = Owner->GetWorld())
        {
            World->GetScene().RemoveLight(this);
        }
    }
}

void UPointLightComponent::InitializeComponent()
{
    if (AActor* Owner = GetOwner())
    {
        if (UWorld* World = Owner->GetWorld())
        {
            World->GetScene().AddLight(this);
        }
    }

    Super::InitializeComponent();
}

void UPointLightComponent::DrawDebug(FLineBatcher* LineBatcher) const
{
	if (LineBatcher == nullptr)
	{
		return;
	}

	const FVector Center = GetWorldLocation();

	// 반지름 음수 방지
	const float SafeRadius = (Radius > 0.0f) ? Radius : 0.0f;

	AddWireSphere(LineBatcher, Center, SafeRadius, LightColor);
}
