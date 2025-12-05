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
#include "DTrackInputFeatures.h"

#include "Runtime/Launch/Resources/Version.h"

#include "DTrackLiveLinkRole.h"
#include "DTrackInputModule.h"
#include "Features/IModularFeatures.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"
#include "ILiveLinkClient.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"

#include "InputCoreTypes.h"


const FKey FDTrackInputKey::Flystick_Trigger( "Flystick_Trigger" );
const FKey FDTrackInputKey::Flystick_Button1( "Flystick_Button1" );
const FKey FDTrackInputKey::Flystick_Button2( "Flystick_Button2" );
const FKey FDTrackInputKey::Flystick_Button3( "Flystick_Button3" );
const FKey FDTrackInputKey::Flystick_Button4( "Flystick_Button4" );
const FKey FDTrackInputKey::Flystick_Button5( "Flystick_Button5" );
const FKey FDTrackInputKey::Flystick_Button6( "Flystick_Button6" );
const FKey FDTrackInputKey::Flystick_Button7( "Flystick_Button7" );
const FKey FDTrackInputKey::Flystick_Button8( "Flystick_Button8" );
const FKey FDTrackInputKey::Flystick_JoystickX( "Flystick_JoystickX" );
const FKey FDTrackInputKey::Flystick_JoystickY( "Flystick_JoystickY" );



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

	m_button_mapping.Add( FDTrackInputKey::Flystick_Trigger.GetFName() );
	m_button_mapping.Add( FDTrackInputKey::Flystick_Button1.GetFName() );
	m_button_mapping.Add( FDTrackInputKey::Flystick_Button2.GetFName() );
	m_button_mapping.Add( FDTrackInputKey::Flystick_Button3.GetFName() );
	m_button_mapping.Add( FDTrackInputKey::Flystick_Button4.GetFName() );
	m_button_mapping.Add( FDTrackInputKey::Flystick_Button5.GetFName() );
	m_button_mapping.Add( FDTrackInputKey::Flystick_Button6.GetFName() );
	m_button_mapping.Add( FDTrackInputKey::Flystick_Button7.GetFName() );
	m_button_mapping.Add( FDTrackInputKey::Flystick_Button8.GetFName() );
	m_joystick_mapping.Add( FDTrackInputKey::Flystick_JoystickX.GetFName() );
	m_joystick_mapping.Add( FDTrackInputKey::Flystick_JoystickY.GetFName() );
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


