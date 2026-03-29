#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ULlamaActionHandler.h"
#include "ULlamaStructures.h"
#include "NpcData.generated.h"

USTRUCT(BlueprintType)
struct FNpcData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine=true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LoraModelPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VoiceModelPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FActionData> ActionData;
};

USTRUCT(BlueprintType)
struct FArrayOfString
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC's template")
	TArray<FString> Strings;
};

USTRUCT(BlueprintType)
struct FNpcActionTemplate
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC's template")
	FString ActionTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC's template")
	FString RequestTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC's template")
	FString UsrStateTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC's template")
	FString NpcStateTemplate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC's template")
	TMap<FString, FArrayOfString> Parameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC's template")
	UULlamaActionHandlerBase* ActionHandler{nullptr};
};

USTRUCT(BlueprintType)
struct FNpcVoices
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC's template")
	USoundWave* Neutral{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC's template")
	USoundWave* Happy{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC's template")
	USoundWave* Sad{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC's template")
	USoundWave* Angry{nullptr};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC's template")
	USoundWave* Surprise{nullptr};
};

UCLASS(BlueprintType)
class ULLAMA_DEMO_API UNPCDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC's template")
	FName Id;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC's template")
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine=true), Category = "NPC's template")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC's template")
	TArray<FNpcActionTemplate> ActionData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC's template")
	FNpcVoices NpcVoices;

	UFUNCTION(BlueprintCallable, Category = "NPC's template")
	FString ToJson() const;
};

UCLASS(BlueprintType)
class ULLAMA_DEMO_API UNPCDataRegistry : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, UNPCDataAsset*> Npc;

	UFUNCTION(BlueprintCallable, Category = "NpcDataRegistry")
	UNPCDataAsset* GetNpcData(const FName& NpcName) const;

	UFUNCTION(BlueprintCallable, Category = "NpcDataRegistry")
	FString GetNpcDataJson(const FName& NpcName) const;
};


