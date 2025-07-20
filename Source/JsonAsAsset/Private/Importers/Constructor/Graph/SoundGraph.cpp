/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Importers/Constructor/Graph/SoundGraph.h"

#include "AssetToolsModule.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "IAssetTools.h"
#include "Misc/MessageDialog.h"
#include "Sound/SoundCue.h"
#include "Settings/JsonAsAssetSettings.h"

void ISoundGraph::ConstructNodes(USoundCue* SoundCue, TArray<TSharedPtr<FJsonValue>> JsonArray, TMap<FString, USoundNode*>& OutNodes) {
	for (const TSharedPtr<FJsonValue> JsonValue : JsonArray) {
		const TSharedPtr<FJsonObject> CurrentNodeObject = JsonValue->AsObject();

		if (!CurrentNodeObject->HasField(TEXT("Type"))) {
			continue;
		}
		
		FString NodeType = CurrentNodeObject->GetStringField(TEXT("Type"));
		FString NodeName = CurrentNodeObject->GetStringField(TEXT("Name"));

		/* Filter only exports with SoundNode at the start */
		if (NodeType.StartsWith("SoundNode")) {
			USoundNode* SoundCueNode = CreateEmptyNode(FName(*NodeName), FName(*NodeType), SoundCue);

			OutNodes.Add(NodeName, SoundCueNode);
		}
	}
}

USoundNode* ISoundGraph::CreateEmptyNode(FName Name, const FName Type, USoundCue* SoundCue) {
	UClass* Class = FindObject<UClass>(ANY_PACKAGE, *Type.ToString());

	/* TODO: Construct the sound node manually to have the exact same object name */
	return SoundCue->ConstructSoundNode<USoundNode>(
		Class,
		false
	);
}

