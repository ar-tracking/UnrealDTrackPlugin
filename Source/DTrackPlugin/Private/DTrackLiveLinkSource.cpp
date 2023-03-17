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

#include "DTrackJointsData.h"
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

#include "DTrackInterface.h"



#define LOCTEXT_NAMESPACE "DTrackLiveLinkSource"


namespace DTrackLiveLinkSourceUtils {
	
	int32 get_finger_first_joint_id(EDTrackFingerType n_finger_type) {
		switch (n_finger_type) {
		case EDTrackFingerType::FT_Thumb: return (int32)EDTrackJointId::thumb_01_l;
		case EDTrackFingerType::FT_Index: return (int32)EDTrackJointId::index_01_l;
		case EDTrackFingerType::FT_Middle: return (int32)EDTrackJointId::middle_01_l;
		case EDTrackFingerType::FT_Ring: return (int32)EDTrackJointId::ring_01_l;
		case EDTrackFingerType::FT_Pinky: return (int32)EDTrackJointId::pinky_01_l;
		default: return 0;
		}
	}
}

FDTrackLiveLinkSource::FDTrackLiveLinkSource()
	: m_client(nullptr) {
}

void FDTrackLiveLinkSource::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) {

	m_client = InClient;
	m_source_guid = InSourceGuid;
	if (!m_sdk_handler.IsValid()) {
		m_sdk_handler = MakeShared<FDTrackSDKHandler>(this);
	}
}

void FDTrackLiveLinkSource::InitializeSettings(ULiveLinkSourceSettings* InSettings) {

	m_source_settings = CastChecked<UDTrackLiveLinkSourceSettings>(InSettings);
	reset_datamaps();
	m_sdk_handler->start_listening(m_source_settings->m_server_settings);

	switch (m_source_settings->m_server_settings.m_coordinate_system) {
	case EDTrackCoordinateSystemType::CST_Normal:
		UE_LOG( LogDTrackPlugin, Warning, TEXT("EDTrackCoordinateSystemType::CST_Normal") );
		break;
	case EDTrackCoordinateSystemType::CST_Unreal_Adapted:
		UE_LOG( LogDTrackPlugin, Warning, TEXT("EDTrackCoordinateSystemType::CST_Unreal_Adapted") );
		break;
	case EDTrackCoordinateSystemType::CST_Powerwall:
		UE_LOG( LogDTrackPlugin, Warning, TEXT("EDTrackCoordinateSystemType::CST_Powerwall") );
	default:
		UE_LOG( LogDTrackPlugin, Warning, TEXT("No suitable EDTrackCoordinateSystemType") );
	}
}

bool FDTrackLiveLinkSource::IsSourceStillValid() const {

	return m_sdk_handler->is_active();
}

FText FDTrackLiveLinkSource::GetSourceStatus() const {

	return FText::Format(LOCTEXT("SourceStatus", "{0}"), FText::FromString(m_sdk_handler->get_status()));
}



#if ENGINE_MAJOR_VERSION == 5 || ( ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION>=24 )
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

			//Body data always consists of Location and Rotation. No need to make verification to resend static data
			const FString subject_name = FString::Printf(TEXT("DTrack-Body-%02d"), n_itemId);
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


