#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NpcData.generated.h"

USTRUCT(BlueprintType)
struct FNpcData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine=true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine=true))
	FText ChatExample;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LoraModelPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VoiceModelPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString KnowledgeBaseId;
};

