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


#include "IDTrackInputModule.h"
#include "InputCoreTypes.h"

class FDTrackFlystickInputDevice;
class ILiveLinkClient;
class IModularFeature;


DECLARE_LOG_CATEGORY_EXTERN(LogDTrackInput, Log, All);

/**
 * Manages DTrack flysticks subjects to create input device from them
 */
class FDTrackInputModule : public IDTrackInputModule
{
public:
	
	/* Category */
	const FName FlystickKeyCategory = FName("ART_Flystick");
	/* Keys */
	static const FKey FlystickTrigger;
	static const FKey FlystickButton1;
	static const FKey FlystickButton2;
	static const FKey FlystickButton3;
	static const FKey FlystickButton4;
	static const FKey FlystickButton5;
	static const FKey FlystickButton6;
	static const FKey FlystickButton7;
	static const FKey FlystickButton8;
	/* Axis */
	static const FKey FlystickThumbstickX;
	static const FKey FlystickThumbstickY;
	
	//~ Begin IInputDeviceModule implementation
	virtual TSharedPtr< class IInputDevice > CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override;
	//~ End IInputDeviceModule implementation
};