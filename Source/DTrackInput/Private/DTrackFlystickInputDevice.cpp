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

#include "DTrackFlystickInputDevice.h"

#include "DTrackLiveLinkRole.h"
#include "DTrackInputModule.h"
#include "Features/IModularFeatures.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"
#include "ILiveLinkClient.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"

#include "InputCoreTypes.h"


FDTrackFlystickInputDevice::FDTrackFlystickInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) 
	: m_livelink_client(nullptr)
	, m_message_handler(InMessageHandler)
	, m_initial_button_repeat_delay(0.2f)
	, m_button_repeat_delay(0.1f) {

	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	if (ModularFeatures.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName)) {
		register_with_livelink();
	}
	else {
		ModularFeatures.OnModularFeatureRegistered().AddRaw(this, &FDTrackFlystickInputDevice::on_modular_feature_registerd);
	}

	m_button_mapping.Add( EKeys::Gamepad_LeftTrigger.GetFName() );
	m_button_mapping.Add( EKeys::Gamepad_FaceButton_Bottom.GetFName() );
	m_button_mapping.Add( EKeys::Gamepad_FaceButton_Right.GetFName() );
	m_button_mapping.Add( EKeys::Gamepad_FaceButton_Left.GetFName() );
	m_button_mapping.Add( EKeys::Gamepad_FaceButton_Top.GetFName() );
	m_button_mapping.Add( EKeys::Gamepad_DPad_Up.GetFName() );
	m_button_mapping.Add( EKeys::Gamepad_DPad_Down.GetFName() );
	m_button_mapping.Add( EKeys::Gamepad_DPad_Right.GetFName() );
	m_button_mapping.Add( EKeys::Gamepad_DPad_Left.GetFName() );

	m_joystick_mapping.Add( EKeys::Gamepad_LeftX.GetFName() );
	m_joystick_mapping.Add( EKeys::Gamepad_LeftY.GetFName() );
}

FDTrackFlystickInputDevice::~FDTrackFlystickInputDevice() {
	
	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	ModularFeatures.OnModularFeatureRegistered().RemoveAll(this);

	if (m_livelink_client)
	{
		m_livelink_client->OnLiveLinkSubjectAdded().RemoveAll(this);
		m_livelink_client->OnLiveLinkSubjectRemoved().RemoveAll(this);
	}
}

void FDTrackFlystickInputDevice::Tick(float DeltaTime) {

}

