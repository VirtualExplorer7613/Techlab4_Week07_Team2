#pragma once
#include "Core/Types.h"

// View별 장면 표시 방식. 셰이딩 모델과 Wireframe(래스터라이저 Fill Mode)을 한 메뉴에서 고른다.
// 값은 editor.ini에 정수로 저장되므로 순서를 바꾸면 저장된 설정이 다른 모드로 읽힌다.
enum class EViewMode : uint8
{
    Lit_Phong,     // 기본값
    Lit_Lambert,
    Lit_Gouraud,
    Unlit,
    WorldNormal,
    Wireframe,
    MAX
};

// 뷰포트 콤보에 표시할 이름. EViewMode와 같은 순서다.
inline constexpr const char* ViewModeLabels[] =
{
    "Lit_Phong",
    "Lit_Lambert",
    "Lit_Gouraud",
    "Unlit",
    "World Normal",
    "Wireframe",
};

// Renderer에서 ViewMode에 대응하여 가져올 Permutation 이름, 각 모드에 대응하는 셰이더 프로그램을 가져오는 데에 사용된다.
inline constexpr const char* ViewModeToPermutation[] =
{
    "PHONG",
    "LAMBERT",
    "GOURAUD",
    "UNLIT",
    "WORLDNORMAL",
    "UNLIT",
};
static_assert(sizeof(ViewModeLabels) / sizeof(ViewModeLabels[0]) == static_cast<size_t>(EViewMode::MAX),
    "ViewModeLabels must match EViewMode");

static_assert(sizeof(ViewModeToPermutation) / sizeof(ViewModeToPermutation[0]) == static_cast<size_t>(EViewMode::MAX),
    "ViewModeToPermutation must match EViewMode");
