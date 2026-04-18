#pragma once

#include "CoreMinimal.h"
#include "ULlamaStructures.h"
#include "UObject/Object.h"
#include "KnowledgeBaseData.generated.h"

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class ULLAMA_DEMO_API UKnowledgeBaseDataGetterBase : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="NpcKnowledgeBase")
	FString GetNpcState(UObject* VM, const FString& ActionName, const TMap<FString, FString>& Params);

	virtual FString GetNpcState_Implementation(UObject* VM, const FString& ActionName, const TMap<FString, FString>& Params);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="NpcKnowledgeBase")
	FString GetUserState(UObject* VM, const FString& ActionName, const TMap<FString, FString>& Params);

	virtual FString GetUserState_Implementation(UObject* VM, const FString& ActionName, const TMap<FString, FString>& Params);
};

USTRUCT(BlueprintType)
struct FKnowledgeBaseRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Request;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FNpcAction Action;
};

UCLASS(BlueprintType)
class UNpcKnowledgeBase : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "NpcKnowledgeBase")
	bool Init(FName NpcName);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FKnowledgeBaseRecord> KnowledgeBaseDataGetters;

	UFUNCTION(BlueprintCallable, Category = "NpcKnowledgeBase")
	TArray<FString> GetKnowledgeBaseDataSummaries() const;

	UFUNCTION(BlueprintCallable, Category = "NpcKnowledgeBase")
	FNpcAction GetNpcDataGetterAction(int32 Idx) const;
};