void FDTrackFlystickInputDevice::PreInit()
{
	FName Flystick_Category = "ART_Flystick";
	EKeys::AddMenuCategoryDisplayInfo( Flystick_Category, FText::AsCultureInvariant("A.R.T. Flystick"), TEXT("GraphEditor.PadEvent_16x") );

	EKeys::AddKey(FKeyDetails( FDTrackInputKey::Flystick_Trigger,    FText::AsCultureInvariant( FDTrackInputKey::Flystick_Trigger.GetFName().ToString() ),   FKeyDetails::GamepadKey, Flystick_Category) );
	EKeys::AddKey(FKeyDetails( FDTrackInputKey::Flystick_Button1,    FText::AsCultureInvariant( FDTrackInputKey::Flystick_Button1.GetFName().ToString() ),   FKeyDetails::GamepadKey, Flystick_Category) );
	EKeys::AddKey(FKeyDetails( FDTrackInputKey::Flystick_Button2,    FText::AsCultureInvariant( FDTrackInputKey::Flystick_Button2.GetFName().ToString() ),   FKeyDetails::GamepadKey, Flystick_Category) );
	EKeys::AddKey(FKeyDetails( FDTrackInputKey::Flystick_Button3,    FText::AsCultureInvariant( FDTrackInputKey::Flystick_Button3.GetFName().ToString() ),   FKeyDetails::GamepadKey, Flystick_Category) );
	EKeys::AddKey(FKeyDetails( FDTrackInputKey::Flystick_Button4,    FText::AsCultureInvariant( FDTrackInputKey::Flystick_Button4.GetFName().ToString() ),   FKeyDetails::GamepadKey, Flystick_Category) );
	EKeys::AddKey(FKeyDetails( FDTrackInputKey::Flystick_Button5,    FText::AsCultureInvariant( FDTrackInputKey::Flystick_Button5.GetFName().ToString() ),   FKeyDetails::GamepadKey, Flystick_Category) );
	EKeys::AddKey(FKeyDetails( FDTrackInputKey::Flystick_Button6,    FText::AsCultureInvariant( FDTrackInputKey::Flystick_Button6.GetFName().ToString() ),   FKeyDetails::GamepadKey, Flystick_Category) );
	EKeys::AddKey(FKeyDetails( FDTrackInputKey::Flystick_Button7,    FText::AsCultureInvariant( FDTrackInputKey::Flystick_Button7.GetFName().ToString() ),   FKeyDetails::GamepadKey, Flystick_Category) );
	EKeys::AddKey(FKeyDetails( FDTrackInputKey::Flystick_Button8,    FText::AsCultureInvariant( FDTrackInputKey::Flystick_Button8.GetFName().ToString() ),   FKeyDetails::GamepadKey, Flystick_Category) );
	
#if  ENGINE_MAJOR_VERSION < 4  || ( ENGINE_MAJOR_VERSION == 4  &&  ENGINE_MINOR_VERSION < 26 )
	EKeys::AddKey(FKeyDetails( FDTrackInputKey::Flystick_JoystickX,  FText::AsCultureInvariant( FDTrackInputKey::Flystick_JoystickX.GetFName().ToString() ), FKeyDetails::GamepadKey | FKeyDetails::FloatAxis, Flystick_Category) );
	EKeys::AddKey(FKeyDetails( FDTrackInputKey::Flystick_JoystickY,  FText::AsCultureInvariant( FDTrackInputKey::Flystick_JoystickY.GetFName().ToString() ), FKeyDetails::GamepadKey | FKeyDetails::FloatAxis, Flystick_Category) );
#else
	EKeys::AddKey(FKeyDetails( FDTrackInputKey::Flystick_JoystickX,  FText::AsCultureInvariant( FDTrackInputKey::Flystick_JoystickX.GetFName().ToString() ), FKeyDetails::GamepadKey | FKeyDetails::Axis1D, Flystick_Category) );
	EKeys::AddKey(FKeyDetails( FDTrackInputKey::Flystick_JoystickY,  FText::AsCultureInvariant( FDTrackInputKey::Flystick_JoystickY.GetFName().ToString() ), FKeyDetails::GamepadKey | FKeyDetails::Axis1D, Flystick_Category) );
#endif
}


void FDTrackFlystickInputDevice::Tick(float DeltaTime) {

}


