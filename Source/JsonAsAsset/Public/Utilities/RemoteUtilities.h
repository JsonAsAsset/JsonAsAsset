/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Utilities/Compatibility.h"

#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

class FRemoteUtilities {
public:
#if ENGINE_UE5
	static TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> ExecuteRequestSync(TSharedRef<IHttpRequest> HttpRequest, float LoopDelay = 0.02);
#else
	static TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> ExecuteRequestSync(const TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& HttpRequest, float LoopDelay = 0.02);
#endif
};
