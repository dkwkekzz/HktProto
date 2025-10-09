#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"

class FHktReliableUdpServer;

class FHktLocalServer : public FTickableGameObject
{
public:
    FHktLocalServer();
    ~FHktLocalServer();

    void Start();
    void Stop();
    bool IsRunning() const;

    // FTickableGameObject interface
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override;
    virtual TStatId GetStatId() const override;
    // ~FTickableGameObject interface

private:
    TUniquePtr<FHktReliableUdpServer> Server;
};

