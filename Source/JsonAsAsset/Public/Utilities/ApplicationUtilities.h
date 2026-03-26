/* Copyright JsonAsAsset Contributors 2024-2026 */

#pragma once

#ifndef __linux__
#include "Windows/WindowsHWrapper.h"
#include "TlHelp32.h"
#endif // __linux__

inline void SpawnPrompt(const FString& Title, const FString& Text) {
	FText DialogTitle = FText::FromString(Title);
	const FText DialogMessage = FText::FromString(Text);

	FMessageDialog::Open(EAppMsgType::Ok, DialogMessage);
}

inline auto SpawnYesNoPrompt = [](const FString& Title, const FString& Text, const TFunction<void(bool)>& OnResponse) {
	const FText DialogTitle = FText::FromString(Title);
	const FText DialogMessage = FText::FromString(Text);

#if UE5_3_BEYOND
	const EAppReturnType::Type Response = FMessageDialog::Open(EAppMsgType::YesNo, DialogMessage, DialogTitle);
#else
	const EAppReturnType::Type Response = FMessageDialog::Open(EAppMsgType::YesNo, DialogMessage, &DialogTitle);
#endif
	
	OnResponse(Response == EAppReturnType::Yes);
};

#ifndef __linux__
inline void CloseApplicationByProcessName(const FString& ProcessName) {
	DWORD ProcessID = 0;

	const HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (Snapshot != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 ProcessEntry;
		ProcessEntry.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(Snapshot, &ProcessEntry)) {
			do {
				if (FCString::Stricmp(ProcessEntry.szExeFile, ProcessName.GetCharArray().GetData()) == 0) {
					ProcessID = ProcessEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(Snapshot, &ProcessEntry));
		}
		
		CloseHandle(Snapshot);
	}

	if (ProcessID != 0) {
		const HANDLE Process = OpenProcess(PROCESS_TERMINATE, false, ProcessID);
		
		if (Process != nullptr) {
			TerminateProcess(Process, 0);
			CloseHandle(Process);
		}
	}
}

inline bool IsProcessRunning(const FString& ProcessName) {
	bool IsRunning = false;

	/* Convert FString to WCHAR */
	const TCHAR* ProcessNameChar = *ProcessName;

	const HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Snapshot != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 ProcessEntry;
		ProcessEntry.dwSize = sizeof(ProcessEntry);

		if (Process32First(Snapshot, &ProcessEntry)) {
			do {
				if (_wcsicmp(ProcessEntry.szExeFile, ProcessNameChar) == 0) {
					IsRunning = true;
					break;
				}
			} while (Process32Next(Snapshot, &ProcessEntry));
		}

		CloseHandle(Snapshot);
	}

	return IsRunning;
}
#endif // __linux__

#ifdef __linux__
// hacky, just assume cloud is running.
inline bool IsProcessRunning(const FString& ProcessName) {
	return true;
}
#endif // __linux__

inline void LaunchURL(const FString& URL) {
	FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
}