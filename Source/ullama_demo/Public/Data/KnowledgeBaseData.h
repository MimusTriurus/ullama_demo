#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "KnowledgeBaseData.generated.h"

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class ULLAMA_DEMO_API UKnowledgeBaseDataGetterBase : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="NpcKnowledgeBase")
	FString Get();

	virtual FString Get_Implementation();
};

USTRUCT(BlueprintType)
struct ULLAMA_DEMO_API FKnowledgeBaseData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine=true))
	FText Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UKnowledgeBaseDataGetterBase* DataGetter;
};

UCLASS(BlueprintType)
class UNpcKnowledgeBaseData : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FKnowledgeBaseData> KnowledgeBaseDatasTemplates;
};

UCLASS(BlueprintType)
class UNpcKnowledgeBaseDataRegistry : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, UNpcKnowledgeBaseData*> KnowledgeBases;
	
	UFUNCTION(BlueprintCallable, Category = "NpcKnowledgeBase")
	UNpcKnowledgeBaseData* GetNpcKnowledgeBaseData(const FName& NpcName) const;

	UFUNCTION(BlueprintCallable, Category = "NpcKnowledgeBase")
	FString GetKnowledgeBaseData(const FName& NpcName, int32 Idx) const;

	UFUNCTION(BlueprintCallable, Category = "NpcKnowledgeBase")
	TArray<FString> GetNpcKnowledgeBaseDataSummaries(const FName& NpcName) const;
};
