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

#include "DtrackLiveLinkRetargetAsset.h"

#include "BonePose.h"
#include "Roles/LiveLinkAnimationTypes.h"
#include "AnimationRuntime.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"

#include "DTrackPlugin.h"	// LogDTrackPlugin




UDTrackLiveLinkRetargetAsset::UDTrackLiveLinkRetargetAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bRotationOnly = false;

	BoneNameMap =
	{
		{TEXT("pelvis"), 					TEXT("pelvis")},
		{TEXT("spine_01"), 					TEXT("spine_01")},
		{TEXT("spine_02"), 					TEXT("spine_02")},
		{TEXT("spine_03"), 					TEXT("spine_03")},
		{TEXT("clavicle_l"), 				TEXT("clavicle_l")},
		{TEXT("upperarm_l"), 				TEXT("upperarm_l")},
		{TEXT("lowerarm_l"), 				TEXT("lowerarm_l")},
		{TEXT("hand_l"), 					TEXT("hand_l")},
		{TEXT("index_01_l"),				TEXT("index_01_l")},
		{TEXT("index_02_l"),				TEXT("index_02_l")},
		{TEXT("index_03_l"),				TEXT("index_03_l")},
		{TEXT("middle_01_l"), 				TEXT("middle_01_l")},
		{TEXT("middle_02_l"), 				TEXT("middle_02_l")},
		{TEXT("middle_03_l"), 				TEXT("middle_03_l")},
		{TEXT("pinky_01_l"),				TEXT("pinky_01_l")},
		{TEXT("pinky_02_l"),				TEXT("pinky_02_l")},
		{TEXT("pinky_03_l"),				TEXT("pinky_03_l")},
		{TEXT("ring_01_l"), 				TEXT("ring_01_l")},
		{TEXT("ring_02_l"), 				TEXT("ring_02_l")},
		{TEXT("ring_03_l"), 				TEXT("ring_03_l")},
		{TEXT("thumb_01_l"),				TEXT("thumb_01_l")},
		{TEXT("thumb_02_l"),				TEXT("thumb_02_l")},
		{TEXT("thumb_03_l"),				TEXT("thumb_03_l")},
		{TEXT("clavicle_r"),				TEXT("clavicle_r")},
		{TEXT("upperarm_r"), 				TEXT("upperarm_r")},
		{TEXT("lowerarm_r"),				TEXT("lowerarm_r")},
		{TEXT("hand_r"), 					TEXT("hand_r")},
		{TEXT("index_01_r"),				TEXT("index_01_r")},
		{TEXT("index_02_r"),				TEXT("index_02_r")},
		{TEXT("index_03_r"),				TEXT("index_03_r")},
		{TEXT("middle_01_r"), 				TEXT("middle_01_r")},
		{TEXT("middle_02_r"), 				TEXT("middle_02_r")},
		{TEXT("middle_03_r"), 				TEXT("middle_03_r")},
		{TEXT("pinky_01_r"),				TEXT("pinky_01_r")},
		{TEXT("pinky_02_r"),				TEXT("pinky_02_r")},
		{TEXT("pinky_03_r"),				TEXT("pinky_03_r")},
		{TEXT("ring_01_r"), 				TEXT("ring_01_r")},
		{TEXT("ring_02_r"), 				TEXT("ring_02_r")},
		{TEXT("ring_03_r"), 				TEXT("ring_03_r")},
		{TEXT("thumb_01_r"),				TEXT("thumb_01_r")},
		{TEXT("thumb_02_r"),				TEXT("thumb_02_r")},
		{TEXT("thumb_03_r"), 				TEXT("thumb_03_r")},
		{TEXT("neck_01"), 					TEXT("neck_01")},
		{TEXT("head"), 						TEXT("head")},
		{TEXT("thigh_l"), 					TEXT("thigh_l")},
		{TEXT("calf_l"), 					TEXT("calf_l")},
		{TEXT("foot_l"), 					TEXT("foot_l")},
		{TEXT("ball_l"), 					TEXT("ball_l")},
		{TEXT("thigh_r"), 					TEXT("thigh_r")},
		{TEXT("calf_r"), 					TEXT("calf_r")},
		{TEXT("foot_r"), 					TEXT("foot_r")},
		{TEXT("ball_r"), 					TEXT("ball_r")}
	};

	RightHandBoneNames =
	{
		{TEXT("hand_r")},
		{TEXT("index_01_r")},
		{TEXT("index_02_r")},
		{TEXT("index_03_r")},
		{TEXT("middle_01_r")},
		{TEXT("middle_02_r")},
		{TEXT("middle_03_r")},
		{TEXT("pinky_01_r")},
		{TEXT("pinky_02_r")},
		{TEXT("pinky_03_r")},
		{TEXT("ring_01_r")},
		{TEXT("ring_02_r")},
		{TEXT("ring_03_r")},
		{TEXT("thumb_01_r")},
		{TEXT("thumb_02_r")},
		{TEXT("thumb_03_r")},
	};
	LeftHandBoneNames =
	{
		{TEXT("hand_l")},
		{TEXT("index_01_l")},
		{TEXT("index_02_l")},
		{TEXT("index_03_l")},
		{TEXT("middle_01_l")},
		{TEXT("middle_02_l")},
		{TEXT("middle_03_l")},
		{TEXT("pinky_01_l")},
		{TEXT("pinky_02_l")},
		{TEXT("pinky_03_l")},
		{TEXT("ring_01_l")},
		{TEXT("ring_02_l")},
		{TEXT("ring_03_l")},
		{TEXT("thumb_01_l")},
		{TEXT("thumb_02_l")},
		{TEXT("thumb_03_l")},
	};
}


