/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Settings/Redirector.h"

#include "Settings/JsonAsAssetSettings.h"
#include "Utilities/EngineUtilities.h"

/************************************
 **** Redirect History ************ */
TMap<FString, TArray<FJRedirectorPoint>> FJRedirects::History;

void FJRedirects::Clear() {
	History.Empty();
}

void FJRedirects::Redirect(FString& Path) {
	TArray<FJRedirectorPoint> AppliedPointers;

	const UJsonAsAssetSettings* Settings = GetSettings();

	for (const FJRedirector& Redirect : Settings->Redirectors) {
		if (!Redirect.Enable) continue;

		for (const FJRedirectorPoint& Point : Redirect.Points) {
			if (Path.Contains(Point.From)) {
				AppliedPointers.Add(Point);
				
				Path = Path.Replace(*Point.From, *Point.To);
			}
		}
	}

	TArray<FJRedirectorPoint>& Pointers = History.FindOrAdd(Path);
	Pointers.Append(AppliedPointers);
}

void FJRedirects::Reverse(FString& Path) {
	TArray<FJRedirectorPoint>* Points = History.Find(Path);
	if (!Points) {
		return;
	}

	for (const FJRedirectorPoint Point : *Points) {
		Path = Path.Replace(*Point.To, *Point.From);
	}
}
