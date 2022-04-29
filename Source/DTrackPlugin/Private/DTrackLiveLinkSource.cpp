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

#include "DTrackLiveLinkSource.h"

#include "DTrackPlugin.h"
#include "DTrackLiveLinkSourceSettings.h"
#include "DTrackSDKHandler.h"
#include "DTrackLiveLinkRole.h"
#include "ILiveLinkClient.h"
#include "Misc/App.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkTransformRole.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "Roles/LiveLinkTransformTypes.h"


#define LOCTEXT_NAMESPACE "DTrackLiveLinkSource"


FDTrackLiveLinkSource::FDTrackLiveLinkSource()
	: m_client(nullptr) {
}

void FDTrackLiveLinkSource::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) {

	m_client = InClient;
	m_source_guid = InSourceGuid;
	if (!m_sdk_handler.IsValid()) {
		m_sdk_handler = MakeUnique<FDTrackSDKHandler>(this);
	}
}

void FDTrackLiveLinkSource::InitializeSettings(ULiveLinkSourceSettings* InSettings) {

	m_source_settings = CastChecked<UDTrackLiveLinkSourceSettings>(InSettings);
	reset_datamaps();
	m_sdk_handler->start_listening(m_source_settings->m_server_settings);
}

bool FDTrackLiveLinkSource::IsSourceStillValid() const {

	return m_sdk_handler->is_active();
}

FText FDTrackLiveLinkSource::GetSourceStatus() const {

	return FText::Format(LOCTEXT("SourceStatus", "{0}"), FText::FromString(m_sdk_handler->get_status()));
}



#if ENGINE_MINOR_VERSION >= 24
	TSubclassOf<ULiveLinkSourceSettings>  FDTrackLiveLinkSource::GetSettingsClass() const {
		return UDTrackLiveLinkSourceSettings::StaticClass();
	}
#else	
	UClass* FDTrackLiveLinkSource::GetCustomSettingsClass() const {
		return UDTrackLiveLinkSourceSettings::StaticClass();
	}	
#endif



void FDTrackLiveLinkSource::OnSettingsChanged(ULiveLinkSourceSettings* InSettings, const FPropertyChangedEvent& InPropertyChangedEvent) {

	if (InSettings != nullptr && InSettings == m_source_settings.Get()) {

		//If server settings have changed, restart the connection and reset our cached data. 
		if (m_current_server_settings != m_source_settings->m_server_settings) {

			m_current_server_settings = m_source_settings->m_server_settings;

			m_sdk_handler->stop_listening();

			reset_datamaps();

			m_sdk_handler->start_listening(m_source_settings->m_server_settings);
		}
	}
}

void FDTrackLiveLinkSource::handle_body_data_anythread(double n_worldtime, double n_timestamp, int32 n_itemId, float n_quality, const FVector& n_location, const FRotator& n_rotation) {

	//When quality is below 0, the body was not visible by tracking system
	if (n_quality <= 0.0f) {
		return;
	}

	FLiveLinkSubjectKey key;

	{
		FScopeLock Lock(&m_data_access_criticalsection);

		if (const FLiveLinkSubjectKey* found_ptr = m_body_subjects.Find(n_itemId)) {

			key = *found_ptr;
		}
		else {

			FString subject_name;
			std::string body_name = "";
			if(m_sdk_handler->get_body_name(n_itemId, body_name))
			{
				subject_name = UTF8_TO_TCHAR(body_name.c_str());
			}
			else
			{
				subject_name = FString::Printf(TEXT("DTrack-Body-%02d"), n_itemId);
			}

			//Body data always consists of Location and Rotation. No need to make verification to resend static data
			key = FLiveLinkSubjectKey(m_source_guid, *subject_name);
			m_body_subjects.Add(n_itemId, key);

			FLiveLinkStaticDataStruct static_data(FLiveLinkTransformStaticData::StaticStruct());
			m_client->PushSubjectStaticData_AnyThread(key, ULiveLinkTransformRole::StaticClass(), MoveTemp(static_data));
		}
	}

	//Fill transform data
	FLiveLinkFrameDataStruct frame_data(FLiveLinkTransformFrameData::StaticStruct());
	FLiveLinkTransformFrameData* transform_data = frame_data.Cast<FLiveLinkTransformFrameData>();
	transform_data->Transform.SetLocation(n_location);
	transform_data->Transform.SetRotation(n_rotation.Quaternion());
	transform_data->Transform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

	transform_data->WorldTime = FLiveLinkWorldTime(n_worldtime, 0.0);
	const FFrameRate rate = FApp::GetTimecodeFrameRate();
	transform_data->MetaData.SceneTime = FQualifiedFrameTime(FTimecode::FromTimespan(FTimespan::FromSeconds(n_timestamp), rate, FTimecode::IsDropFormatTimecodeSupported(rate), false), rate);
	m_client->PushSubjectFrameData_AnyThread(key, MoveTemp(frame_data));
}

