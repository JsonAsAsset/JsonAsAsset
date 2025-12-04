/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Validation.generated.h"

UINTERFACE()
class UJsonAsAssetValidationInterface : public UInterface {
	GENERATED_BODY()
};

/* Interface for any settings object that can be validated */
class IJsonAsAssetValidationInterface {
	GENERATED_BODY()
public:
	/* Should return true if valid. Should fill Reasons with any failures. */
	virtual bool IsValid(TArray<FString>& InvalidReasons) const { return true; };

	/* Convenience */
	virtual bool IsValid() const {
		TArray<FString> Dummy;
		
		return IsValid(Dummy);
	}
};