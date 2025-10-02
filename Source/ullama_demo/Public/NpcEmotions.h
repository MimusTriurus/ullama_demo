#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NpcEmotions.generated.h"

UENUM(BlueprintType)
enum class ENpcEmotion : uint8
{
	None	 = 99  UMETA(DisplayName = "None"),
	Neutral	 = 0   UMETA(DisplayName = "Neutral"),
	Angry	 = 1   UMETA(DisplayName = "Angry"),
	Happy	 = 2   UMETA(DisplayName = "Happy"),
	Sad		 = 3   UMETA(DisplayName = "Sad"),
	Surprise = 4   UMETA(DisplayName = "Surprise")
};

UCLASS()
class UNpcEmotionsHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "ENpcEmotions")
	static FString EmotionEnumToString(ENpcEmotion EnumValue);

	UFUNCTION(BlueprintCallable, Category = "ENpcEmotions")
	static ENpcEmotion StringToEmotionEnum(const FString& InString);

	UFUNCTION(BlueprintPure, Category = "ENpcEmotions")
	static TArray<FString> EmotionEnumToArray();
};
