// Rail Agliullin Dev. All Rights Reserved


#include "Pawns/MD_CameraPawn.h"

#include "EnhancedInputSubsystems.h"
#include "MD_GameplayTags.h"
#include "AbilitySystem/MD_AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/MD_CharacterBase.h"
#include "Components/SceneComponent.h"
#include "Components/Input/MyDotaInputComponent.h"
#include "Controllers/MD_PlayerController.h"
#include "DataAssets/StartupData/DataAsset_StartupDataBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

AMD_CameraPawn::AMD_CameraPawn() : SnapSpeed(30.f)
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
	CameraBoom->SetRelativeRotation(FRotator(-50.f, -45.f, 0.f));
	CameraBoom->bDoCollisionTest = false;
	
	// Create the camera component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));

	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;
	
	AbilitySystem = CreateDefaultSubobject<UMD_AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

void AMD_CameraPawn::BeginPlay()
{
	Super::BeginPlay();
	
	
	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("CameraLimit"), FoundVolumes);
	if (FoundVolumes.Num() > 0)
	{
		CameraBounds = FoundVolumes[0]->GetComponentsBoundingBox();
	}
	
	if (!HasAuthority())
	{
		AbilitySystem->InitAbilityActorInfo(this, this);
	}else
	{
		AbilitySystem->InitAbilityActorInfo(this, this);
		if (!StartupData.IsValid())
		{
			if (UDataAsset_StartupDataBase* LoadedData = StartupData.LoadSynchronous())
			{
				LoadedData->GiveToAbilitySystemComponent(AbilitySystem);
			}
		}
		else
		{
			StartupData->GiveToAbilitySystemComponent(AbilitySystem);
		}
	}
}

UAbilitySystemComponent* AMD_CameraPawn::GetAbilitySystemComponent() const
{
	return GetMDAbilitySystemComponent();
}

void AMD_CameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	checkf(InputConfigDataAsset, TEXT("Forgot to assign a valid asset as input config"));
	ULocalPlayer* LocalPlayer = GetController<APlayerController>()->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* EnhancedInputLPSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	
	check(EnhancedInputLPSubsystem)
	
	UMyDotaInputComponent* DotaInputComponent = CastChecked<UMyDotaInputComponent>(PlayerInputComponent);
	
	DotaInputComponent->BindAbilityInputAction(InputConfigDataAsset, this, &ThisClass::Input_AbilityInputPressed, &ThisClass::Input_AbilityInputReleased);
}
void AMD_CameraPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (AbilitySystem->HasMatchingGameplayTag(MyDotaTags::Camera_FollowingHero))
	{
		FollowHeroSmoothly(DeltaSeconds);
	}else
	{
		TryMoveToCameraOnEdge();
	}
}

void AMD_CameraPawn::ClampCameraLocation(FVector& OutLocation)
{
	if (CameraBounds.IsValid == 0) return;
	
	OutLocation.X = FMath::Clamp(OutLocation.X, CameraBounds.Min.X, CameraBounds.Max.X);
	OutLocation.Y = FMath::Clamp(OutLocation.Y, CameraBounds.Min.Y, CameraBounds.Max.Y);
}

void AMD_CameraPawn::FollowHeroSmoothly(const float& InTime)
{
	AMD_PlayerController* PC = Cast<AMD_PlayerController>(GetController());
	if (PC)
	{
		FollowingTarget = PC->GetHero();
	}
	
	if (FollowingTarget)
	{
		const FVector TargetLoc = FollowingTarget->GetActorLocation();
		const FVector CurrentLoc = GetActorLocation();
		
		FVector NewLoc = FMath::VInterpTo(CurrentLoc, TargetLoc, InTime, SnapSpeed);
		ClampCameraLocation(NewLoc);
		SetActorLocation(NewLoc);
	}
}

void AMD_CameraPawn::Input_AbilityInputPressed(FGameplayTag InInputTag)
{
	AbilitySystem->OnAbilityInputPressed(InInputTag);
}

void AMD_CameraPawn::Input_AbilityInputReleased(FGameplayTag InInputTag)
{
	AbilitySystem->OnAbilityInputReleased(InInputTag);
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
	if (Input.IsNearlyZero()) return;

	const float DeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f;
	if (DeltaSeconds <= 0.f) return;
	
	FVector Forward = CameraComponent->GetForwardVector();
	Forward.Z = 0.f;
	Forward.Normalize();

	FVector Right = CameraComponent->GetRightVector();
	Right.Z = 0.f;
	Right.Normalize();

	// 1. Считаем желаемое смещение
	const FVector Movement = (Forward * Input.Y + Right * Input.X) * MoveSpeed * DeltaSeconds;
	
	FVector NewLocation = GetActorLocation() + Movement;
	ClampCameraLocation(NewLocation);

	// 4. Устанавливаем итоговую позицию (вместо AddActorWorldOffset)
	SetActorLocation(NewLocation);
}