void FDTrackFlystickInputDevice::SendControllerEvents() {

	const double current_time = FPlatformTime::Seconds();

	for (TPair<FName, FFlystickState>& pair : m_flystick_state)	
	{
		const FName& flystick_name = pair.Key;
		FFlystickState& flystick_state = pair.Value;

		FLiveLinkSubjectFrameData frame_data;
		if (m_livelink_client->EvaluateFrame_AnyThread(flystick_name, UDTrackFlystickInputRole::StaticClass(), frame_data)) {

			FDTrackFlystickInputStaticData* flystick_static_data = frame_data.StaticData.Cast<FDTrackFlystickInputStaticData>();
			FDTrackFlystickInputFrameData* flystick_frame_data = frame_data.FrameData.Cast<FDTrackFlystickInputFrameData>();

			if (!flystick_state.m_is_initialized) {
				flystick_state.m_buttons_state.Init(false, flystick_static_data->m_button_count);
				flystick_state.m_joysticks_state.Init(0.0f, flystick_static_data->m_joystick_count);
				flystick_state.m_buttons_repeat_time.Init(0.0, flystick_static_data->m_button_count);
				flystick_state.m_is_initialized = true;
			}

			//Process buttons
			for (int32 i = 0; i < flystick_frame_data->m_button_state.Num(); ++i) 
			{

				const bool current_button_state = flystick_frame_data->m_button_state[i];
				if ( current_button_state != flystick_state.m_buttons_state[i] ) 
				{
					//UE_LOG( LogDTrackInput, Warning, TEXT( "flystick name %s id %d button %d state %d"), 
					//	*flystick_name.ToString(), flystick_static_data->m_flystick_id, i, int( ! flystick_state.m_buttons_state[i] ) );

					if (current_button_state)	{
						m_message_handler->OnControllerButtonPressed(m_button_mapping[i], flystick_static_data->m_flystick_id, false);
					}
					else {
						m_message_handler->OnControllerButtonReleased(m_button_mapping[i], flystick_static_data->m_flystick_id, false);
					}

					if (current_button_state) {
						flystick_state.m_buttons_repeat_time[i] = current_time + m_initial_button_repeat_delay;
					}

					flystick_state.m_buttons_state[i] = current_button_state;
				}
				else if (current_button_state && flystick_state.m_buttons_repeat_time[i] < current_time) {
					m_message_handler->OnControllerButtonPressed(m_button_mapping[i], flystick_static_data->m_flystick_id, true);

					flystick_state.m_buttons_repeat_time[i] = current_time + m_button_repeat_delay;
				}
			}


			bool flystick_changed = false;

			for (int32 i = 0; i < flystick_frame_data->m_joystick_state.Num(); ++i) 
			{			
				const float current_joystick_state = flystick_frame_data->m_joystick_state[i];
				if (current_joystick_state != flystick_state.m_joysticks_state[i])
				{
					flystick_changed = true;
			
					m_message_handler->OnControllerAnalog( m_joystick_mapping[i], flystick_static_data->m_flystick_id, current_joystick_state);
					flystick_state.m_joysticks_state[i] = current_joystick_state;
				}
			}

			//if ( flystick_changed  &&  flystick_frame_data->m_joystick_state.Num() == 2)  {
			//	UE_LOG( LogDTrackInput, Warning, TEXT( "flystick name %s id %d joystick x %.2f y %.2f"), 
			//		*flystick_name.ToString(),  flystick_static_data->m_flystick_id,
			//		flystick_state.m_joysticks_state[0], flystick_state.m_joysticks_state[1] );
			//}
		}
	}
	
}

void FDTrackFlystickInputDevice::SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) {
	m_message_handler = InMessageHandler;
}

bool FDTrackFlystickInputDevice::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) {
	return false;
}

void FDTrackFlystickInputDevice::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) {

}

void FDTrackFlystickInputDevice::SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values) {

}

void FDTrackFlystickInputDevice::register_with_livelink() {

	m_livelink_client = &IModularFeatures::Get().GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
	m_livelink_client->OnLiveLinkSubjectAdded().AddRaw(this, &FDTrackFlystickInputDevice::on_livelink_subject_added_handler);
	m_livelink_client->OnLiveLinkSubjectRemoved().AddRaw(this, &FDTrackFlystickInputDevice::on_livelink_subject_removed_handler);
}

void FDTrackFlystickInputDevice::on_modular_feature_registerd(const FName& Type, IModularFeature* ModularFeature) {

	if (Type == ILiveLinkClient::ModularFeatureName) {
		register_with_livelink();
	}
}

void FDTrackFlystickInputDevice::on_livelink_subject_added_handler(FLiveLinkSubjectKey n_subject_key) {

	FLiveLinkSubjectKey* subject_ptr = m_flysticks.FindByPredicate([&](const FLiveLinkSubjectKey& other) { return other == n_subject_key; });
	if (subject_ptr)
	{
		UE_LOG(LogDTrackInput, Warning, TEXT("Subject '%s' was added but already present in the input list"), *n_subject_key.SubjectName.ToString());
		return;
	}

	TSubclassOf<ULiveLinkRole> subject_role = m_livelink_client->GetSubjectRole(n_subject_key.SubjectName);
	if (subject_role->IsChildOf(UDTrackFlystickInputRole::StaticClass()))
	{
		m_flysticks.Add(n_subject_key);
		m_flystick_state.Add(n_subject_key.SubjectName);
	}
}

void FDTrackFlystickInputDevice::on_livelink_subject_removed_handler(FLiveLinkSubjectKey n_subject_key) {

	FLiveLinkSubjectKey* subject_ptr = m_flysticks.FindByPredicate([&](const FLiveLinkSubjectKey& other) { return other == n_subject_key; });
	if (subject_ptr) {

		m_flysticks.RemoveSingleSwap(*subject_ptr);
	}

	m_flystick_state.Remove(n_subject_key.SubjectName);
}
