// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FogVisibilityComponent.generated.h"

class AFogOfWarManager;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYDOTA_API UFogVisibilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UFogVisibilityComponent();

protected:

	virtual void BeginPlay() override;

private:

	UFUNCTION()
	void OnInit();

	UFUNCTION()
	void OnVisibilityChanged(AActor* InActor, const bool bNewVisible);

	UPROPERTY()
	AFogOfWarManager* FogManager = nullptr;

	bool IsServer;
	bool IsOwnedByLocalPlayer;

	FString GetOwnerString();
};
