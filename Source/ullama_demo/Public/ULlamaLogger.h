// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ULlamaLogger.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogULlama, Display, All);

UCLASS()
class ULLAMA_DEMO_API UULlamaLogger : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "ULlamaLog")
	static void PrintUsrRequest(const FUserRequestData& UsrRequestData);

	UFUNCTION(BlueprintCallable, Category = "ULlamaLog")
	static void PrintNpcResponse(const FLlmResponseData& NpcResponseData);

	UFUNCTION(BlueprintCallable, Category = "ULlamaLog")
	static void PrintMessage(const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "ULlamaLog")
	static void PrintError(const FString& Error);

	UFUNCTION(BlueprintCallable, Category = "ULlamaLog")
	static void PrintWarning(const FString& Warning);
};
