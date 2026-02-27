// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Pawn.h"
#include "MD_CameraPawn.generated.h"

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
	//~ End IAbilitySystemInterface Interface
	
	FORCEINLINE UCameraComponent* GetTopDownCameraComponent() const {return CameraComponent;}
	FORCEINLINE USpringArmComponent* GetCameraBoom() const {return CameraBoom;}
	FORCEINLINE UMD_AbilitySystemComponent* GetMDAbilitySystemComponent() const {return AbilitySystem;}

protected:
	virtual void Tick(float DeltaSeconds) override;

public:

	/** Зум камеры (обычно привязан к колесику мыши) */
	void HandleCameraZoom(float AxisValue);

protected:
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
	
	void ClampCameraLocation(FVector& OutLocation);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UMD_AbilitySystemComponent* AbilitySystem;
	
	UPROPERTY(EditAnywhere, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
	
private:
	
	void TryMoveToCameraOnEdge();
	void HandleCameraMove(const FVector2D& Input);
	void FollowHeroSmoothly(const float& InTime);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent;
	
	UPROPERTY()
	AActor* FollowingTarget;
};
