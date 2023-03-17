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

enum class EDTrackJointId : uint8 {
	pelvis					// Bone id 00 Pelvis
	, spine_01				// Bone id 01 Lower lumbar spine
	, spine_02				// Bone id 02 Upper lumbar spine
	, spine_03				// Bone id 03 Lower thoracic spine
	, spine_04				// Bone id 04 Upper thoracic spine @note : Unreal base skeleton has 3 spine bones... do we use this one or not?
	, neck_01				// Bone id 05 Neck
	, head					// Bone id 06 Head
	, clavicle_l			// Bone id 07 Left shoulder
	, upperarm_l			// Bone id 08 Left upper arm (humerus)
	, lowerarm_l			// Bone id 09 Left lower arm (radius/ula)
	, hand_l				// Bone id 10 Left hand
	, clavicle_r			// Bone id 11 Right shoulder
	, upperarm_r			// Bone id 12 Right upper arm (humerus)
	, lowerarm_r			// Bone id 13 Right lower arm (radius/ula)
	, hand_r				// Bone id 14 Right hand
	, thigh_l				// Bone id 15 Left upper leg (femur)
	, calf_l				// Bone id 16 Left lower leg (fibula/tibia)
	, foot_l				// Bone id 17 Left foot
	, thigh_r				// Bone id 18 Right upper leg (femur)
	, calf_r				// Bone id 19 Right lower leg (fibula/tibia)
	, foot_r				// Bone id 20 Right foot
	, thumb_01_l			// Bone id 21 Left Thumb : root
	, thumb_02_l			// Bone id 22 Left Thumb : middle
	, thumb_03_l			// Bone id 23 Left Thumb : outer
	, thumb_04_l			// Bone id 24 Left Thumb : tip
	, index_01_l			// Bone id 25 Left Index : root
	, index_02_l			// Bone id 26 Left Index : middle
	, index_03_l			// Bone id 27 Left Index : outer
	, index_04_l			// Bone id 28 Left Index : tip
	, middle_01_l			// Bone id 29 Left Middle : root
	, middle_02_l			// Bone id 30 Left Middle : middle
	, middle_03_l			// Bone id 31 Left Middle : outer
	, middle_04_l			// Bone id 32 Left Middle : tip
	, ring_01_l				// Bone id 33 Left Ring : root
	, ring_02_l				// Bone id 34 Left Ring : middle
	, ring_03_l				// Bone id 35 Left Ring : outer
	, ring_04_l				// Bone id 36 Left Ring : tip
	, pinky_01_l			// Bone id 37 Left Pinky : root
	, pinky_02_l			// Bone id 38 Left Pinky : middle
	, pinky_03_l			// Bone id 39 Left Pinky : outer
	, pinky_04_l			// Bone id 40 Left Pinky : tip
	, thumb_01_r			// Bone id 41 Right Thumb : root
	, thumb_02_r			// Bone id 42 Right Thumb : middle
	, thumb_03_r			// Bone id 43 Right Thumb : outer
	, thumb_04_r			// Bone id 44 Right Thumb : tip
	, index_01_r			// Bone id 45 Right Index : root
	, index_02_r			// Bone id 46 Right Index : middle
	, index_03_r			// Bone id 47 Right Index : outer
	, index_04_r			// Bone id 48 Right Index : tip
	, middle_01_r			// Bone id 49 Left Middle : root
	, middle_02_r			// Bone id 50 Left Middle : middle
	, middle_03_r			// Bone id 51 Left Middle : outer
	, middle_04_r			// Bone id 52 Left Middle : tip
	, ring_01_r				// Bone id 53 Right Ring : root
	, ring_02_r				// Bone id 54 Right Ring : middle
	, ring_03_r				// Bone id 55 Right Ring : outer
	, ring_04_r				// Bone id 56 Right Ring : tip
	, pinky_01_r			// Bone id 57 Left Pinky : root
	, pinky_02_r			// Bone id 58 Right Pinky : middle
	, pinky_03_r			// Bone id 59 Right Pinky : outer
	, pinky_04_r			// Bone id 60 Right Pinky : tip
};

struct FDTrackHumanModelBoneInfo {

