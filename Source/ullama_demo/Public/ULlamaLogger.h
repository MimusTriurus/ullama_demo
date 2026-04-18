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
	UFUNCTION(BlueprintCallable, Category = "ENpcEmotions")
	static void PrintUsrRequest(const FUserRequestData& UsrRequestData);

	UFUNCTION(BlueprintCallable, Category = "ENpcEmotions")
	static void PrintNpcResponse(const FLlmResponseData& NpcResponseData);
};
