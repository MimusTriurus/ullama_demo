// Fill out your copyright notice in the Description page of Project Settings.


#include "ULlamaLogger.h"

#include "ULlamaStructures.h"

DEFINE_LOG_CATEGORY(LogULlama);

void UULlamaLogger::PrintUsrRequest(const FUserRequestData& UsrRequestData)
{
	UE_LOG(
		LogULlama,
		Display,
		TEXT("===\nRequest: %s\nNpcState: %s\nUserState: %s\n==="),
		*UsrRequestData.RequestOfUser,
		*UsrRequestData.Context,
		*UsrRequestData.StateOfUser
	);
}

void UULlamaLogger::PrintNpcResponse(const FLlmResponseData& NpcResponseData)
{
	UE_LOG(
		LogULlama,
		Display,
		TEXT("PrintNpcResponse")
	);
}
