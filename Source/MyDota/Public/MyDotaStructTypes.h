#pragma once
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"

#include "MyDotaStructTypes.generated.h"

USTRUCT(BlueprintType)
struct FMDHeroAbilitySet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> AbilityToGrant;

	bool IsValid() const
	{
		return InputTag.IsValid() && AbilityToGrant;
	}
};

// Структура для эффективной передачи
USTRUCT()
struct FFogBitData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<uint32> BitArray; // Храним биты внутри 32-битных чисел

	void SetBit(int32 Index, bool bVisible)
	{
		int32 WordIndex = Index / 32;
		int32 BitIndex = Index % 32;
		if (bVisible) BitArray[WordIndex] |= (1 << BitIndex);
		else BitArray[WordIndex] &= ~(1 << BitIndex);
	}
};

// массив структур для всех источников обзора
USTRUCT(BlueprintType)
struct FVisionSource
{

	GENERATED_BODY()

	UPROPERTY()
	AActor* SourceActor;

	float Radius;
	uint8 LastViewerHeight;
	FVector LastLocation;
	bool bIsDirty;

	TArray<int32> VisibleIndices;

	FVisionSource()
		: SourceActor(nullptr)
		, Radius(0.0f)
		, LastViewerHeight(0)
		, bIsDirty(false)
	{
	}
};

UENUM(BlueprintType)
enum class EMDTeam : uint8
{
	None,
	Dire,
	Radiant,
	Neutral
};

UENUM(BlueprintType)
enum EChangeType
{
	None UMETA(DisplayName = "None"),
	PlayerJoined UMETA(DisplayName = "Player Joined"),
	PlayerLeft UMETA(DisplayName = "Player Left"),
	HeroSelected UMETA(DisplayName = "Hero Selected"),
	TeamChanged UMETA(DisplayName = "Team Changed")
};

USTRUCT(BlueprintType)
struct FPlayerTeamInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 PlayerId = -1;

	UPROPERTY(BlueprintReadOnly)
	int32 Team = -1;

	UPROPERTY(BlueprintReadOnly)
	int32 HeroId = -1;

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly)
	int32 Slot = -1;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EChangeType> ChangeType = EChangeType::None;

public:

	EMDTeam GetTeam() const
	{
		return static_cast<EMDTeam>(Team);
	}

	void SetTeam(EMDTeam NewTeam)
	{
		Team = static_cast<int32>(NewTeam);
	}
};

USTRUCT(BlueprintType)
struct FTestStruct
{
	GENERATED_BODY()

	UPROPERTY()
	int32 PlayerId = -1;
	UPROPERTY()
	int32 RandomInt = 0;
};
