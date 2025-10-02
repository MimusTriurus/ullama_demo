// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/UserData.h"
// ## You are interacting with the user described below:<user_description>- **Name**: Alex Petrov- **Sex**: Male- **Age**: 30- **Emotion**: Neutral </user_description>
FString UUserBioDataHelper::UserBioDataToString(FUserBioData Data)
{
	FString Output = TEXT("## You are interacting with the user described below:\n<user_description>\n");

	Output += FString::Printf(TEXT("Name: %s %s\n"), *Data.FirstName, *Data.LastName);
	Output += FString::Printf(TEXT("Age: %d\n"), Data.Age);
	const UEnum* EnumPtr = StaticEnum<ESex>();
	if (EnumPtr)
	{
		FText DisplayText = EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Data.Gender));
		FString DisplayName = DisplayText.ToString();
		Output += FString::Printf(TEXT("Gender: %s\n"), *DisplayName);
	}
	if (Data.CustomAttributes.Num() > 0)
	{
		Output += TEXT("Custom Attributes:\n");
		for (const auto& Pair : Data.CustomAttributes)
		{
			Output += FString::Printf(TEXT(" - %s: %s\n"), *Pair.Key, *Pair.Value);
		}	
	}
	Output += TEXT("</user_description>");
	return Output;
}
