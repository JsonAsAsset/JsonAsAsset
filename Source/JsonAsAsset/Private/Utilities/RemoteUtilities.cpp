/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Utilities/RemoteUtilities.h"

#include "HttpManager.h"
#include "HttpModule.h"
#include "Modules/Log.h"

#if ENGINE_UE5
TSharedPtr<IHttpResponse>
#else
TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>
#endif
	FRemoteUtilities::ExecuteRequestSync(

	/* Different type declarations for HttpRequest on UE5 */
#if ENGINE_UE5
	TSharedRef<IHttpRequest> HttpRequest
#else
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& HttpRequest
#endif
	/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	
	, const float LoopDelay)
{
	const bool bStartedRequest = HttpRequest->ProcessRequest();
	if (!bStartedRequest) {
		UE_LOG(LogJsonAsAsset, Error, TEXT("Failed to start HTTP Request."));
		return nullptr;
	}

	double LastTime = FPlatformTime::Seconds();
	while (EHttpRequestStatus::Processing == HttpRequest->GetStatus()) {
		const double AppTime = FPlatformTime::Seconds();
		FHttpModule::Get().GetHttpManager().Tick(AppTime - LastTime);
		
		LastTime = AppTime;
		FPlatformProcess::Sleep(LoopDelay);
	}

	return HttpRequest->GetResponse();
}
