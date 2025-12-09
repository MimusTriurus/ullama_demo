// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UserData.generated.h"

UENUM(BlueprintType)
enum class ESex : uint8
{
	Male        UMETA(DisplayName = "Male"),
	Female      UMETA(DisplayName = "Female")
};

USTRUCT(BlueprintType)
struct ULLAMA_DEMO_API FUserBioData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UserBioData")
	FString FirstName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UserBioData")
	FString LastName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UserBioData")
	int32 Age;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UserBioData")
	ESex Gender;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UserBioData")
	TMap<FString, FString> CustomAttributes;
};

UCLASS()
class UUserBioDataHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "UserBioDataHelper")
	static FString UserBioDataToString(FUserBioData Data);
};


