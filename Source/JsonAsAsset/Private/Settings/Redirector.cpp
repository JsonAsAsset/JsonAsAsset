/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Settings/Redirector.h"

#include "Settings/JsonAsAssetSettings.h"
#include "Settings/Runtime.h"
#include "Utilities/EngineUtilities.h"

/************************************
 **** Redirect History ************ */
TMap<FString, TArray<FJRedirectorPoint>> FJRedirects::History;

bool FJRedirector::IsEnabled() const {
	bool bIsEnabled = Enable;

	if (!GJsonAsAssetRuntime.Profile.Name.IsEmpty()) {
		if (Profiles.Num() > 0 && !Profiles.Contains(GJsonAsAssetRuntime.Profile.Name)) {
			bIsEnabled = false;
		}
	}

	/* If there are specific profiles that go with this redirector, and the cloud is disabled, don't use any. */
	if (!GetSettings()->bEnableCloudServer && Profiles.Num() > 0) {
		bIsEnabled = false;
	}
	
	return bIsEnabled;
}

void FJRedirects::Clear() {
	History.Empty();
}

void FJRedirects::Redirect(FString& Path) {
	TArray<FJRedirectorPoint> Points;

	const UJsonAsAssetSettings* Settings = GetSettings();

	for (const FJRedirector& Redirect : Settings->Redirectors) {
		if (!Redirect.IsEnabled()) continue;

		for (const FJRedirectorPoint& Point : Redirect.Points) {
			if (Path.Contains(Point.From)) {
				Points.Add(Point);
				
				Path = Path.Replace(*Point.From, *Point.To);
			}
		}
	}

	TArray<FJRedirectorPoint>& Pointers = History.FindOrAdd(Path);
	Pointers.Append(Points);
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
