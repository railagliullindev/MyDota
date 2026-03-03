// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Pawn.h"
#include "MD_CameraPawn.generated.h"

struct FGameplayTag;
class UDataAsset_InputConfig;
class UDataAsset_StartupDataBase;
class UGameplayAbility;
class UMD_AbilitySystemComponent;
class USpringArmComponent;
class UCameraComponent;
class USceneComponent;

UCLASS()
class MYDOTA_API AMD_CameraPawn : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:

	AMD_CameraPawn();

	virtual void BeginPlay() override;

	//~ Begin IAbilitySystemInterface Interface.
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	//~ End IAbilitySystemInterface Interface

	FORCEINLINE UCameraComponent* GetTopDownCameraComponent() const
	{
		return CameraComponent;
	}
	FORCEINLINE USpringArmComponent* GetCameraBoom() const
	{
		return CameraBoom;
	}
	FORCEINLINE UMD_AbilitySystemComponent* GetMDAbilitySystemComponent() const
	{
		return AbilitySystem;
	}

protected:

	virtual void Tick(float DeltaSeconds) override;

	void ClampCameraLocation(FVector& OutLocation);

	/** Скорость перемещения камеры по карте */
	UPROPERTY(EditAnywhere, Category = "Camera|Movement")
	float MoveSpeed = 2000.f;

	/** Скорость изменения дистанции камеры (зум) */
	UPROPERTY(EditAnywhere, Category = "Camera|Zoom")
	float ZoomSpeed = 2500.f;

	/** Минимальная дистанция (максимальный зум внутрь) */
	UPROPERTY(EditAnywhere, Category = "Camera|Zoom")
	float MinZoomDistance = 800.f;

	/** Максимальная дистанция (максимальный зум наружу) */
	UPROPERTY(EditAnywhere, Category = "Camera|Zoom")
	float MaxZoomDistance = 2500.f;

	/** Включено ли перемещение камеры по краям экрана, как в Dota */
	UPROPERTY(EditAnywhere, Category = "Camera|EdgeScroll")
	bool bEnableEdgeScroll = true;

	/** Размер зоны, считающейся "краем экрана", в пикселях */
	UPROPERTY(EditAnywhere, Category = "Camera|EdgeScroll", meta = (ClampMin = "1.0"))
	float EdgeScrollThreshold = 40.f;

	UPROPERTY(EditAnywhere, Category = "Camera|Following")
	float SnapSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	FBox CameraBounds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UMD_AbilitySystemComponent* AbilitySystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Startup Data")
	TSoftObjectPtr<UDataAsset_StartupDataBase> StartupData;

private:

	void TryMoveToCameraOnEdge();
	void HandleCameraMove(const FVector2D& Input);
	void FollowHeroSmoothly(const float& InTime);

	void Input_AbilityInputPressed(FGameplayTag InInputTag);
	void Input_AbilityInputReleased(FGameplayTag InInputTag);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent;

	UPROPERTY()
	AActor* FollowingTarget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Startup Data", meta = (AllowPrivateAccess = "true"))
	UDataAsset_InputConfig* InputConfigDataAsset;
};