void FDTrackLiveLinkSource::handle_hand_data_anythread(
	double n_worldtime, double n_timestamp, int32 n_itemId, 
	float n_quality, bool n_is_right_hand, 
	const FTransform& n_transform, 
	TArray<EDTrackFingerType>& n_temp_fingers_type, 
	TArray<FDTrackFinger>& n_temp_fingers ) 
{
	//When quality is below 0, the body was not visible by tracking system
	if (n_quality <= 0.0f) {
		return;
	}

	check(n_temp_fingers_type.Num() == n_temp_fingers.Num());

	FLiveLinkSubjectKey key;
	const int32 bone_count = 1 + 4 * n_temp_fingers_type.Num(); //4 bones per finger + 1 for the hand
	const int32 property_count = n_temp_fingers_type.Num() * 1; //tip radius per finger

	{	FScopeLock Lock(&m_data_access_criticalsection);

		bool bNeedToUpdateStaticData = false;
		if ( const FLiveLinkSubjectKey* found_ptr = m_hand_subjects.Find( n_itemId ) ) {

			//Subject exists. Make sure its data match what was previously received
			key = *found_ptr;

			if (const FDTrackHandStaticData* found_data = m_hand_static_data_map.Find(key.SubjectName.Name))
			{
				//Verify if we have the same number of fingers (1 bone for the hand + n bones for the fingers) and that it's the same hand side
				if (found_data->m_fingers_type != n_temp_fingers_type || found_data->m_is_right_hand != n_is_right_hand)
					bNeedToUpdateStaticData = true;
			}
		}
		else
		{
			//Subject doesn't exist. Add it to our map and mark static data update required
			const FString subject_name = FString::Printf(TEXT("DTrack-%sHand-%02d"), n_is_right_hand ? TEXT("Right") : TEXT("Left"), n_itemId);
			key = FLiveLinkSubjectKey(m_source_guid, *subject_name);
			m_hand_subjects.Add(n_itemId, key);

			bNeedToUpdateStaticData = true;
		}

		if (bNeedToUpdateStaticData)
		{
			FLiveLinkStaticDataStruct static_data(FDTrackHandStaticData::StaticStruct());
			FDTrackHandStaticData* hand_static_data = static_data.Cast<FDTrackHandStaticData>();

			hand_static_data->m_is_right_hand = n_is_right_hand;
			hand_static_data->m_fingers_type = MoveTemp(n_temp_fingers_type);

			hand_static_data->BoneNames.Reserve(bone_count);
			hand_static_data->BoneParents.Reserve(bone_count); 

			hand_static_data->BoneNames.Add( n_is_right_hand ? 
				DTrackHumanModelUtils::human_model_bones[(int32)EDTrackJointId::hand_r].name 
				: DTrackHumanModelUtils::human_model_bones[(int32)EDTrackJointId::hand_l].name );

			hand_static_data->BoneParents.Add(INDEX_NONE);

			hand_static_data->PropertyNames.Reserve(property_count);

			for (int32 i = 0; i < hand_static_data->m_fingers_type.Num(); ++i)
			{
				const int32 id_offset = n_is_right_hand ? (int32)EDTrackJointId::thumb_01_r - (int32)EDTrackJointId::thumb_01_l : 0;
				int32 finger_index = DTrackLiveLinkSourceUtils::get_finger_first_joint_id(hand_static_data->m_fingers_type[i]) + id_offset;

				const FDTrackHumanModelBoneInfo& finger_bone = DTrackHumanModelUtils::human_model_bones[finger_index++];
				hand_static_data->BoneNames.Add(finger_bone.name);
				hand_static_data->BoneParents.Add(0);

				for (int32 j = 0; j < 3; ++j) {
					const FDTrackHumanModelBoneInfo& bone = DTrackHumanModelUtils::human_model_bones[finger_index++];
					hand_static_data->BoneNames.Add(bone.name);
					hand_static_data->BoneParents.Add(hand_static_data->BoneParents.Num()-1);
				}

				hand_static_data->PropertyNames.Add(*FString::Printf(TEXT("%s_tipradius"), *finger_bone.name.ToString()));
			}

			m_client->PushSubjectStaticData_AnyThread(key, UDTrackHandRole::StaticClass(), MoveTemp(static_data));

			//Update our map with the latest static data
			m_hand_static_data_map.FindOrAdd(key.SubjectName.Name) = *hand_static_data;
		}
	}

	FLiveLinkFrameDataStruct frame_data(FLiveLinkAnimationFrameData::StaticStruct());
	FLiveLinkAnimationFrameData* hand_data = frame_data.Cast<FLiveLinkAnimationFrameData>();
	hand_data->Transforms.Reserve(bone_count);
	hand_data->PropertyValues.Reserve(property_count);

	//First add the hand bone as the root
	hand_data->Transforms.Add(n_transform);


	//Create hierarchy of transform for the hand

	const FTransform hand_root_inverse_transform = hand_data->Transforms[0].Inverse();

	const FDTrackHandStaticData* hand_static_data = m_hand_static_data_map.Find(key.SubjectName.Name);
	for (int32 i = 0; i < hand_static_data->m_fingers_type.Num(); ++i) 
	{
		const FDTrackFinger& finger = n_temp_fingers[i];
		int32 id_offset = n_is_right_hand ? (int32)EDTrackJointId::thumb_01_r - (int32)EDTrackJointId::thumb_01_l : 0;

		//Inner phalanx related to hand root
		hand_data->Transforms.Add(finger.m_inner_phalanx_transform * hand_root_inverse_transform);

		//Middle phalanx related to inner phalanx
		hand_data->Transforms.Add(finger.m_middle_phalanx_transform * finger.m_inner_phalanx_transform.Inverse());

		//Outer phalanx related to middle phalanx
		hand_data->Transforms.Add(finger.m_outer_phalanx_transform * finger.m_middle_phalanx_transform.Inverse());

		//Tip phalanx related to outer phalanx
		hand_data->Transforms.Add(finger.m_tip_transform * finger.m_outer_phalanx_transform.Inverse());

		//Add finger tip radius to property
		hand_data->PropertyValues.Add(finger.m_tip_radius);
	}

	hand_data->WorldTime = FLiveLinkWorldTime(n_worldtime, 0.0);
	const FFrameRate rate = FApp::GetTimecodeFrameRate();
	hand_data->MetaData.SceneTime = FQualifiedFrameTime(FTimecode::FromTimespan(FTimespan::FromSeconds(n_timestamp), rate, FTimecode::IsDropFormatTimecodeSupported(rate), false), rate);

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

	//Clear subjects that were added
	for (const TPair<int32, FLiveLinkSubjectKey>& entry : m_hand_subjects) {

		const FLiveLinkSubjectKey& key = entry.Value;
		m_client->RemoveSubject_AnyThread(key);
	}
	m_hand_subjects.Empty();
	m_hand_static_data_map.Empty();

	//Clear subjects that were added
	for (const TPair<int32, FLiveLinkSubjectKey>& entry : m_human_subjects) {

		const FLiveLinkSubjectKey& key = entry.Value;
		m_client->RemoveSubject_AnyThread(key);
	}
	m_human_subjects.Empty();
	m_human_static_data_map.Empty();
}

bool FDTrackLiveLinkSource::RequestSourceShutdown() {
	if(m_sdk_handler.IsValid())
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
