/* Copyright JsonAsAsset Contributors 2024-2025 */

#include "Utilities/RemoteUtilities.h"

#include "HttpManager.h"
#include "HttpModule.h"
#include "Modules/LogCategory.h"
#include "Serialization/JsonSerializer.h"

#if ENGINE_UE5
TSharedPtr<IHttpResponse> FRemoteUtilities::ExecuteRequestSync(TSharedRef<IHttpRequest> HttpRequest, float LoopDelay)
#else
TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> FRemoteUtilities::ExecuteRequestSync(const TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& HttpRequest, float LoopDelay)
#endif
{
	const bool bStartedRequest = HttpRequest->ProcessRequest();
	if (!bStartedRequest)
	{
		UE_LOG(LogJsonAsAsset, Error, TEXT("Failed to start HTTP Request."));
		return nullptr;
	}

	double LastTime = FPlatformTime::Seconds();
	while (EHttpRequestStatus::Processing == HttpRequest->GetStatus())
	{
		const double AppTime = FPlatformTime::Seconds();
		FHttpModule::Get().GetHttpManager().Tick(AppTime - LastTime);
		LastTime = AppTime;
		FPlatformProcess::Sleep(LoopDelay);
	}

	return HttpRequest->GetResponse();
}
