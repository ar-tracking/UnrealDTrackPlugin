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

#include "ILiveLinkSource.h"

#if PLATFORM_WINDOWS
	// Avoid 'warning C4005' when including DTrackSDK
	#include "Windows/AllowWindowsPlatformTypes.h"
	#include "Windows/PreWindowsApi.h"
#endif

#include "DTrackSDK.hpp"

#if PLATFORM_WINDOWS
	#include "Windows/PostWindowsApi.h"
	#include "Windows/HideWindowsPlatformTypes.h"
#endif

#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"

#include "DTrackLiveLinkSourceSettings.h"


class FDTrackLiveLinkSource;

/**
 * Class to handle DTrack data reception
 */
class DTRACKPLUGIN_API FDTrackSDKHandler : public FRunnable
{
public:

	FDTrackSDKHandler(FDTrackLiveLinkSource* n_livelink_source);
	virtual ~FDTrackSDKHandler();

public:
	
	// Open a connection with DTrack SDK with the desired server settings
	bool start_listening(const FDTrackServerSettings& n_server_settings);

	// Stops the server listening thread if any and stop measurements on the SDK
	void stop_listening();

public:

	// Returns true if connection to server is active
	bool is_active() const;

	// Gets the status of the sdk to see if an error is present
	FString get_status() const;

public:
	//~ Begin FRunnable interface
	virtual uint32 Run() override;
	virtual void Stop() override;
	//~ End FRunnable interface


protected:

	/// Each time we received data, we update the time for this frame. Either using the timestamp or the current time.
	void update_frametime();

	/// after receive, treat body info and send it to listeners
	void handle_bodies();

	/// after receive, treat flystick info and send it to listeners
	void handle_flysticks();

	/// translate dtrack rotation matrix to rotator according to selected room calibration
	FRotator from_dtrack_rotation(const double(&n_matrix)[9]);

	/// translate dtrack translation to unreal space
	FVector from_dtrack_location(const double(&n_translation)[3]);

	/// Start measurement on the server if it's not already running. Returns false if it failed
	bool start_measurement();

	/// Stop measurement on the server if it's running. Returns false if it failed
	bool stop_measurement();

	/// Returns string representation of the last error from the server
	FString get_last_error_string() const;

private:

	// Current ART server settings
	FDTrackServerSettings m_server_settings;
	
	// SDK pointer to access received data
	TUniquePtr<DTrackSDK> m_dtrack;

	// LiveLink Source that owns us
	FDTrackLiveLinkSource* m_livelink_source;

	// Thread safe bool for stopping the thread
	FThreadSafeBool m_is_active;

	// Flag set when measurement was started for dtrack2
	FThreadSafeBool m_is_measuring;

	// Flag set during dtrack sdk connection
	FThreadSafeBool m_is_connecting;

	// Thread where we poll the sdk for new frames
	TUniquePtr<FRunnableThread> m_thread;

	// World time at which we received the frames
	double m_frame_worldtime;

	// Timestamp if available for the frame. -1.0 if not available
	double m_frame_timestamp_seconds;

private:

	/// Number of seconds per day for timestamp adjustments
	static const int32 m_seconds_per_day;

	/// room coordinate adoption matrix for "normal" setting
	static const FMatrix  m_trafo_normal;

	/// transposed variant cached
	static const FMatrix  m_trafo_normal_transposed;

	/// room coordinate adoption matrix for "power wall" setting
	static const FMatrix  m_trafo_powerwall;

	/// transposed variant cached
	static const FMatrix  m_trafo_powerwall_transposed;

	/// room coordinate adoption matrix for "unreal adapted" setting
	static const FMatrix  m_trafo_unreal_adapted;

	/// transposed variant cached
	static const FMatrix  m_trafo_unreal_adapted_transposed;
};
