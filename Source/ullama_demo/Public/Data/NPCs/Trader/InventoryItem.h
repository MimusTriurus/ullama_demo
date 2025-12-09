#pragma once

#include "InventoryItem.generated.h"

UENUM(BlueprintType)
enum class EItemTypeEnum : uint8
{
	Weapon UMETA(DisplayName = "Weapon"),
	Ammo UMETA(DisplayName = "Ammo"),
	Upgrade UMETA(DisplayName = "Upgrade"),
	Medication UMETA(DisplayName = "Medication"),
	Information UMETA(DisplayName = "Information"),

	All UMETA(DisplayName = "All"),

	Unknown UMETA(DisplayName = "Unknown"),
};
// start obsolete
USTRUCT(BlueprintType)
struct FItem : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Cost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine=true))
	FText Description;

	virtual FString ToString() const
	{
		return FString::Printf(
			TEXT("Title: %s\nPrice: %d\nDescription: %s"),
			*Title,
			Cost,
			*Description.ToString()
			);
	}
};

USTRUCT(BlueprintType)
struct FWeapon : public FItem
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AmmoType;
};

USTRUCT(BlueprintType)
struct FAmmo : public FItem
{
	GENERATED_USTRUCT_BODY()
};
// end obsolete


USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemTypeEnum ItemTypeEnum{EItemTypeEnum::Unknown};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Cost{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine=true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount{0};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAddItem, FInventoryItem, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDelItem, FInventoryItem, Item);

UCLASS(BlueprintType)
class UInventoryRegistry : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AvailableGold;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FInventoryItem> InventoryItems;

	UFUNCTION(BlueprintCallable, Category = "InventoryItems")
	TArray<FInventoryItem> GetItemsByType(EItemTypeEnum ItemType) const
	{
		TArray<FInventoryItem> Result;
		for (int32 i = 0; i < InventoryItems.Num(); i++)
		{
			if (InventoryItems[i].ItemTypeEnum == ItemType)
			{
				Result.Add(InventoryItems[i]);
			}
		}
		return Result;
	}

	UFUNCTION(BlueprintCallable, Category = "InventoryItems")
	bool RemoveItemByName(FName ItemName)
	{
		for (int32 i = 0; i < InventoryItems.Num(); i++)
		{
			if (InventoryItems[i].Name == ItemName)
			{
				if (InventoryItems[i].Amount)
				{
					InventoryItems[i].Amount -= 1;
					OnDelItem.Broadcast(InventoryItems[i]);
					return true;
				}
			}
		}
		return false;
	}

	UFUNCTION(BlueprintCallable, Category = "InventoryItems")
	bool AddItemByName(FName ItemName)
	{
		for (int32 i = 0; i < InventoryItems.Num(); i++)
		{
			if (InventoryItems[i].Name == ItemName)
			{
				InventoryItems[i].Amount += 1;
				OnAddItem.Broadcast(InventoryItems[i]);
				return true;
			}
		}
		return false;
	}

	UFUNCTION(BlueprintCallable, Category = "InventoryItems")
	bool AddItem(FInventoryItem Item)
	{
		if (!HasItem(Item.Name))
		{
			InventoryItems.Add(Item);
		}
		return AddItemByName(Item.Name);
	}

	UFUNCTION(BlueprintCallable, Category = "InventoryItems")
	FInventoryItem GetItemByName(FName ItemName) const
	{
		for (int32 i = 0; i < InventoryItems.Num(); i++)
		{
			if (InventoryItems[i].Name == ItemName)
			{
				return InventoryItems[i];
			}
		}
		return FInventoryItem();
	}

	UFUNCTION(BlueprintCallable, Category = "InventoryItems")
	bool HasItem(FName ItemName)
	{
		for (int32 i = 0; i < InventoryItems.Num(); i++)
		{
			if (InventoryItems[i].Name == ItemName)
			{
				return true;
			}
		}
		return false;
	}

	UFUNCTION(BlueprintCallable, Category = "InventoryItems")
	FString GetDescriptionByItemName(FName ItemName) const
	{
		for (int32 i = 0; i < InventoryItems.Num(); i++)
		{
			if (InventoryItems[i].Name == ItemName && InventoryItems[i].Amount > 0)
			{
				return FString::Printf(
					TEXT("%s\r\nItem cost: %d dollars"),
					*InventoryItems[i].Description.ToString(),
					InventoryItems[i].Cost
					);
			}
		}
		return FString();
	}

	UFUNCTION(BlueprintCallable, Category = "InventoryItems")
	FString ItemToString(const FItem& Item) const
	{
		return Item.ToString();
	}

	UFUNCTION(BlueprintCallable, Category = "InventoryItems")
	FString GetAvailableItemsByType(EItemTypeEnum ItemType)
	{
		FString Result;
		for (int32 i = 0; i < InventoryItems.Num(); i++)
		{
			if (InventoryItems[i].ItemTypeEnum == ItemType || ItemType == EItemTypeEnum::All)
			{
				if (InventoryItems[i].Amount > 0)
				{
					auto ItemTypeDisplayName = StaticEnum<EItemTypeEnum>()->GetDisplayNameTextByValue(static_cast<int64>(InventoryItems[i].ItemTypeEnum)).ToString();
					Result += FString::Printf(
						TEXT("Title: %s\nType: %s\nPrice: %d\n----\n\n"),
						*InventoryItems[i].Name.ToString(),
						*ItemTypeDisplayName,
						InventoryItems[i].Cost
					);
				}
			}
		}
		return Result;
	}

	FOnAddItem OnAddItem;
	FOnDelItem OnDelItem;
};

UCLASS(BlueprintType)
class UInventorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	UInventoryRegistry* InventoryRegistry;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};

inline void UInventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}
