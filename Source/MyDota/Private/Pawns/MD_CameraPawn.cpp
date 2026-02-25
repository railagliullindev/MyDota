// Rail Agliullin Dev. All Rights Reserved


#include "Pawns/MD_CameraPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

AMD_CameraPawn::AMD_CameraPawn()
{
	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	
	// Create the camera boom component
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 1500.f;
	CameraBoom->SetRelativeRotation(FRotator(-50.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;
	
	// Create the camera component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));

	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;
}

void AMD_CameraPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TryMoveToCameraOnEdge();
}

void AMD_CameraPawn::TryMoveToCameraOnEdge()
{
	if (!bEnableEdgeScroll) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	float MouseX = 0.f;
	float MouseY = 0.f;
	if (!PC->GetMousePosition(MouseX, MouseY))
	{
		return;
	}

	int32 ViewportX = 0;
	int32 ViewportY = 0;
	PC->GetViewportSize(ViewportX, ViewportY);

	if (ViewportX <= 0 || ViewportY <= 0)
	{
		return;
	}

	// Нормализуем позицию мыши относительно центра экрана:
	// NormX: -1 слева, +1 справа
	// NormY: -1 сверху, +1 снизу
	const float NormX = (static_cast<float>(MouseX) / static_cast<float>(ViewportX) - 0.5f) * 2.f;
	const float NormY = (static_cast<float>(MouseY) / static_cast<float>(ViewportY) - 0.5f) * 2.f;
	const float ForwardFromMouse = -NormY; // сверху = вперёд

	FVector2D Direction(0.f, 0.f);

	// Edge scroll с учётом второй оси:
	// - На верхнем/нижнем краю добавляем "влево/вправо" в зависимости от MouseX
	// - На левом/правом краю добавляем "вперёд/назад" в зависимости от MouseY
	if (MouseY <= EdgeScrollThreshold)
	{
		Direction += FVector2D(NormX, 1.f);
	}
	else if (MouseY >= ViewportY - EdgeScrollThreshold)
	{
		Direction += FVector2D(NormX, -1.f);
	}

	if (MouseX <= EdgeScrollThreshold)
	{
		Direction += FVector2D(-1.f, ForwardFromMouse);
	}
	else if (MouseX >= ViewportX - EdgeScrollThreshold)
	{
		Direction += FVector2D(1.f, ForwardFromMouse);
	}

	if (!Direction.IsNearlyZero())
	{
		Direction = Direction.GetClampedToMaxSize(1.f);
		HandleCameraMove(Direction);
	}
}

void AMD_CameraPawn::HandleCameraMove(const FVector2D& Input)
{
	if (Input.IsNearlyZero())
	{
		return;
	}

	const float DeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f;
	if (DeltaSeconds <= 0.f)
	{
		return;
	}

	FVector Forward = GetActorForwardVector();
	Forward.Z = 0.f;
	Forward.Normalize();

	FVector Right = GetActorRightVector();
	Right.Z = 0.f;
	Right.Normalize();

	const FVector Movement =
		(Forward * Input.Y + Right * Input.X) * MoveSpeed * DeltaSeconds;

	AddActorWorldOffset(Movement, false);
}

void AMD_CameraPawn::HandleCameraZoom(float AxisValue)
{
	if (!CameraBoom || FMath::IsNearlyZero(AxisValue))
	{
		return;
	}

	const float DeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f;
	if (DeltaSeconds <= 0.f)
	{
		return;
	}

	const float ZoomDelta = -AxisValue * ZoomSpeed * DeltaSeconds;

	const float NewArmLength = FMath::Clamp(
		CameraBoom->TargetArmLength + ZoomDelta,
		MinZoomDistance,
		MaxZoomDistance);

	CameraBoom->TargetArmLength = NewArmLength;
}

