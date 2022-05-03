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

#include "LiveLinkSourceSettings.h"

#include "DTrackInterface.h"

#include "DTrackLiveLinkSourceSettings.generated.h"


USTRUCT()
struct FDTrackServerSettings
{
	GENERATED_BODY()

public:

	bool operator==(const FDTrackServerSettings& Other) const
	{
		return m_dtrack_server_ip == Other.m_dtrack_server_ip
			&& m_dtrack_server_port == Other.m_dtrack_server_port
			&& m_dtrack_start_mea == Other.m_dtrack_start_mea
			&& m_coordinate_system == Other.m_coordinate_system;
	}

	bool operator!=(const FDTrackServerSettings& Other) const
	{
		return !(*this == Other);
	}

public:

	UPROPERTY(EditAnywhere, Category = "Server Settings", meta = (DisplayName = "DTrack Data Port", ToolTip = "Port your server sends data to"))
	int32 m_dtrack_server_port = 5000;

	UPROPERTY(EditAnywhere, Category = "Server Settings", meta = (DisplayName = "Start DTrack Measurement", ToolTip = "Start measurement via the DTrack2 TCP command channel"))
	bool m_dtrack_start_mea = false;
	
	UPROPERTY(EditAnywhere, Category = "Server Settings", meta = (DisplayName = "DTrack Server IP", ToolTip = "IP of DTrack server host for starting measurement"))
	FString m_dtrack_server_ip = "127.0.0.1";

	UPROPERTY(EditAnywhere, Category = "Server Settings", meta = (DisplayName = "DTrack Room Calibration", ToolTip = "Set this according to your DTrack system's room calibration"))
	EDTrackCoordinateSystemType m_coordinate_system = EDTrackCoordinateSystemType::CST_Unreal_Adapted;
};

UCLASS()
class DTRACKPLUGIN_API UDTrackLiveLinkSourceSettings : public ULiveLinkSourceSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Dtrack settings")
	FDTrackServerSettings m_server_settings;
};
