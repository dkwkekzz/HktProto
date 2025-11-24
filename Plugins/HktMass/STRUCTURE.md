# HktMass 플러그인 구조

## 📁 폴더 구조 (관심사 분리)

```
HktMass/
├── Source/HktMass/
│   ├── Public/                         공개 인터페이스
│   │   ├── Fragments/                  ✅ Fragment 정의 (공유)
│   │   │   ├── HktMassNpcFragments.h
│   │   │   └── HktMassNpcAnimationTypes.h
│   │   ├── Types/                      ✅ 공용 타입 정의
│   │   │   └── HktMassNpcReplicationTypes.h
│   │   ├── HktMassModule.h
│   │   └── HktMassNpcSpawnDataAsset.h
│   │
│   └── Private/                        내부 구현
│       ├── Fragments/                  Fragment 구현
│       │   └── HktMassNpcFragments.cpp
│       │
│       ├── Movement/                   🏃 이동 시스템
│       │   ├── HktMassNpcMovementTrait.h/cpp
│       │   └── HktMassNpcMovementProcessor.h/cpp
│       │
│       ├── Combat/                     ⚔️ 전투 시스템
│       │   └── HktMassNpcCombatTrait.h/cpp
│       │
│       ├── AI/                         🤖 AI 시스템
│       │   ├── HktMassNpcAITrait.h/cpp
│       │   └── HktMassNpcAIProcessor.h/cpp
│       │
│       ├── Animation/                  🎬 애니메이션 시스템
│       │   ├── HktMassNpcAnimationTrait.h/cpp
│       │   ├── HktMassNpcAnimationProcessor.h/cpp
│       │   └── HktMassNpcUpdateISMBoneAnimationProcessor.h/cpp
│       │
│       ├── Representation/             🎨 시각화 시스템
│       │   └── HktMassNpcRepresentationTrait.h/cpp
│       │
│       ├── Replication/                🌐 네트워크 복제 시스템
│       │   ├── HktMassNpcReplicationHelpers.h/cpp
│       │   ├── HktMassNpcClientBubbleInfo.h/cpp
│       │   └── HktMassNpcReplicator.h/cpp
│       │
│       ├── Core/                       🔧 핵심 서브시스템
│       │   └── HktMassEntityModule.h/cpp
│       │
│       ├── HktMassModule.cpp
│       └── HktMassNpcTrait.h/cpp       ⚠️ Legacy
│
└── Resources/
```

## 🎯 설계 원칙

### Public (공개)
**목적**: 다른 모듈/프로젝트에서 참조 가능한 인터페이스

✅ **포함되는 것:**
- Fragment 정의 (데이터 구조)
- 공용 타입 정의
- 모듈 헤더
- 에셋 정의

❌ **포함되지 않는 것:**
- Trait 구현
- Processor 구현
- 내부 헬퍼 클래스

### Private (비공개)
**목적**: 플러그인 내부 구현 세부사항

✅ **포함되는 것:**
- 모든 Trait (헤더 + 구현)
- 모든 Processor (헤더 + 구현)
- Fragment 구현 파일
- 헬퍼 클래스
- 내부 서브시스템

## 📦 컨텐츠별 분류

| 폴더 | 책임 | Trait | Processor |
|------|------|-------|-----------|
| **Fragments** | 데이터 정의 | - | - |
| **Movement** | 이동 처리 | ✅ | ✅ |
| **Combat** | 전투 로직 | ✅ | ❌ |
| **AI** | AI 상태 관리 | ✅ | ✅ |
| **Animation** | 애니메이션 | ✅ | ✅✅ (2개) |
| **Representation** | 시각화 | ✅ | ❌ |
| **Replication** | 네트워크 | ❌ | ✅ |
| **Core** | 서브시스템 | ❌ | ❌ |

## 🔗 Include 경로

### Public Fragment 사용
```cpp
// Fragment 정의는 항상 Public/Fragments에서
#include "Fragments/HktMassNpcFragments.h"
#include "Fragments/HktMassNpcAnimationTypes.h"

// 공용 타입
#include "Types/HktMassNpcReplicationTypes.h"
```

### Private 내부에서 사용
```cpp
// 같은 폴더 내 파일
#include "HktMassNpcMovementTrait.h"
#include "HktMassNpcMovementProcessor.h"

// Fragment는 Public에서
#include "Fragments/HktMassNpcFragments.h"
```

## 💡 장점

### 1. **명확한 API 경계**
- Public: 외부에 노출되는 데이터 구조 (Fragment)
- Private: 내부 구현 세부사항 (Trait, Processor)

### 2. **의존성 최소화**
- 다른 프로젝트는 Fragment만 참조
- Trait/Processor 변경이 외부에 영향 없음

### 3. **컴파일 시간 단축**
- Public 헤더가 변경되지 않으면 재컴파일 불필요
- Private 구현만 변경 시 빠른 빌드

### 4. **관심사 분리**
- 데이터 (Fragment) vs 로직 (Trait/Processor)
- 각 시스템이 독립적인 폴더

### 5. **재사용성**
- Fragment 정의만 가져가서 다른 구현 가능
- 인터페이스와 구현 분리

## 📚 사용 예시

### 외부 프로젝트에서 Fragment 사용
```cpp
// HktProto/Source/HktProto/MySystem.cpp

// Fragment만 include (Public에 있음)
#include "Fragments/HktMassNpcFragments.h"

void MySystem::DoSomething(FMassEntityView& EntityView)
{
    // Fragment 데이터 접근
    FHktNpcCombatFragment& Combat = EntityView.GetFragmentData<FHktNpcCombatFragment>();
    Combat.CurrentHealth -= 10.0f;
}
```

### 플러그인 내부에서 Trait 구현
```cpp
// Plugins/HktMass/Source/HktMass/Private/Movement/HktMassNpcMovementTrait.cpp

#include "HktMassNpcMovementTrait.h"
#include "Fragments/HktMassNpcFragments.h"  // Public에서 가져옴

void UHktMassNpcMovementTrait::BuildTemplate(...)
{
    // Fragment 추가
    BuildContext.AddFragment<FHktNpcMovementFragment>();
}
```

## 🔍 구조 검증

### Public에 있어야 하는 것
- ✅ Fragment 정의 (.h)
- ✅ 공용 타입 정의
- ✅ 모듈 인터페이스

### Private에 있어야 하는 것
- ✅ Fragment 구현 (.cpp)
- ✅ 모든 Trait (.h + .cpp)
- ✅ 모든 Processor (.h + .cpp)
- ✅ 내부 헬퍼 클래스
- ✅ 서브시스템 구현

## 🚀 마이그레이션 체크리스트

- [x] Fragment 헤더 → `Public/Fragments/`
- [x] Fragment 구현 → `Private/Fragments/`
- [x] Trait 헤더/구현 → `Private/{Category}/`
- [x] Processor 헤더/구현 → `Private/{Category}/`
- [x] Replication 타입 → `Public/Types/`
- [x] Include 경로 업데이트
- [x] 빈 폴더 정리

## 📖 참고

이 구조는 다음 원칙을 따릅니다:
- **Pimpl Idiom**: 인터페이스와 구현 분리
- **API/Implementation Split**: 공개 API 최소화
- **Separation of Concerns**: 각 시스템 독립적 관리