	FDTrackHumanModelBoneInfo(FName n_name, int32 n_parent)
		: name(n_name)
		, parent(n_parent) {
	}

	FName name;
	int32 parent;
};

namespace DTrackHumanModelUtils {

	

	/// Human model bone id to to bone names matching closely unreal names
	static const TArray<FDTrackHumanModelBoneInfo> human_model_bones =
	{
		FDTrackHumanModelBoneInfo(TEXT("pelvis"), -1)			
		, FDTrackHumanModelBoneInfo(TEXT("spine_01"), (int32)EDTrackJointId::pelvis)
		, FDTrackHumanModelBoneInfo(TEXT("spine_02"), (int32)EDTrackJointId::spine_01)
		, FDTrackHumanModelBoneInfo(TEXT("spine_03"), (int32)EDTrackJointId::spine_02)
		, FDTrackHumanModelBoneInfo(TEXT("spine_04"), (int32)EDTrackJointId::spine_03)
		, FDTrackHumanModelBoneInfo(TEXT("neck_01"), (int32)EDTrackJointId::spine_04)
		, FDTrackHumanModelBoneInfo(TEXT("head"), (int32)EDTrackJointId::neck_01)
		, FDTrackHumanModelBoneInfo(TEXT("clavicle_l"), (int32)EDTrackJointId::spine_04)
		, FDTrackHumanModelBoneInfo(TEXT("upperarm_l"), (int32)EDTrackJointId::clavicle_l)
		, FDTrackHumanModelBoneInfo(TEXT("lowerarm_l"), (int32)EDTrackJointId::upperarm_l)
		, FDTrackHumanModelBoneInfo(TEXT("hand_l"), (int32)EDTrackJointId::lowerarm_l)
		, FDTrackHumanModelBoneInfo(TEXT("clavicle_r"), (int32)EDTrackJointId::spine_04)
		, FDTrackHumanModelBoneInfo(TEXT("upperarm_r"), (int32)EDTrackJointId::clavicle_r)
		, FDTrackHumanModelBoneInfo(TEXT("lowerarm_r"), (int32)EDTrackJointId::upperarm_r)
		, FDTrackHumanModelBoneInfo(TEXT("hand_r"), (int32)EDTrackJointId::lowerarm_r)
		, FDTrackHumanModelBoneInfo(TEXT("thigh_l"), (int32)EDTrackJointId::pelvis)
		, FDTrackHumanModelBoneInfo(TEXT("calf_l"), (int32)EDTrackJointId::thigh_l)
		, FDTrackHumanModelBoneInfo(TEXT("foot_l"), (int32)EDTrackJointId::calf_l)
		, FDTrackHumanModelBoneInfo(TEXT("thigh_r"), (int32)EDTrackJointId::pelvis)
		, FDTrackHumanModelBoneInfo(TEXT("calf_r"), (int32)EDTrackJointId::thigh_r)
		, FDTrackHumanModelBoneInfo(TEXT("foot_r"), (int32)EDTrackJointId::calf_r)
		, FDTrackHumanModelBoneInfo(TEXT("thumb_01_l"), (int32)EDTrackJointId::hand_l)
		, FDTrackHumanModelBoneInfo(TEXT("thumb_02_l"), (int32)EDTrackJointId::thumb_01_l)
		, FDTrackHumanModelBoneInfo(TEXT("thumb_03_l"), (int32)EDTrackJointId::thumb_02_l)
		, FDTrackHumanModelBoneInfo(TEXT("thumb_04_l"), (int32)EDTrackJointId::thumb_03_l)
		, FDTrackHumanModelBoneInfo(TEXT("index_01_l"), (int32)EDTrackJointId::hand_l)
		, FDTrackHumanModelBoneInfo(TEXT("index_02_l"), (int32)EDTrackJointId::index_01_l)
		, FDTrackHumanModelBoneInfo(TEXT("index_03_l"), (int32)EDTrackJointId::index_02_l)
		, FDTrackHumanModelBoneInfo(TEXT("index_04_l"), (int32)EDTrackJointId::index_03_l)
		, FDTrackHumanModelBoneInfo(TEXT("middle_01_l"), (int32)EDTrackJointId::hand_l)
		, FDTrackHumanModelBoneInfo(TEXT("middle_02_l"), (int32)EDTrackJointId::middle_01_l)
		, FDTrackHumanModelBoneInfo(TEXT("middle_03_l"), (int32)EDTrackJointId::middle_02_l)
		, FDTrackHumanModelBoneInfo(TEXT("middle_04_l"), (int32)EDTrackJointId::middle_03_l)
		, FDTrackHumanModelBoneInfo(TEXT("ring_01_l"), (int32)EDTrackJointId::hand_l)
		, FDTrackHumanModelBoneInfo(TEXT("ring_02_l"), (int32)EDTrackJointId::ring_01_l)
		, FDTrackHumanModelBoneInfo(TEXT("ring_03_l"), (int32)EDTrackJointId::ring_02_l)
		, FDTrackHumanModelBoneInfo(TEXT("ring_04_l"), (int32)EDTrackJointId::ring_03_l)
		, FDTrackHumanModelBoneInfo(TEXT("pinky_01_l"), (int32)EDTrackJointId::hand_l)
		, FDTrackHumanModelBoneInfo(TEXT("pinky_02_l"), (int32)EDTrackJointId::pinky_01_l)
		, FDTrackHumanModelBoneInfo(TEXT("pinky_03_l"), (int32)EDTrackJointId::pinky_02_l)
		, FDTrackHumanModelBoneInfo(TEXT("pinky_04_l"), (int32)EDTrackJointId::pinky_03_l)
		, FDTrackHumanModelBoneInfo(TEXT("thumb_01_r"), (int32)EDTrackJointId::hand_r)
		, FDTrackHumanModelBoneInfo(TEXT("thumb_02_r"), (int32)EDTrackJointId::thumb_01_r)
		, FDTrackHumanModelBoneInfo(TEXT("thumb_03_r"), (int32)EDTrackJointId::thumb_02_r)
		, FDTrackHumanModelBoneInfo(TEXT("thumb_04_r"), (int32)EDTrackJointId::thumb_03_r)
		, FDTrackHumanModelBoneInfo(TEXT("index_01_r"), (int32)EDTrackJointId::hand_r)
		, FDTrackHumanModelBoneInfo(TEXT("index_02_r"), (int32)EDTrackJointId::index_01_r)
		, FDTrackHumanModelBoneInfo(TEXT("index_03_r"), (int32)EDTrackJointId::index_02_r)
		, FDTrackHumanModelBoneInfo(TEXT("index_04_r"), (int32)EDTrackJointId::index_03_r)
		, FDTrackHumanModelBoneInfo(TEXT("middle_01_r"), (int32)EDTrackJointId::hand_r)
		, FDTrackHumanModelBoneInfo(TEXT("middle_02_r"), (int32)EDTrackJointId::middle_01_r)
		, FDTrackHumanModelBoneInfo(TEXT("middle_03_r"), (int32)EDTrackJointId::middle_02_r)
		, FDTrackHumanModelBoneInfo(TEXT("middle_04_r"), (int32)EDTrackJointId::middle_03_r)
		, FDTrackHumanModelBoneInfo(TEXT("ring_01_r"), (int32)EDTrackJointId::hand_r)
		, FDTrackHumanModelBoneInfo(TEXT("ring_02_r"), (int32)EDTrackJointId::ring_01_r)
		, FDTrackHumanModelBoneInfo(TEXT("ring_03_r"), (int32)EDTrackJointId::ring_02_r)
		, FDTrackHumanModelBoneInfo(TEXT("ring_04_r"), (int32)EDTrackJointId::ring_03_r)
		, FDTrackHumanModelBoneInfo(TEXT("pinky_01_r"), (int32)EDTrackJointId::hand_r)
		, FDTrackHumanModelBoneInfo(TEXT("pinky_02_r"), (int32)EDTrackJointId::pinky_01_r)
		, FDTrackHumanModelBoneInfo(TEXT("pinky_03_r"), (int32)EDTrackJointId::pinky_02_r)
		, FDTrackHumanModelBoneInfo(TEXT("pinky_04_r"), (int32)EDTrackJointId::pinky_03_r)
	};

	}
