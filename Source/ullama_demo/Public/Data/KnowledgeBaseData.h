#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "KnowledgeBaseData.generated.h"

USTRUCT(BlueprintType)
struct ULLAMA_DEMO_API FKnowledgeBaseData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine=true))
	FText Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine=true))
	FText FullData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Triggers;
};
