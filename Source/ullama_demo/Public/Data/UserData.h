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


UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class UUserStateGetterBase : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UserState")
	FString Get();

	virtual FString Get_Implementation() const
	{
		return FString();
	}
};

UCLASS(BlueprintType)
class UUserStateDataRegistry : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, UUserStateGetterBase*> UsersStates;

	UFUNCTION(BlueprintCallable, Category = "UserState") const
	FString GetStateByUserName(FName Name)
	{
		if (UsersStates.Contains(Name))
		{
			return UsersStates[Name]->Get();
		}
		return FString();
	}
};



