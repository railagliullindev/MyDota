// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MD_UIHelper.generated.h"

/**
 *
 */
UCLASS()
class MYDOTA_API UMD_UIHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	static FString FormatTime(const int32 TotalSeconds);
};
