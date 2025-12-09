#include "Data/KnowledgeBaseData.h"

FString UKnowledgeBaseDataGetterBase::Get_Implementation()
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

FString UNpcKnowledgeBaseDataRegistry::GetKnowledgeBaseData(const FName& NpcName, int32 Idx) const
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
				Result = KBD.DataGetter->Get();
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