void ISoundGraph::SetupNodes(USoundCue* SoundCueAsset, TMap<FString, USoundNode*> SoundCueNodes, TArray<TSharedPtr<FJsonValue>> JsonObjectArray) const {
	auto MainJsonObject = JsonObjectArray[0]->AsObject();
	auto MainJsonObjectProperties = MainJsonObject->TryGetField(TEXT("Properties"))->AsObject();

	/* If Node is connected to Root Node */
	if (MainJsonObjectProperties->HasField(TEXT("FirstNode"))) {
		auto FirstNodeProp = MainJsonObjectProperties->TryGetField(TEXT("FirstNode"))->AsObject();
		auto FirstNodeName = FirstNodeProp->TryGetField(TEXT("ObjectName"))->AsString();

		int32 ColonIndex = FirstNodeName.Find(TEXT(":"));
		int32 QuoteIndex = FirstNodeName.Find(TEXT("'"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		FString ChildNodeName = FirstNodeName.Mid(ColonIndex + 1, QuoteIndex - ColonIndex - 1);

		USoundNode** FirstNode = SoundCueNodes.Find(ChildNodeName);
		UEdGraphNode* RootNode = SoundCueAsset->SoundCueGraph->Nodes[0];

		/* Connect Node to Root Node */
		if (FirstNode && RootNode) {
			ConnectEdGraphNode((*FirstNode)->GetGraphNode(), RootNode, 0);
		}
	}

	/* Connections done here */
	for (TSharedPtr<FJsonValue> JsonValue : JsonObjectArray) {
		TSharedPtr<FJsonObject> CurrentNodeObject = JsonValue->AsObject();

		if (!CurrentNodeObject->HasField(TEXT("Type"))) {
			continue;
		}
		
		FString NodeType = CurrentNodeObject->GetStringField(TEXT("Type"));
		FString NodeName = CurrentNodeObject->GetStringField(TEXT("Name"));

		/* Make sure it has Properties and it's a SoundNode */
		if (!CurrentNodeObject->HasField(TEXT("Properties")) || !NodeType.StartsWith("SoundNode")) {
			continue;
		}

		TSharedPtr<FJsonObject> NodeProperties = CurrentNodeObject->TryGetField(TEXT("Properties"))->AsObject();

		USoundNode** CurrentNode = SoundCueNodes.Find(NodeName);
		USoundNode* Node = *CurrentNode;
		
		/* Filter only node with ChildNodes and handle the pins */
		if (NodeProperties->HasField(TEXT("ChildNodes"))) {
			TArray<TSharedPtr<FJsonValue>> CurrentNodeChildNodes = NodeProperties->TryGetField(TEXT("ChildNodes"))->AsArray();

			/* Save an index of the current connection */
			int32 ConnectionIndex = 0;

			for (TSharedPtr<FJsonValue> CurrentNodeValue : CurrentNodeChildNodes) {
				auto CurrentNodeChildNode = CurrentNodeValue->AsObject();

				/* Insert a child node if it doesn't exist */
				if (!Node->ChildNodes.IsValidIndex(ConnectionIndex)) {
					Node->InsertChildNode(ConnectionIndex);
				}

				if (CurrentNodeChildNode->HasField(TEXT("ObjectName"))) {
					auto CurrentChildNodeObjectName = CurrentNodeChildNode->TryGetField(TEXT("ObjectName"))->AsString();

					int32 ColonIndex = CurrentChildNodeObjectName.Find(TEXT(":"));
					int32 QuoteIndex = CurrentChildNodeObjectName.Find(TEXT("'"), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
					FString CurrentChildNodeName = CurrentChildNodeObjectName.Mid(ColonIndex + 1, QuoteIndex - ColonIndex - 1);

					USoundNode** CurrentChildNode = SoundCueNodes.Find(CurrentChildNodeName);
					int CurrentPin = ConnectionIndex + 1;

					/* Connect it */
					if (CurrentNode && CurrentChildNode) {
						ConnectSoundNode(*CurrentChildNode, *CurrentNode, CurrentPin);
					}
				}
				
				ConnectionIndex++;
			}
		}

		/* Deserialize Node Properties */
		GetObjectSerializer()->DeserializeObjectProperties(RemovePropertiesShared(NodeProperties, TArray<FString>
		{
			"ChildNodes"
		}), *CurrentNode);

		/* Import Sound Wave */
		if (Cast<USoundNodeWavePlayer>(Node) != nullptr) {
			auto WavePlayerNode = Cast<USoundNodeWavePlayer>(Node);

			if (NodeProperties->HasField(TEXT("SoundWaveAssetPtr"))) {
				FString AssetPtr = NodeProperties->TryGetField(TEXT("SoundWaveAssetPtr"))->AsObject()->GetStringField(TEXT("AssetPathName"));

				USoundWave* SoundWave = Cast<USoundWave>(StaticLoadObject(USoundWave::StaticClass(), nullptr, *AssetPtr));
				
				/* Already exists */
				if (SoundWave != nullptr) {
					WavePlayerNode->SetSoundWave(SoundWave);
				} else {
					/* Get URL from Settings */
					const UJsonAsAssetSettings* Settings = GetDefault<UJsonAsAssetSettings>();

					/* Import SoundWave */
					FString AudioURL = FString::Format(*(Settings->CloudURL + "/api/export?raw=false&path={0}"), { AssetPtr });
					FString AbsoluteSavePath = FString::Format(*("{0}Cache/{1}." + Settings->AssetSettings.SoundImportSettings.AudioFileExtension), { FPaths::ProjectDir(), FPaths::GetBaseFilename(AssetPtr) });

					ImportSoundWave(AudioURL, AbsoluteSavePath, AssetPtr, WavePlayerNode);
				}
			}
		}
	}
}

void ISoundGraph::ConnectEdGraphNode(UEdGraphNode* NodeToConnect, UEdGraphNode* NodeToConnectTo, int Pin = 1) {
	NodeToConnect->Pins[0]->MakeLinkTo(NodeToConnectTo->Pins[Pin]);
}

void ISoundGraph::ConnectSoundNode(const USoundNode* NodeToConnect, const USoundNode* NodeToConnectTo, int Pin = 1) {
	if (NodeToConnectTo->GetGraphNode()->Pins.IsValidIndex(Pin))
	{
		NodeToConnect->GetGraphNode()->Pins[0]->MakeLinkTo(NodeToConnectTo->GetGraphNode()->Pins[Pin]);
	}
}

void ISoundGraph::ImportSoundWave(const FString& URL, FString SavePath, FString AssetPtr, USoundNodeWavePlayer* Node) const {
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	
	HttpRequest->OnProcessRequestComplete().BindLambda([this, SavePath, AssetPtr, Node](const FHttpRequestPtr& Request, const FHttpResponsePtr& Response, const bool bWasSuccessful)
	{
		OnDownloadSoundWave(Request, Response, bWasSuccessful, SavePath, AssetPtr, Node);
	});

	HttpRequest->SetURL(URL);
	HttpRequest->SetVerb("GET");
	HttpRequest->ProcessRequest();
}

void ISoundGraph::OnDownloadSoundWave(FHttpRequestPtr Request, const FHttpResponsePtr& Response, bool bWasSuccessful, FString SavePath, FString AssetPtr, USoundNodeWavePlayer* Node) {
	if (bWasSuccessful && Response.IsValid()) {
		FFileHelper::SaveArrayToFile(Response->GetContent(), *SavePath);

		if (!FPaths::FileExists(SavePath)) {
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Format(TEXT("Failed To Find File {0} In Cache!"), { SavePath })));
			return;
		}

		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		UAutomatedAssetImportData* ImportData = NewObject<UAutomatedAssetImportData>();
		ImportData->Filenames.Add(SavePath);
		ImportData->DestinationPath = FPaths::GetPath(AssetPtr);
		ImportData->bReplaceExisting = true;
		
		auto AssetsImported = AssetTools.ImportAssetsAutomated(ImportData);
		if (!AssetsImported.IsValidIndex(0)) {
			USoundWave* SoundWave = Cast<USoundWave>(StaticLoadObject(USoundWave::StaticClass(), nullptr, *AssetPtr));
			Node->SetSoundWave(SoundWave);

			return;
		}
		
		USoundWave* ImportedWave = Cast<USoundWave>(AssetsImported[0]);

		if (!ImportedWave) {
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Format(TEXT("Failed To Import Wave {0}!"), { AssetPtr })));
			return;
		}

		Node->SetSoundWave(ImportedWave);
	} else {
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Format(TEXT("Failed To Download Audio {0}!"), { SavePath })));
	}
}