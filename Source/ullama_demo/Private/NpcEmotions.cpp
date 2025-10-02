#include "NpcEmotions.h"

FString UNpcEmotionsHelpers::EmotionEnumToString(ENpcEmotion EnumValue)
{
	const UEnum* EnumPtr = StaticEnum<ENpcEmotion>();
	if (!EnumPtr)
	{
		return FString("Invalid");
	}
	// FText Display = EnumPtr->GetDisplayNameTextByValue((int64)EnumValue);
	// return Display.ToString();
	return EnumPtr->GetNameStringByValue((int64)EnumValue);
}

ENpcEmotion UNpcEmotionsHelpers::StringToEmotionEnum(const FString& InString)
{
	const UEnum* EnumPtr = StaticEnum<ENpcEmotion>();
	if (!EnumPtr)
	{
		return ENpcEmotion::None;
	}
	for (int32 Index = 0; Index < EnumPtr->NumEnums(); ++Index)
	{
		FText DisplayText = EnumPtr->GetDisplayNameTextByIndex(Index);
		if (!DisplayText.IsEmpty() && DisplayText.ToString().Equals(InString, ESearchCase::IgnoreCase))
		{
			int64 Value = EnumPtr->GetValueByIndex(Index);
			return static_cast<ENpcEmotion>(Value);
		}
	}
	for (int32 Index = 0; Index < EnumPtr->NumEnums(); ++Index)
	{
		FString NameStr = EnumPtr->GetNameStringByIndex(Index);
		if (NameStr.Equals(InString, ESearchCase::IgnoreCase))
		{
			int64 Value = EnumPtr->GetValueByIndex(Index);
			return static_cast<ENpcEmotion>(Value);
		}
	}
	return ENpcEmotion::None;
}

TArray<FString> UNpcEmotionsHelpers::EmotionEnumToArray()
{
	TArray<FString> Result;
	const UEnum* EnumPtr = StaticEnum<ENpcEmotion>();
	if (!EnumPtr) return Result;

	for (int32 i = 1; i < EnumPtr->NumEnums() - 1; ++i)
	{
		Result.Add(EnumPtr->GetNameStringByIndex(i));
	}
	return Result;
}
