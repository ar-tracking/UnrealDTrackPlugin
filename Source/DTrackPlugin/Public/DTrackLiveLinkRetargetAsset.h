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
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/ObjectMacros.h"
#include <LiveLinkRetargetAsset.h>
#include "AnimationRuntime.h"
#include "DTrackLiveLinkRetargetAsset.generated.h"


/**
 * DTrack LiveLink remapping asset
 */
UCLASS(Blueprintable, BlueprintType)
class DTRACKPLUGIN_API UDTrackLiveLinkRetargetAsset : public ULiveLinkRetargetAsset
{
	GENERATED_UCLASS_BODY()

	//~ Begin ULiveLinkRetargetAsset interface
	virtual void Initialize() override;
	virtual void BuildPoseFromAnimationData(float DeltaTime, const FLiveLinkSkeletonStaticData* InSkeletonData, const FLiveLinkAnimationFrameData* InFrameData, FCompactPose& OutPose) override;
	//~ End ULiveLinkRetargetAsset interface

	/**
	* Reset every finger bone position to its ref-position, hence: Only take the DTracks rotation.
	* Solves: Several joint position issues where joints won't connect each other
	* Causes: Bones might overlap because they use the real world rotations with other bone positions.
	*/
	UPROPERTY(EditAnywhere, Category = "DTrack Retargeting Asset", meta = (DisplayName = "Use Rotation Only", ToolTip = "Set this to move every bone to the positions of its representation in the reference skeleton. Results in no gaps between bones. Fingers might overlap."))
	bool bRotationOnly;

	/**
	* Used to remap skeletons bone names if they are different. (Left = bones to remap; Right = Correct bone name)
	*/
	UPROPERTY(EditAnywhere, Category = "DTrack Retargeting Asset")
	TMap<FName, FName> BoneNameMap;

protected:
	FName GetRemappedBoneName(FName BoneName) const;

	TArray<FName> RightHandBoneNames;
	TArray<FName> LeftHandBoneNames;

	TMap<FName, FCompactPoseBoneIndex> BoneToCPBIndexMap;
};
