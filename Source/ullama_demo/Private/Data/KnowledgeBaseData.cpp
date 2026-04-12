#include "Data/KnowledgeBaseData.h"

FString UKnowledgeBaseDataGetterBase::Get_Implementation(UObject* VM, const FString& ActionName, const TMap<FString, FString>& Params)
{
	return FString();
}

UNpcKnowledgeBaseData* UNpcKnowledgeBaseDataRegistry::GetNpcKnowledgeBaseData(const FName& NpcName) const
{
	if (UNpcKnowledgeBaseData* const* Found = KnowledgeBases.Find(NpcName))
	{
		return *Found;
	}
	return nullptr;
}

FString UNpcKnowledgeBaseDataRegistry::GetKnowledgeBaseData(const FName& NpcName, int32 Idx, UObject* PlayerController) const
{
	FString Result;
	if (auto Found = KnowledgeBases.Find(NpcName))
	{
		UNpcKnowledgeBaseData* KB = *Found;
		if (KB->KnowledgeBaseDatasTemplates.IsValidIndex(Idx))
		{
			auto& KBD = KB->KnowledgeBaseDatasTemplates[Idx];
			if (KBD.DataGetter)
			{
				Result = KBD.DataGetter->Get(nullptr, "", {});
			}
			else
			{
				Result = TEXT("");
			}
		}
	}
	return Result;
}

TArray<FString> UNpcKnowledgeBaseDataRegistry::GetNpcKnowledgeBaseDataSummaries(const FName& NpcName) const
{
	TArray<FString> Summaries;
	if (auto Found = KnowledgeBases.Find(NpcName))
	{
		UNpcKnowledgeBaseData* KB = *Found;
		for (auto& Template : KB->KnowledgeBaseDatasTemplates)
		{
			Summaries.Add(Template.Summary.ToString());
		}
	}
	return Summaries;
}

bool UNpcKnowledgeBase::Init(FName NpcName)
{
    KnowledgeBaseDataGetters.Empty();
	
    FString FilePath = FPaths::ProjectDir() / TEXT("Resources/NPCs") / NpcName.ToString() / TEXT("knowledge_base.json");
    FPaths::NormalizeFilename(FilePath);

    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("UNpcKnowledgeBase::Init - failed to load file: %s"), *FilePath);
        return false;
    }

    TArray<TSharedPtr<FJsonValue>> JsonArray;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, JsonArray))
    {
        UE_LOG(LogTemp, Warning, TEXT("UNpcKnowledgeBase::Init - failed to parse JSON: %s"), *FilePath);
        return false;
    }

    for (const TSharedPtr<FJsonValue>& JsonValue : JsonArray)
    {
        const TSharedPtr<FJsonObject>* RecordObject;
        if (!JsonValue->TryGetObject(RecordObject))
            continue;

        FKnowledgeBaseRecord Record;

        (*RecordObject)->TryGetStringField(TEXT("request"), Record.Request);

        const TSharedPtr<FJsonObject>* ActionObject;
        if ((*RecordObject)->TryGetObjectField(TEXT("action"), ActionObject))
        {
            (*ActionObject)->TryGetStringField(TEXT("name"), Record.Action.Name);

            const TSharedPtr<FJsonObject>* ParamsObject;
            if ((*ActionObject)->TryGetObjectField(TEXT("parameters"), ParamsObject))
            {
                for (const auto& Pair : (*ParamsObject)->Values)
                {
                    FString ParamValue;
                    if (Pair.Value->TryGetString(ParamValue))
                    {
                        Record.Action.Parameters.Add(Pair.Key, ParamValue);
                    }
                }
            }
        }

        KnowledgeBaseDataGetters.Add(Record);
    }

    UE_LOG(LogTemp, Log, TEXT("UNpcKnowledgeBase::Init - loaded %d records for NPC '%s'"),
        KnowledgeBaseDataGetters.Num(), *NpcName.ToString());

    return KnowledgeBaseDataGetters.Num() > 0;
}

TArray<FString> UNpcKnowledgeBase::GetKnowledgeBaseDataSummaries() const
{
	TArray<FString> Summaries;

	for (const auto& Getter : KnowledgeBaseDataGetters)
	{
		Summaries.Add(Getter.Request);
	}
	return Summaries;
}

FNpcAction UNpcKnowledgeBase::GetNpcDataGetterAction(int32 Idx) const
{
	return KnowledgeBaseDataGetters[Idx].Action;
}
