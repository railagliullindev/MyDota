// Rail Agliullin Dev. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "DataAssets/Input/DataAsset_InputConfig.h"
#include "MyDotaInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class MYDOTA_API UMyDotaInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
	
public:
	template <class UserObject, typename CallbackFunc>
	void BindNativeInputAction(const UDataAsset_InputConfig* InInputConfig, const FGameplayTag& InInputTag,
							   ETriggerEvent TriggerEvent, UserObject* ContextOnject, CallbackFunc Func);

	template <class UserObject, typename CallbackFunc>
	void BindAbilityInputAction(const UDataAsset_InputConfig* InInputConfig, UserObject* ContextOnject, CallbackFunc InputPressFunc, CallbackFunc InputReleaseFunc);
};

template <class UserObject, typename CallbackFunc>
void UMyDotaInputComponent::BindNativeInputAction(const UDataAsset_InputConfig* InInputConfig,
												   const FGameplayTag& InInputTag, ETriggerEvent TriggerEvent,
												   UserObject* ContextOnject, CallbackFunc Func)
{
	checkf(InInputConfig, TEXT("Input config data asset is null, cant not proceed with binding"));

	if (UInputAction* FoundAction = InInputConfig->FindNativeInputActionByTag(InInputTag))
	{
		BindAction(FoundAction, TriggerEvent, ContextOnject, Func);
	}
}

template <class UserObject, typename CallbackFunc>
void UMyDotaInputComponent::BindAbilityInputAction(const UDataAsset_InputConfig* InInputConfig,
	UserObject* ContextOnject, CallbackFunc InputPressFunc, CallbackFunc InputReleaseFunc)
{
	checkf(InInputConfig, TEXT("Input config data asset is null, cant not proceed with binding"));

	for(const auto& AbilityActionConfig : InInputConfig->AbilityInputActions)
	{
		if(!AbilityActionConfig.IsValid()) continue;

		BindAction(AbilityActionConfig.InputAction, ETriggerEvent::Started, ContextOnject, InputPressFunc, AbilityActionConfig.InputTag);
		BindAction(AbilityActionConfig.InputAction, ETriggerEvent::Completed, ContextOnject, InputReleaseFunc, AbilityActionConfig.InputTag);
	}
}
