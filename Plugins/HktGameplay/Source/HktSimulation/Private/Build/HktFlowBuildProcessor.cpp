// Copyright Hkt Studios, Inc. All Rights Reserved.

#include "Build/HktFlowBuildProcessor.h"
#include "Build/IFlowDefinition.h"
#include "Build/FlowDefinitionRegistry.h"
#include "Core/HktVMBuilder.h"
#include "HktIntentInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogFlowBuild, Log, All);

FHktFlowBuildProcessor::FHktFlowBuildProcessor()
{
}

FHktFlowBuildProcessor::~FHktFlowBuildProcessor()
{
    ClearCache();
}

const FHktProgram* FHktFlowBuildProcessor::GetOrBuildProgram(
    const FGameplayTag& FlowTag,
    const FHktIntentEvent* Event,
    FHktAttributeStore* Attributes)
{
    // 캐시 확인
    const FHktProgram* Cached = ProgramCache.Get(FlowTag);
    if (Cached)
    {
        if (bDebugLog)
        {
            UE_LOG(LogFlowBuild, Verbose, TEXT("Cache hit for: %s"), *FlowTag.ToString());
        }
        return Cached;
    }
    
    // 빌드
    TSharedPtr<FHktProgram> NewProgram = BuildProgramFromDefinition(FlowTag, Event, Attributes);
    if (!NewProgram.IsValid() || !NewProgram->IsValid())
    {
        UE_LOG(LogFlowBuild, Warning, TEXT("Failed to build program for: %s"), *FlowTag.ToString());
        return nullptr;
    }
    
    // 캐시에 추가
    ProgramCache.Add(FlowTag, NewProgram);
    
    if (bDebugLog)
    {
        UE_LOG(LogFlowBuild, Log, TEXT("Built and cached program for: %s (Instructions: %d)"), 
            *FlowTag.ToString(), NewProgram->GetInstructionCount());
        NewProgram->DebugDump();
    }
    
    return NewProgram.Get();
}

const FHktProgram* FHktFlowBuildProcessor::RebuildProgram(
    const FGameplayTag& FlowTag,
    const FHktIntentEvent* Event,
    FHktAttributeStore* Attributes)
{
    // 캐시 무효화
    InvalidateCache(FlowTag);
    
    // 다시 빌드
    return GetOrBuildProgram(FlowTag, Event, Attributes);
}

const FHktProgram* FHktFlowBuildProcessor::GetCachedProgram(const FGameplayTag& FlowTag) const
{
    return ProgramCache.Get(FlowTag);
}

void FHktFlowBuildProcessor::InvalidateCache(const FGameplayTag& FlowTag)
{
    // FHktProgramCache에 Remove가 없으므로 전체 리빌드 필요
    // TODO: FHktProgramCache에 Remove 메서드 추가
    if (bDebugLog)
    {
        UE_LOG(LogFlowBuild, Log, TEXT("Cache invalidation requested for: %s"), *FlowTag.ToString());
    }
}

void FHktFlowBuildProcessor::ClearCache()
{
    ProgramCache.Clear();
    
    if (bDebugLog)
    {
        UE_LOG(LogFlowBuild, Log, TEXT("Program cache cleared"));
    }
}

int32 FHktFlowBuildProcessor::GetCachedProgramCount() const
{
    return ProgramCache.Num();
}

TSharedPtr<FHktProgram> FHktFlowBuildProcessor::BuildProgramFromDefinition(
    const FGameplayTag& FlowTag,
    const FHktIntentEvent* Event,
    FHktAttributeStore* Attributes)
{
    // Registry에서 FlowDefinition 찾기
    IFlowDefinition* Definition = FFlowDefinitionRegistry::Find(FlowTag);
    if (!Definition)
    {
        UE_LOG(LogFlowBuild, Warning, TEXT("No FlowDefinition registered for: %s"), *FlowTag.ToString());
        return nullptr;
    }
    
    // 이벤트 유효성 검사
    if (Event && !Definition->ValidateEvent(*Event))
    {
        UE_LOG(LogFlowBuild, Warning, TEXT("Event validation failed for: %s"), *FlowTag.ToString());
        return nullptr;
    }
    
    // Builder 생성 및 빌드
    FHktVMBuilder Builder;
    
    // 더미 이벤트 (Event가 nullptr인 경우)
    FHktIntentEvent DummyEvent;
    if (!Event)
    {
        DummyEvent.EventTag = FlowTag;
        Event = &DummyEvent;
    }
    
    bool bSuccess = Definition->BuildBytecode(Builder, *Event, Attributes);
    if (!bSuccess)
    {
        UE_LOG(LogFlowBuild, Warning, TEXT("BuildBytecode failed for: %s"), *FlowTag.ToString());
        return nullptr;
    }
    
    // 프로그램 빌드 및 반환
    return Builder.Build(FlowTag);
}
