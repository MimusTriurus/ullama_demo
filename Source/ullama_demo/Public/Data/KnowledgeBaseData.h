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

UCLASS(BlueprintType)
class UNpcDataTableRegistry : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, UDataTable*> DataTables;

	UFUNCTION(BlueprintCallable, Category = "Data")
	UDataTable* GetTableByKey(FName TableKey) const;
};

UCLASS()
class UKnowledgeBaseDataHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "UKnowledgeBaseHelper")
	static TArray<FString> UKbGetSummaries(UDataTable* DataTable);

	UFUNCTION(BlueprintCallable, Category = "UKnowledgeBaseHelper")
	static bool UKbGetData(UDataTable* DataTable, int32 Index, UPARAM(ref) FKnowledgeBaseData& KbData);
};
