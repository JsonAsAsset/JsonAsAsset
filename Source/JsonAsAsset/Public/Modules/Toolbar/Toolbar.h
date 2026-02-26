/* Copyright JsonAsAsset Contributors 2024-2026 */

#pragma once

#include "CoreMinimal.h"
#include "Utilities/Compatibility.h"
#include "Toolbar.generated.h"

UCLASS()
class JSONASASSET_API UJsonAsAssetToolbar : public UObject {
	GENERATED_BODY()
public:
	void Register();
	void AddCloudButtons(FToolMenuSection& Section);
	
#if ENGINE_UE4
	static void UE4Register(FToolBarBuilder& Builder);
#endif

	/* Checks if JsonAsAsset is fit to function */
	void IsFitToFunction(TFunction<void(bool)> OnResponse);
	
	/* Checks if JsonAsAsset is fit to function, then called Import */
	void ImportAction();

	/* Opens a JSON file dialog */
	void Import();
	
	/* UI Display ~~~~~~~~~~~~~~ */
	static TSharedRef<SWidget> CreateMenuDropdown();
	static TSharedRef<SWidget> CreateCloudMenuDropdown();
	
	static bool IsToolBarVisible();

protected:
	/* Wait for Cloud to Initialize */
	void HandleCloudWaiting();

	FTimerHandle WaitForCloudTimer;
	int32 CloudDotCount = 0;

	void WaitForCloudTimerCallback();
	void CancelWaitForCloudTimer();
};
