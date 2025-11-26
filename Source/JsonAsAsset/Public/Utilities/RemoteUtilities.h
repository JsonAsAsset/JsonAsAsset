/* Copyright JsonAsAsset Contributors 2024-2025 */

#pragma once

#include "Utilities/Compatibility.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

class JSONASASSET_API FRemoteUtilities {
public:
	static TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> ExecuteRequestSync(

		/* Different type declarations for HttpRequest on UE5 */
#if ENGINE_UE5
		TSharedRef<IHttpRequest> HttpRequest,
#else
		const TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& HttpRequest,
#endif
		/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
		
		float LoopDelay = 0.02);
};