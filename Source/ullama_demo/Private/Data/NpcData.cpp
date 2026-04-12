#include "Data/NpcData.h"

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

FString UNPCDataAsset::ToJson() const
{
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());

    RootObject->SetStringField(TEXT("Id"), this->Id.ToString());
    RootObject->SetStringField(TEXT("Name"), this->Name.ToString());
    RootObject->SetStringField(TEXT("Description"), this->Description.ToString());

    TArray<TSharedPtr<FJsonValue>> ActionsArray;
    for (const FNpcActionTemplate& Action : this->ActionData)
    {
        TSharedPtr<FJsonObject> ActionObj = MakeShareable(new FJsonObject());

        ActionObj->SetStringField(TEXT("ActionTemplate"), Action.ActionTemplate);
        ActionObj->SetStringField(TEXT("RequestTemplate"), Action.RequestTemplate);
        ActionObj->SetStringField(TEXT("UsrStateTemplate"), Action.UsrStateTemplate);
        ActionObj->SetStringField(TEXT("NpcStateTemplate"), Action.NpcStateTemplate);

        TSharedPtr<FJsonObject> ParamsObj = MakeShareable(new FJsonObject());
        for (const auto& ParamPair : Action.Parameters)
        {
            const FString& ParamKey = ParamPair.Key;
            const FArrayOfString& ArrayOfStrings = ParamPair.Value;

            TArray<TSharedPtr<FJsonValue>> StringValues;
            for (const FString& Str : ArrayOfStrings.Strings)
            {
                StringValues.Add(MakeShareable(new FJsonValueString(Str)));
            }
            ParamsObj->SetArrayField(ParamKey, StringValues);
        }
        ActionObj->SetObjectField(TEXT("Parameters"), ParamsObj);

        ActionsArray.Add(MakeShareable(new FJsonValueObject(ActionObj)));
    }
    RootObject->SetArrayField(TEXT("ActionData"), ActionsArray);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
    return OutputString;
}

UULlamaActionHandlerBase* UNPCDataAsset::GetActionByName(const FString& ActionName)
{
    for (const FNpcActionTemplate& Action : this->ActionData)
    {
        if (Action.ActionTemplate.StartsWith(ActionName))
        {
            return Action.ActionHandler;
        }
    }
    return nullptr;
}

UKnowledgeBaseDataGetterBase* UNPCDataAsset::GetDataGetterByName(const FString& DataGetterName)
{
    for (const FNpcActionTemplate& Action : this->ActionData)
    {
        if (Action.ActionTemplate.StartsWith(DataGetterName))
        {
            return Action.DataGetter;
        }
    }
    return nullptr;
}

UNPCDataAsset* UNPCDataRegistry::GetNpcData(const FName& NpcName) const
{
    if (Npc.Contains(NpcName))
    {
        return Npc[NpcName];
    }
    return nullptr;
}

FString UNPCDataRegistry::GetNpcDataJson(const FName& NpcName) const
{
    auto npc = GetNpcData(NpcName);
    if (npc)
    {
        return npc->ToJson();
    }
    return FString();
}