void FDTrackLiveLinkSource::handle_flystick_data_anythread(double n_worldtime, double n_timestamp, int32 n_itemId, float n_quality, const FVector& n_location, const FRotator& n_rotation, TArray<bool>& n_temp_buttons, TArray<float>& n_temp_joysticks) {

	//Handle flystick with transform and inputs
	handle_flystick_input_anythread(n_worldtime, n_timestamp, n_itemId, n_temp_buttons, n_temp_joysticks);

	//Also create a subject only for transform data if quality is good
	if (n_quality > 0.0f)
	{
		handle_flystick_body_anythread(n_worldtime, n_timestamp, n_itemId, n_quality, n_location, n_rotation);
	}
}

void FDTrackLiveLinkSource::handle_flystick_body_anythread(double n_worldtime, double n_timestamp, int32 n_itemId, float n_quality, const FVector& n_location, const FRotator& n_rotation) {

	FLiveLinkSubjectKey key;
	{
		FScopeLock Lock(&m_data_access_criticalsection);

		if (const FLiveLinkSubjectKey* found_ptr = m_flystick_body_subjects.Find(n_itemId)) {

			key = *found_ptr;
		}
		else {

			//Flystick transform only data always consists of Location and Rotation. No need to make verification to resend static data
			const FString subject_name = FString::Printf(TEXT("DTrack-FlystickBody-%02d"), n_itemId);
			key = FLiveLinkSubjectKey(m_source_guid, *subject_name);
			m_flystick_body_subjects.Add(n_itemId, key);

			FLiveLinkStaticDataStruct static_data(FLiveLinkTransformStaticData::StaticStruct());
			m_client->PushSubjectStaticData_AnyThread(key, ULiveLinkTransformRole::StaticClass(), MoveTemp(static_data));
		}
	}

	//Fill transform data
	FLiveLinkFrameDataStruct frame_data(FLiveLinkTransformFrameData::StaticStruct());
	FLiveLinkTransformFrameData* transform_data = frame_data.Cast<FLiveLinkTransformFrameData>();
	transform_data->Transform.SetLocation(n_location);
	transform_data->Transform.SetRotation(n_rotation.Quaternion());
	transform_data->Transform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

	transform_data->WorldTime = FLiveLinkWorldTime(n_worldtime, 0.0);
	const FFrameRate rate = FApp::GetTimecodeFrameRate();
	transform_data->MetaData.SceneTime = FQualifiedFrameTime(FTimecode::FromTimespan(FTimespan::FromSeconds(n_timestamp), rate, FTimecode::IsDropFormatTimecodeSupported(rate), false), rate);
	m_client->PushSubjectFrameData_AnyThread(key, MoveTemp(frame_data));
}


