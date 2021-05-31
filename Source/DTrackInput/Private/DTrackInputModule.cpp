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

#include "DTrackInputModule.h"

#include "DTrackFlystickInputDevice.h"

DEFINE_LOG_CATEGORY(LogDTrackInput);

/* Keys */
const FKey FDTrackInputModule::FlystickTrigger = FKey("Flystick_Trigger");
const FKey FDTrackInputModule::FlystickButton1 = FKey("Flystick_Button_1");
const FKey FDTrackInputModule::FlystickButton2 = FKey("Flystick_Button_2");
const FKey FDTrackInputModule::FlystickButton3 = FKey("Flystick_Button_3");
const FKey FDTrackInputModule::FlystickButton4 = FKey("Flystick_Button_4");
const FKey FDTrackInputModule::FlystickButton5 = FKey("Flystick_Button_5");
const FKey FDTrackInputModule::FlystickButton6 = FKey("Flystick_Button_6");
const FKey FDTrackInputModule::FlystickButton7 = FKey("Flystick_Button_7");
const FKey FDTrackInputModule::FlystickButton8 = FKey("Flystick_Button_8");
/* Axis */
const FKey FDTrackInputModule::FlystickThumbstickX = FKey("Flystick_Thumbstick_X");
const FKey FDTrackInputModule::FlystickThumbstickY = FKey("Flystick_Thumbstick_Y");

TSharedPtr< class IInputDevice > FDTrackInputModule::CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) {
	EKeys::AddMenuCategoryDisplayInfo(FlystickKeyCategory, FText::AsCultureInvariant("A.R.T. Flystick"), TEXT("GraphEditor.PadEvent_16x"));
	
	EKeys::AddKey(FKeyDetails(FlystickTrigger, FText::AsCultureInvariant("Flystick Trigger"), FKeyDetails::GamepadKey, FlystickKeyCategory));
	EKeys::AddKey(FKeyDetails(FlystickButton1, FText::AsCultureInvariant("Flystick Button 1"), FKeyDetails::GamepadKey, FlystickKeyCategory));
	EKeys::AddKey(FKeyDetails(FlystickButton2, FText::AsCultureInvariant("Flystick Button 2"), FKeyDetails::GamepadKey, FlystickKeyCategory));
	EKeys::AddKey(FKeyDetails(FlystickButton3, FText::AsCultureInvariant("Flystick Button 3"), FKeyDetails::GamepadKey, FlystickKeyCategory));
	EKeys::AddKey(FKeyDetails(FlystickButton4, FText::AsCultureInvariant("Flystick Button 4"), FKeyDetails::GamepadKey, FlystickKeyCategory));
	EKeys::AddKey(FKeyDetails(FlystickButton5, FText::AsCultureInvariant("Flystick Button 5"), FKeyDetails::GamepadKey, FlystickKeyCategory));
	EKeys::AddKey(FKeyDetails(FlystickButton6, FText::AsCultureInvariant("Flystick Button 6"), FKeyDetails::GamepadKey, FlystickKeyCategory));
	EKeys::AddKey(FKeyDetails(FlystickButton7, FText::AsCultureInvariant("Flystick Button 7"), FKeyDetails::GamepadKey, FlystickKeyCategory));
	EKeys::AddKey(FKeyDetails(FlystickButton8, FText::AsCultureInvariant("Flystick Button 8"), FKeyDetails::GamepadKey, FlystickKeyCategory));
	EKeys::AddKey(FKeyDetails(FlystickThumbstickX,  FText::AsCultureInvariant("Flystick Thumbstick X"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D, FlystickKeyCategory));
	EKeys::AddKey(FKeyDetails(FlystickThumbstickY,  FText::AsCultureInvariant("Flystick Thumbstick Y"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D, FlystickKeyCategory));
	
	return TSharedPtr< class IInputDevice >(new FDTrackFlystickInputDevice(InMessageHandler));
}

IMPLEMENT_MODULE(FDTrackInputModule, DTrackInput);
