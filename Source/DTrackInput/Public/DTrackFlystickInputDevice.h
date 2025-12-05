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

#include "IInputDevice.h"
#include "LiveLinkTypes.h"


// Only one FlyStick supported in current version
#define MAX_NUM_FLYSTICK 1


class ILiveLinkClient;
class IModularFeature;


/**
 * Handles Flystick subject data and generate input events
 */
class DTRACKINPUT_API FDTrackFlystickInputDevice : public IInputDevice {
	
public:
	FDTrackFlystickInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler);
	virtual ~FDTrackFlystickInputDevice();

public:
	FLiveLinkSubjectKey get_subject_key() const;

	static void PreInit();

	//~Begin IInputDevice interface
	virtual void Tick(float DeltaTime) override;
	virtual void SendControllerEvents() override;
	virtual void SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override;
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override;
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values) override;
	//~End IInputDevice interface

protected:

	// Used to register ourselves with live link callbacks
	void register_with_livelink();

	// Used to know when live link is available to register to its callbacks
	void on_modular_feature_registerd(const FName& Type, IModularFeature* ModularFeature);

	// Used to update our list of available flysticks
	void on_livelink_subject_added_handler(FLiveLinkSubjectKey n_subject_key);

	// Used to update our list of available flysticks
	void on_livelink_subject_removed_handler(FLiveLinkSubjectKey n_subject_key);

protected:

	struct FFlystickState
	{
		bool m_is_initialized = false;

		// Joystick status from previous frame
		TArray<float> m_joysticks_state;

		// Button status from previous frame (pressed down or not)
		TArray<bool> m_buttons_state;

		// Next time to trigger a repeated button pressed action
		TArray<double> m_buttons_repeat_time;
	};

	TArray<FGamepadKeyNames::Type> m_button_mapping;
	TArray<FGamepadKeyNames::Type> m_joystick_mapping;

	/** Controller states */
	TMap<FName, FFlystickState> m_flystick_state;

	// LiveLink client to fetch data from our subject
	ILiveLinkClient* m_livelink_client;

	//List of registered flystick subjects
	TArray<FLiveLinkSubjectKey> m_flysticks;

	/** handler to send all messages to */
	TSharedRef<FGenericApplicationMessageHandler> m_message_handler;

	/** Delay before sending a repeat message after a button was first pressed */
	float m_initial_button_repeat_delay;

	/** Delay before sending a repeat message after a button has been pressed for a while */
	float m_button_repeat_delay;
};