void FDTrackLiveLinkSource::handle_flystick_input_anythread(double n_worldtime, double n_timestamp, int32 n_itemId, TArray<bool>& n_temp_buttons, TArray<float>& n_temp_joysticks)
{
	FLiveLinkSubjectKey key;
	{
		FScopeLock Lock(&m_data_access_criticalsection);

		bool bNeedToUpdateStaticData = false;
		if (const FLiveLinkSubjectKey* found_ptr = m_flystick_input_subjects.Find(n_itemId)) {

			//Subject exists. Make sure its data match what was previously received
			key = *found_ptr;

			if (const FDTrackFlystickInputStaticData* found_data = m_flystick_input_static_data_map.Find(key.SubjectName.Name)) {

				if (found_data->m_button_count != n_temp_buttons.Num() || found_data->m_joystick_count != n_temp_joysticks.Num()) {

					bNeedToUpdateStaticData = true;
				}
			}
		}
		else {
			const FString subject_name = FString::Printf(TEXT("DTrack-FlystickInput-%02d"), n_itemId);
			key = FLiveLinkSubjectKey(m_source_guid, *subject_name);
			m_flystick_input_subjects.Add(n_itemId, key);

			bNeedToUpdateStaticData = true;
		}

		if (bNeedToUpdateStaticData) {

			FLiveLinkStaticDataStruct static_data(FDTrackFlystickInputStaticData::StaticStruct());
			FDTrackFlystickInputStaticData* flystick_static_data = static_data.Cast<FDTrackFlystickInputStaticData>();
			flystick_static_data->m_flystick_id = n_itemId;
			flystick_static_data->m_button_count = n_temp_buttons.Num();
			flystick_static_data->m_joystick_count = n_temp_joysticks.Num();

			m_client->PushSubjectStaticData_AnyThread(key, UDTrackFlystickInputRole::StaticClass(), MoveTemp(static_data));
			
			//Update our map with the latest static data
			m_flystick_input_static_data_map.FindOrAdd(key.SubjectName.Name) = *flystick_static_data;
		}
	}

	//Flystick input is made of a transform, buttons state and joysticks state.
	FLiveLinkFrameDataStruct frame_data(FDTrackFlystickInputFrameData::StaticStruct());
	FDTrackFlystickInputFrameData* flystick_data = frame_data.Cast<FDTrackFlystickInputFrameData>();

	flystick_data->m_button_state.Reserve(n_temp_buttons.Num());
	flystick_data->m_joystick_state.Reserve(n_temp_joysticks.Num());
	for (int32 i = 0; i < n_temp_buttons.Num(); ++i) {

		flystick_data->m_button_state.Add(n_temp_buttons[i]);
	}

	for (int32 i = 0; i < n_temp_joysticks.Num(); ++i) {

		flystick_data->m_joystick_state.Add(n_temp_joysticks[i]);
	}

	flystick_data->WorldTime = FLiveLinkWorldTime(n_worldtime, 0.0);
	const FFrameRate rate = FApp::GetTimecodeFrameRate();
	flystick_data->MetaData.SceneTime = FQualifiedFrameTime(FTimecode::FromTimespan(FTimespan::FromSeconds(n_timestamp), rate, FTimecode::IsDropFormatTimecodeSupported(rate), false), rate);
	m_client->PushSubjectFrameData_AnyThread(key, MoveTemp(frame_data));
}



void FDTrackLiveLinkSource::reset_datamaps() {

	FScopeLock Lock(&m_data_access_criticalsection);

	//Clear bodies that were added
	for (const TPair<int32, FLiveLinkSubjectKey>& entry : m_body_subjects) {

		const FLiveLinkSubjectKey& key = entry.Value;
		m_client->RemoveSubject_AnyThread(key);
	}
	m_body_subjects.Empty();

	//Clear flystick bodies that were added
	for (const TPair<int32, FLiveLinkSubjectKey>& entry : m_flystick_body_subjects) {

		const FLiveLinkSubjectKey& key = entry.Value;
		m_client->RemoveSubject_AnyThread(key);
	}
	m_flystick_body_subjects.Empty();

	//Clear flystick inputs that were added
	for (const TPair<int32, FLiveLinkSubjectKey>& entry : m_flystick_input_subjects) {

		const FLiveLinkSubjectKey& key = entry.Value;
		m_client->RemoveSubject_AnyThread(key);
	}
	m_flystick_input_subjects.Empty();
	m_flystick_input_static_data_map.Empty();

}

bool FDTrackLiveLinkSource::RequestSourceShutdown() {

	m_sdk_handler->Stop();
	reset_datamaps();
	return true;
}

FText FDTrackLiveLinkSource::GetSourceType() const {

	return FText().FromString(TEXT("DTrack"));
}

FText FDTrackLiveLinkSource::GetSourceMachineName() const {

	if (m_source_settings.IsValid()) {

		return FText::FromString(FString::Printf(TEXT("%s:%d"), *m_source_settings->m_server_settings.m_dtrack_server_ip, m_source_settings->m_server_settings.m_dtrack_server_port));
	}
	else {
		return NSLOCTEXT("LiveLinkDTrackLiveLinkSource", "MachineNameStatus", "Invalid");
	}
}

#undef LOCTEXT_NAMESPACE 
