Core 폴더 (Private/Core/):
파일	역할
HktVMTypes.h	기본 타입, 64비트 레지스터 union, OpCode enum, 8바이트 고정 명령어
HktVMProgram.h	프로그램 구조체, 태그 테이블, 상수 풀, 프로그램 캐시
HktVMBatch.h	SoA 레이아웃 VM 배치 (64개 동시 실행), 리스트 저장소
HktVMDispatch.h/cpp	함수 포인터 테이블, 30+ 명령어 구현
HktStateTypes.h	상태 타입 정의 (Entity, Player, Process)
HktStateStore.h	통합 상태 저장소 (엔티티/플레이어/프로세스)
HktStateCache.h	VM별 로컬 캐시 (실행 중 사용)
HktVMBuilder.h	Fluent API 바이트코드 빌더
HktVM.h	VM 실행 엔진

핵심 설계 특징
자연어 시간-공간 연속성
Flow는 선형으로 읽히며, WaitSeconds, WaitUntilCollision 등이 시간을 추상화
콜백 없이 Yield/Resume 패턴으로 시간 흐름 유지
DOD 최적화
SoA 레이아웃: PC, State, WaitTimer 등 필드별 연속 배열
64바이트 캐시 라인 정렬 (alignas(64))
고정 크기 명령어 (8바이트)
함수 포인터 테이블로 switch 오버헤드 제거
5요소 구현
명령어 (무엇을): EHktOp enum, 30+ 연산
태그 (어떻게): 프로그램 내 태그 테이블로 8비트 인덱싱
분기 (언제): Jump, JumpIfZero, ForEach 등
레지스터 (휘발성): 16개 64비트 레지스터
속성 (영구적): FHktStateStore 통합 저장소

핵심 설계
1. FHktAttribute (상태 단위)
모든 값은 int32 기반 (Fixed Point: float * 100)
FGameplayTag로 식별 (무한 확장 가능)
2. FHktEntityState (엔티티 상태)
FHktAttributeMap Attributes;   // 태그 → int32 매핑FHktTagSet StatusTags;         // 상태 태그 (Burning, Stunned...)FHktTagSet BuffTags;           // 버프 태그
3. FHktProcessState (VM 프로세스 상태)
FHktTagSet DoingTags;          // 현재 진행 중인 작업FHktTagSet CompletedTags;      // 완료된 작업
4. 캐시 효율 패턴
VM 시작 → StateCache.Init() → 로컬 캐시로 복사    ↓VM 실행 → Cache에서 읽기/쓰기 (인접 메모리)    ↓VM 종료 → StateCache.Flush() → 원본에 일괄 적용
새로운 OpCode
OpCode	설명
JumpIfHasTag	엔티티가 태그를 가지면 점프
JumpIfNotHasTag	엔티티가 태그를 안가지면 점프
JumpIfProcessDoing	프로세스가 태그 작업 중이면 점프
JumpIfProcessDone	프로세스가 태그 작업 완료면 점프
LoadAttrByTag	태그로 속성 로드
StoreAttrByTag	태그로 속성 저장
AddAttrByTag	태그 속성에 값 더하기
AddStatusTag	상태 태그 추가
RemoveStatusTag	상태 태그 제거
MarkProcessDoing	프로세스 Doing 태그 설정
MarkProcessDone	프로세스 Done 태그 설정

// 기존 (enum 기반)
Builder.LoadAttribute(REG, 0xFF, EHktAttrType::Health);

// 새로운 (태그 기반 - 확장 가능)
FGameplayTag HealthTag = FGameplayTag::RequestGameplayTag("Attr.Health");
Builder.LoadAttrByTag(REG, 0xFF, HealthTag);

// 태그 분기
Builder.JumpIfOwnerHasTag(TAG_Status_Burning, SkipPC);

// 프로세스 진행 표시
Builder.MarkDoing(TAG_Action_Casting);
// ... 캐스팅 로직 ...
Builder.MarkDone(TAG_Action_Casting);