void FDTrackFlystickInputDevice::SendControllerEvents()
{
#if  ENGINE_MAJOR_VERSION >= 5
	//FPlatformUserId UserId = FPlatformMisc::GetPlatformUserForUserIndex( LocalUserIndex );		// TODO
	FPlatformUserId UserId = IPlatformInputDeviceMapper::Get().GetPrimaryPlatformUser();
	FInputDeviceId DeviceId = INPUTDEVICEID_NONE;
#endif

	const double current_time = FPlatformTime::Seconds();

	for (TPair<FName, FFlystickState>& pair : m_flystick_state)	
	{
		const FName& flystick_name = pair.Key;
		FFlystickState& flystick_state = pair.Value;

		FLiveLinkSubjectFrameData frame_data;
		if (m_livelink_client->EvaluateFrame_AnyThread(flystick_name, UDTrackFlystickInputRole::StaticClass(), frame_data))
		{
			FDTrackFlystickInputStaticData* flystick_static_data = frame_data.StaticData.Cast<FDTrackFlystickInputStaticData>();
			FDTrackFlystickInputFrameData* flystick_frame_data = frame_data.FrameData.Cast<FDTrackFlystickInputFrameData>();

			if (!flystick_state.m_is_initialized)
			{
				flystick_state.m_buttons_state.Init(false, flystick_static_data->m_button_count);
				flystick_state.m_joysticks_state.Init(0.0f, flystick_static_data->m_joystick_count);
				flystick_state.m_buttons_repeat_time.Init(0.0, flystick_static_data->m_button_count);
				flystick_state.m_is_initialized = true;
			}

#if  ENGINE_MAJOR_VERSION >= 5
			IPlatformInputDeviceMapper::Get().RemapControllerIdToPlatformUserAndDevice( flystick_static_data->m_flystick_id, UserId, DeviceId );
#endif
			//Process buttons
			for (int32 i = 0; i < flystick_frame_data->m_button_state.Num(); ++i) 
			{

				const bool current_button_state = flystick_frame_data->m_button_state[i];
				if ( current_button_state != flystick_state.m_buttons_state[i] ) 
				{
					//UE_LOG( LogDTrackInput, Warning, TEXT( "flystick name %s id %d button %d state %d"), 
					//	*flystick_name.ToString(), flystick_static_data->m_flystick_id, i, int( ! flystick_state.m_buttons_state[i] ) );

					if (current_button_state)
					{
#if  ENGINE_MAJOR_VERSION >= 5
						m_message_handler->OnControllerButtonPressed( m_button_mapping[i], UserId, DeviceId, false );
#else
						m_message_handler->OnControllerButtonPressed( m_button_mapping[i], flystick_static_data->m_flystick_id, false );
#endif
					}

					else 
					{
#if  ENGINE_MAJOR_VERSION >= 5
						m_message_handler->OnControllerButtonReleased( m_button_mapping[i], UserId, DeviceId, false );
#else
						m_message_handler->OnControllerButtonReleased( m_button_mapping[i], flystick_static_data->m_flystick_id, false );
#endif
					}

					if (current_button_state) 
						flystick_state.m_buttons_repeat_time[i] = current_time + m_initial_button_repeat_delay;

					flystick_state.m_buttons_state[i] = current_button_state;
				}

				else if (current_button_state && flystick_state.m_buttons_repeat_time[i] < current_time)
				{
#if  ENGINE_MAJOR_VERSION >= 5
					m_message_handler->OnControllerButtonPressed( m_button_mapping[i], UserId, DeviceId, true );
#else
					m_message_handler->OnControllerButtonPressed( m_button_mapping[i], flystick_static_data->m_flystick_id, true );
#endif
					flystick_state.m_buttons_repeat_time[i] = current_time + m_button_repeat_delay;
				}
			}


			bool flystick_changed = false;

			for (int32 i = 0; i < flystick_frame_data->m_joystick_state.Num(); ++i) 
			{			
				if (i >= 2) break;	// TODO, support 'jt' entry of Fly2+

				const float current_joystick_state = flystick_frame_data->m_joystick_state[i];
				if (current_joystick_state != flystick_state.m_joysticks_state[i])
				{
					flystick_changed = true;

#if  ENGINE_MAJOR_VERSION >= 5
					m_message_handler->OnControllerAnalog( m_joystick_mapping[i], UserId, DeviceId, current_joystick_state );
#else
					m_message_handler->OnControllerAnalog( m_joystick_mapping[i], flystick_static_data->m_flystick_id, current_joystick_state );
#endif
					flystick_state.m_joysticks_state[i] = current_joystick_state;
				}
			}
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

#if  ENGINE_MAJOR_VERSION == 5 
	TSubclassOf<ULiveLinkRole> subject_role = m_livelink_client->GetSubjectRole_AnyThread(n_subject_key.SubjectName);
#else
	TSubclassOf<ULiveLinkRole> subject_role = m_livelink_client->GetSubjectRole(n_subject_key.SubjectName);
#endif
	
	if (!subject_role ||  !subject_role->IsValidLowLevelFast())
		return;
	
	if (subject_role->IsChildOf(UDTrackFlystickInputRole::StaticClass())  &&  m_flysticks.Num() < MAX_NUM_FLYSTICK)
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
