#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "HktIntentGameState.generated.h"

/**
 * GameState for HktIntent system.
 * Manages frame synchronization between Server and Client.
 */
UCLASS()
class HKTINTENT_API AHktIntentGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AHktIntentGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;

	/** Get the current logical frame number (Synced) */
	UFUNCTION(BlueprintCallable, Category = "Hkt|Time")
	int32 GetCurrentFrame() const;

	/** Set the server frame (Called by GameMode) */
	void SetServerFrame(int32 NewFrame) { ServerFrame = NewFrame; }

protected:
	/** Absolute frame number from Server */
	UPROPERTY(ReplicatedUsing = OnRep_ServerFrame, VisibleInstanceOnly, Category = "Hkt|Time")
	int32 ServerFrame;

	UFUNCTION()
	void OnRep_ServerFrame();

private:
	/** Local estimation of the server frame */
	int32 ClientEstimatedFrame;
	
	/** Frame accumulator for tick */
	float FrameAccumulator;

	/** Fixed Frame Rate (e.g. 60Hz) */
	static constexpr float FixedFrameTime = 1.0f / 30.0f; // 30Hz for logic
};