void UDTrackLiveLinkRetargetAsset::Initialize()
{}


void UDTrackLiveLinkRetargetAsset::BuildPoseFromAnimationData(
	float DeltaTime, const FLiveLinkSkeletonStaticData* InSkeletonData,
	const FLiveLinkAnimationFrameData* InFrameData, FCompactPose& OutPose)
{
	check(InSkeletonData);
	check(InFrameData);


	const TArray<FName>& SourceBoneNames = InSkeletonData->BoneNames;

	TArray<FName, TMemStackAllocator<>> TransformedBoneNames;
	TransformedBoneNames.Reserve(SourceBoneNames.Num());

	for (const FName& SrcBoneName : SourceBoneNames)
		TransformedBoneNames.Add( GetRemappedBoneName(SrcBoneName) );


	for (int32 i = 0; i < TransformedBoneNames.Num(); ++i)
	{
		FTransform BoneTransform = InFrameData->Transforms[i];
		
		int32 MeshIndex = OutPose.GetBoneContainer().GetPoseBoneIndexForBoneName(TransformedBoneNames[i]);

		if (MeshIndex != INDEX_NONE)
		{
			FCompactPoseBoneIndex CPIndex = OutPose.GetBoneContainer().MakeCompactPoseIndex(FMeshPoseBoneIndex(MeshIndex));
			if (CPIndex != INDEX_NONE)
			{
				BoneToCPBIndexMap.Add(TransformedBoneNames[i], CPIndex);
				OutPose[CPIndex] = BoneTransform;
			}
		}
	}


	/**
	* Set the hand position to the ref hand position through the ref index position
	* Reason:  DTracks hand-bone is at the index-bone position. 
	* To convert it to the unreal standard skeleton we add the ref-pose index location to the hand-bone.
	*/

	FVector indexRefLoc;
	int32 handInt;
	FVector handLoc, newLoc;
	FQuat handRot, newRot;


	if (BoneToCPBIndexMap.Contains("hand_r") && BoneToCPBIndexMap.Contains("index_01_r"))
	{
		handInt   =  OutPose.GetBoneContainer().MakeMeshPoseIndex( *BoneToCPBIndexMap.Find("hand_r") ).GetInt();

		handRot      =  InFrameData->Transforms[ handInt ].GetRotation();

		handLoc      = InFrameData->Transforms[ handInt ].GetLocation();
		indexRefLoc  = OutPose.GetRefPose( *BoneToCPBIndexMap.Find("index_01_r") ).GetLocation();

		newLoc    =  handLoc + (-1) * ( handRot * indexRefLoc );
		newRot    =  handRot;

		OutPose[ *BoneToCPBIndexMap.Find("hand_r") ].SetLocation( newLoc );
		OutPose[ *BoneToCPBIndexMap.Find("hand_r") ].SetRotation( handRot );
	}
	else 
	if (BoneToCPBIndexMap.Contains("hand_l") && BoneToCPBIndexMap.Contains("index_01_l"))
	{
		handInt   =  OutPose.GetBoneContainer().MakeMeshPoseIndex( *BoneToCPBIndexMap.Find("hand_l") ).GetInt();

		handRot      =  InFrameData->Transforms[ handInt ].GetRotation();

		handLoc      = InFrameData->Transforms[ handInt ].GetLocation();
		indexRefLoc  = OutPose.GetRefPose( *BoneToCPBIndexMap.Find("index_01_l") ).GetLocation();

		newLoc    =  handLoc + (-1) * ( handRot * indexRefLoc ); 
		newRot    =  handRot;

		OutPose[ *BoneToCPBIndexMap.Find("hand_l") ].SetLocation( newLoc );
		OutPose[ *BoneToCPBIndexMap.Find("hand_l") ].SetRotation( handRot );
	}

	
	for (auto It = BoneToCPBIndexMap.CreateConstIterator(); It; ++It)
	{
		if ( It.Key() == ("hand_r") || It.Key() == ("hand_l") )
			continue;
		OutPose[ It.Value() ].SetLocation ( OutPose.GetRefPose( It.Value() ).GetLocation() );
	}
}



FName UDTrackLiveLinkRetargetAsset::GetRemappedBoneName(FName BoneName) const
{
	// Return the mapped name if we have one, otherwise just pass back the base name
	const FName* OutName = BoneNameMap.Find(BoneName);
	if (OutName != nullptr)
	{
		return *OutName;
	}
	return BoneName;
}

