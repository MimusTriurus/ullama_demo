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
struct ULLAMA_DEMO_API FKnowledgeBaseData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine=true))
	FText Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UKnowledgeBaseDataGetterBase* DataGetter{nullptr};
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
	FString GetKnowledgeBaseData(const FName& NpcName, int32 Idx, UObject* PlayerController = nullptr) const;

	UFUNCTION(BlueprintCallable, Category = "NpcKnowledgeBase")
	TArray<FString> GetNpcKnowledgeBaseDataSummaries(const FName& NpcName) const;
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
