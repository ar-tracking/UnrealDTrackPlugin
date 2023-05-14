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

#include "Roles/LiveLinkAnimationTypes.h"
#include "Roles/LiveLinkTransformTypes.h"

#include "DTrackInterface.h"

#include "DTrackLiveLinkTypes.generated.h"



/**
 * Static data for inputs of flystick tracking data.
 */
USTRUCT(BlueprintType)
struct DTRACKPLUGIN_API FDTrackFlystickInputStaticData : public FLiveLinkBaseStaticData
{
	GENERATED_BODY()

public:

	// id of this flystick
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flystick")
	int32 m_flystick_id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flystick")
	int32 m_button_count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flystick")
	int32 m_joystick_count = 0;
};

/**
 * Frame data for inputs of flystick tracking data.
 */
USTRUCT(BlueprintType)
struct DTRACKPLUGIN_API FDTrackFlystickInputFrameData : public FLiveLinkBaseFrameData
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flystick")
	TArray<bool> m_button_state;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flystick")
	TArray<float> m_joystick_state;
};

/**
 * Facility structure to handle flystick data in blueprint
 */
USTRUCT(BlueprintType)
struct DTRACKPLUGIN_API FDTrackFlystickInputBlueprintData : public FLiveLinkBaseBlueprintData
{
	GENERATED_BODY()

public:
	// Static data that should not change every frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveLink")
	FDTrackFlystickInputStaticData m_static_data;

	// Dynamic data that can change every frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveLink")
	FDTrackFlystickInputFrameData m_frame_data;
};


/**
 * Static data for hand tracking data.
 */
USTRUCT(BlueprintType)
struct DTRACKPLUGIN_API FDTrackHandStaticData : public FLiveLinkSkeletonStaticData
{
	GENERATED_BODY()

public:

	// Number of tracked fingers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hands")
		bool m_is_right_hand = true;

	// Tracked finger types
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fingers")
		TArray<EDTrackFingerType> m_fingers_type;
};


/**
 * Facility structure to handle hand data in blueprint
 */
USTRUCT(BlueprintType)
struct DTRACKPLUGIN_API FDTrackHandBlueprintData : public FLiveLinkBaseBlueprintData
{
	GENERATED_BODY()

public:
	// Static data that should not change every frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveLink")
		FDTrackHandStaticData m_static_data;

	// Dynamic data that can change every frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LiveLink")
		FLiveLinkAnimationFrameData m_frame_data;
};
