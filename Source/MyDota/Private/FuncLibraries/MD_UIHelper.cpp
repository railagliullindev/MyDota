// Rail Agliullin Dev. All Rights Reserved

#include "FuncLibraries/MD_UIHelper.h"

FString UMD_UIHelper::FormatTime(const int32 TotalSeconds)
{
	const int32 Minutes = TotalSeconds / 60;
	const int32 Seconds = TotalSeconds % 60;

	return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
}