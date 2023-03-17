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

#include "DTrackLiveLinkRole.h"

#include "DTrackLiveLinkTypes.h"
#include "LiveLinkRoleTrait.h"

#define LOCTEXT_NAMESPACE "DTrackLiveLinkRole"



/**
 * UDTrackFlystickRole
 */

UScriptStruct* UDTrackFlystickInputRole::GetStaticDataStruct() const
{
	return FDTrackFlystickInputStaticData::StaticStruct();
}

UScriptStruct* UDTrackFlystickInputRole::GetFrameDataStruct() const
{
	return FDTrackFlystickInputFrameData::StaticStruct();
}

UScriptStruct* UDTrackFlystickInputRole::GetBlueprintDataStruct() const
{
	return FDTrackFlystickInputBlueprintData::StaticStruct();
}

bool UDTrackFlystickInputRole::InitializeBlueprintData(const FLiveLinkSubjectFrameData& InSourceData, FLiveLinkBlueprintDataStruct& OutBlueprintData) const
{
	bool is_success = false;

	FDTrackFlystickInputBlueprintData* blueprint_data = OutBlueprintData.Cast<FDTrackFlystickInputBlueprintData>();
	const FDTrackFlystickInputStaticData* static_data = InSourceData.StaticData.Cast<FDTrackFlystickInputStaticData>();
	const FDTrackFlystickInputFrameData* frame_data = InSourceData.FrameData.Cast<FDTrackFlystickInputFrameData>();
	if (blueprint_data && static_data && frame_data)
	{
		GetStaticDataStruct()->CopyScriptStruct(&blueprint_data->m_static_data, static_data);
		GetFrameDataStruct()->CopyScriptStruct(&blueprint_data->m_frame_data, frame_data);
		is_success = true;
	}

	return is_success;
}

FText UDTrackFlystickInputRole::GetDisplayName() const {

	return LOCTEXT("DTrackFlystickRole", "DTrack Flystick Input");
}


/**
 * UDTrackHandRole
 */
UScriptStruct* UDTrackHandRole::GetStaticDataStruct() const {

	return FDTrackHandStaticData::StaticStruct();
}

UScriptStruct* UDTrackHandRole::GetFrameDataStruct() const {

	return FLiveLinkAnimationFrameData::StaticStruct();
}

UScriptStruct* UDTrackHandRole::GetBlueprintDataStruct() const {

	return FDTrackHandBlueprintData::StaticStruct();
}

bool UDTrackHandRole::InitializeBlueprintData(const FLiveLinkSubjectFrameData& InSourceData, FLiveLinkBlueprintDataStruct& OutBlueprintData) const {

	bool is_success = false;

	FDTrackHandBlueprintData* blueprint_data = OutBlueprintData.Cast<FDTrackHandBlueprintData>();
	const FDTrackHandStaticData* static_data = InSourceData.StaticData.Cast<FDTrackHandStaticData>();
	const FLiveLinkAnimationFrameData* frame_data = InSourceData.FrameData.Cast<FLiveLinkAnimationFrameData>();
	if (blueprint_data && static_data && frame_data)
	{
		GetStaticDataStruct()->CopyScriptStruct(&blueprint_data->m_static_data, static_data);
		GetFrameDataStruct()->CopyScriptStruct(&blueprint_data->m_frame_data, frame_data);
		is_success = true;
	}

	return is_success;
}

FText UDTrackHandRole::GetDisplayName() const {

	return LOCTEXT("DTrackHandRole", "DTrack Hand");
}

#undef LOCTEXT_NAMESPACE