/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#define REGISTER_IMPORTER(ImporterClass, AcceptedTypes, Category) \
namespace { \
    struct FAutoRegister_##ImporterClass { \
        FAutoRegister_##ImporterClass() { \
            FImporterRegistrationInfo Info(FString(Category), &CreateImporter<ImporterClass>); \
            GetFactoryRegistry().Add(AcceptedTypes, Info); \
        } \
    }; \
    static FAutoRegister_##ImporterClass AutoRegister_##ImporterClass; \
}

FORCEINLINE uint32 GetTypeHash(const TArray<FString>& Array) {
    uint32 Hash = 0;
    
    for (const FString& Str : Array) {
        Hash = HashCombine(Hash, GetTypeHash(Str));
    }
    
    return Hash;
}