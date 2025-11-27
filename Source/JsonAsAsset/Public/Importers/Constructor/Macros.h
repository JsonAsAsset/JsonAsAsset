/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Utilities/EngineUtilities.h"

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