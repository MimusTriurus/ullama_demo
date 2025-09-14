#include "Data/KnowledgeBaseData.h"

UDataTable* UNpcDataTableRegistry::GetTableByKey(FName TableKey) const
{
	if (UDataTable* const* Found = DataTables.Find(TableKey))
	{
		return *Found;
	}
	return nullptr;
}

TArray<FString> UKnowledgeBaseDataHelper::UKbGetSummaries(UDataTable* DataTable)
{
	TArray<FKnowledgeBaseData*> KnowledgeBaseDatas;
	DataTable->GetAllRows(TEXT("==> UKbGetSummaries: Load KB datas"), KnowledgeBaseDatas);
	TArray<FString> Summaries;
	for (int32 i = 0; i < KnowledgeBaseDatas.Num(); ++i)
	{
		Summaries.Emplace(KnowledgeBaseDatas[i]->Summary.ToString());
	}
	return Summaries;
}

bool UKnowledgeBaseDataHelper::UKbGetData(UDataTable* DataTable, int32 Index, FKnowledgeBaseData& KbData)
{
	TArray<FKnowledgeBaseData*> KnowledgeBaseDatas;
	DataTable->GetAllRows(TEXT("==> UKbGetSummaries: Load KB datas"), KnowledgeBaseDatas);
	if (Index >= 0 && Index < KnowledgeBaseDatas.Num())
	{
		KbData = *KnowledgeBaseDatas[Index];
		return true;
	}
	return false;
}
