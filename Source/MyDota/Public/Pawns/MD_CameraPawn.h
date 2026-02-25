// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MD_CameraPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USceneComponent;

UCLASS()
class MYDOTA_API AMD_CameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AMD_CameraPawn();
	
	FORCEINLINE UCameraComponent* GetTopDownCameraComponent() const {return CameraComponent;}
	FORCEINLINE USpringArmComponent* GetCameraBoom() const {return CameraBoom;}

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

private:
	
	void TryMoveToCameraOnEdge();
	void HandleCameraMove(const FVector2D& Input);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent;
};
