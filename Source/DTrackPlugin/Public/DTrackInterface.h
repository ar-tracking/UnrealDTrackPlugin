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

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "DTrackInterface.generated.h"

/** 
 * This represents different room calibration default settings as 
 * set in the DTrack system. Choose the one that corresponds with your setup
 * and transformations will be translated into Unreal's standard coordinate 
 * system
 */
UENUM(BlueprintType, Category=DTrack)
enum class EDTrackCoordinateSystemType : uint8 {

	/// The normal setting with Z-axis pointing upwards
	CST_Normal     UMETA(DisplayName = "Normal"),

	/// Powerwall setting with Y-axis pointing upwards
	CST_Powerwall  UMETA(DisplayName = "Powerwall"),
};


UENUM(BlueprintType)
enum class EDTrackFingerType : uint8 {
	FT_Thumb    UMETA(DisplayName = "Thumb"),
	FT_Index    UMETA(DisplayName = "Index"),
	FT_Middle   UMETA(DisplayName = "Middle"),
	FT_Ring     UMETA(DisplayName = "Ring"),
	FT_Pinky    UMETA(DisplayName = "Pinky")
};


/**
 * This represents one tracked finger data
 */
USTRUCT(BlueprintType)
struct FDTrackFinger {

	GENERATED_BODY()

public:
	//All transforms in world space
	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Transform"))
		FTransform  m_tip_transform;

	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Transform"))
		FTransform  m_outer_phalanx_transform;

	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Transform"))
		FTransform  m_middle_phalanx_transform;

	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Transform"))
		FTransform  m_inner_phalanx_transform;

	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Radius Of Tip"))
		float    m_tip_radius = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Inner Phalanx Length"))
		float    m_inner_phalanx_length = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Middle Phalanx Length"))
		float    m_middle_phalanx_length = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Outer Phalanx Length"))
		float    m_outer_phalanx_length = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Inner Middle Phalanx Angle"))
		float    m_inner_middle_phalanx_angle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Middle Outer Phalanx Angle"))
		float    m_middle_outer_phalanx_angle = 0.0f;
};




#if 0
/**
* This represents one hand tracking data
*/
USTRUCT(BlueprintType, Category = DTrack)
struct FDTrackHand {

	GENERATED_BODY()

	/// true if this is the right hand
	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Right"))
	bool m_right;

	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Location"))
		FVector  m_location;

	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Rotation"))
		FRotator m_rotation;

	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Fingers"))
		TArray<FDTrackFinger> m_fingers;
};
#endif


#if 0
/**
 * This represents one joint of human model tracking.
 */
USTRUCT(BlueprintType)
struct FDTrackJoint {

	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "ID"))
		int32    m_id;

	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Status"))
		float    m_quality;

	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Location"))
		FVector  m_location;

	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Rotation"))
		FRotator m_rotation;

	/// angles in relation to the joint coordinate system
	/**
	 * @todo if this means Euler angles, hand them out as a rotator
	 *    if they are then identical to m_rotation, remove.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Info", meta = (DisplayName = "Angles"))
		TArray<float> m_angles;
};
#endif



