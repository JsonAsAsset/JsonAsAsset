/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Redirector.generated.h"

/* A point in a redirector */
USTRUCT()
struct FJRedirectorPoint {
	GENERATED_BODY()
public:
	/* Original path. (ex: /Plugin_Name/) */
	UPROPERTY(EditAnywhere, Config, Category = RedirectorPoint)
	FString From;

	/* Redirected path. (ex: /Plugin_Name/) */
	UPROPERTY(EditAnywhere, Config, Category = RedirectorPoint)
	FString To;
};

USTRUCT()
struct FJRedirector {
	GENERATED_BODY()
public:
	bool IsEnabled() const;
	
private:
	/* Profiles allowed when using this redirector. (Empty = Allowed By Default) */
	UPROPERTY(EditAnywhere, Config, Category = Metadata)
	TArray<FString> Profiles;
	
	/* The name of this redirector. */
	UPROPERTY(EditAnywhere, Config, Category = Redirector)
	FName Name;

public:
	/* The points on this redirector. */
	UPROPERTY(EditAnywhere, Config, Category = Redirector)
	TArray<FJRedirectorPoint> Points;

private:
	/* Enables this redirector. */
	UPROPERTY(EditAnywhere, Config, Category = Redirector)
	bool Enable = true;
};

/* Redirect Handler */
struct FJRedirects {
	static TMap<FString, TArray<FJRedirectorPoint>> History;

	static void Clear();
	static void Redirect(FString& Path);
	static void Reverse(FString& Path);
};