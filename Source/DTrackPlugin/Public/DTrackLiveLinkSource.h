// Copyright (c) 2019, Advanced Realtime Tracking GmbH
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of copyright holder nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "ILiveLinkSource.h"
#include "DTrackLiveLinkTypes.h"
#include "DTrackLiveLinkSourceSettings.h"

#include "Runtime/Launch/Resources/Version.h" 


class FDTrackSDKHandler;
class ILiveLinkClient;

class DTRACKPLUGIN_API FDTrackLiveLinkSource : public ILiveLinkSource
{
public:

	FDTrackLiveLinkSource();

public:

	//~ Begin ILiveLinkSource implementation
	virtual void ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) override;
	virtual void InitializeSettings(ULiveLinkSourceSettings* Settings) override;
	virtual bool IsSourceStillValid() const override;
	virtual bool RequestSourceShutdown() override;
	virtual FText GetSourceType() const override;
	virtual FText GetSourceMachineName() const override;
	virtual FText GetSourceStatus() const override;
	
#if ENGINE_MAJOR_VERSION == 5 || ( ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION>=24 )
	virtual TSubclassOf<ULiveLinkSourceSettings>  GetSettingsClass() const override;
#else
	virtual UClass* GetCustomSettingsClass() const;
#endif

	virtual void OnSettingsChanged(ULiveLinkSourceSettings* InSettings, const FPropertyChangedEvent& InPropertyChangedEvent) override;
	//~ End ILiveLinkSource

	void handle_body_data_anythread(double n_worldtime, double n_timestamp, int32 n_itemId, float n_quality, const FVector& n_location, const FRotator& n_rotation);
	void handle_flystick_data_anythread(double n_worldtime, double n_timestamp, int32 n_itemId, float n_quality, const FVector& n_location, const FRotator& n_rotation, TArray<bool>& n_temp_buttons, TArray<float>& n_temp_joysticks);

protected:

	void reset_datamaps();
	void handle_flystick_body_anythread(double n_worldtime, double n_timestamp, int32 n_itemId, float n_quality, const FVector& n_location, const FRotator& n_rotation);
	void handle_flystick_input_anythread(double n_worldtime, double n_timestamp, int32 n_itemId, TArray<bool>& n_temp_buttons, TArray<float>& n_temp_joysticks);

	
private:

	// Current server settings to detect if they changed when callback for property change
	FDTrackServerSettings m_current_server_settings;

	// Local pointer to LiveLinkClient
	ILiveLinkClient* m_client;

	// Our identifier in LiveLink
	FGuid m_source_guid;

	// Access to DTrack sdk
	TUniquePtr<FDTrackSDKHandler> m_sdk_handler;

	// Settings instance that we've been assigned
	TWeakObjectPtr<UDTrackLiveLinkSourceSettings> m_source_settings;

	// Reception of data can happen in any thread. Static data maps will be accessed. They can also be accessed in game thread so we need to protected
	mutable FCriticalSection m_data_access_criticalsection;

	// Maps to keep track of subjects that were added and associated static data to know when it changed
	TMap<int32, FLiveLinkSubjectKey> m_body_subjects;
	TMap<int32, FLiveLinkSubjectKey> m_flystick_body_subjects;
	TMap<int32, FLiveLinkSubjectKey> m_flystick_input_subjects;
	TMap<FName, FDTrackFlystickInputStaticData> m_flystick_input_static_data_map;

};