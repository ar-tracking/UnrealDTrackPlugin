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

#include "DTrackPlugin.h"

#include "DTrackLiveLinkSource.h"
#include "Logging/LogMacros.h"
#include "Misc/CoreDelegates.h"



DEFINE_LOG_CATEGORY(LogDTrackPlugin);

void FDTrackPlugin::StartupModule() {

	// Hook to the PreExit callback, to cleanup live link source
	FCoreDelegates::OnPreExit.AddRaw(this, &FDTrackPlugin::handle_app_pre_exit);
}

void FDTrackPlugin::ShutdownModule() {

	FCoreDelegates::OnPreExit.RemoveAll(this);
}

TSharedPtr<ILiveLinkSource> FDTrackPlugin::get_livelink_source() {

	if (!m_livelink_source.IsValid())
	{
		create_livelink_source();
	}

	return m_livelink_source;
}

TSharedPtr<FDTrackLiveLinkSource> FDTrackPlugin::get_fdtrack_livelink_source() {

	if (!m_livelink_source.IsValid())
	{
		create_livelink_source();
	}

	return m_livelink_source;
}

bool FDTrackPlugin::is_livelink_source_valid() const {

	return m_livelink_source.IsValid();
}

TSharedPtr<FDTrackLiveLinkSource> FDTrackPlugin::create_livelink_source() {

	if (!m_livelink_source.IsValid()) {
		m_livelink_source = MakeShared<FDTrackLiveLinkSource>();
	}

	return m_livelink_source;
}

void FDTrackPlugin::handle_app_pre_exit() {

	if (m_livelink_source.IsValid()) {

		m_livelink_source->RequestSourceShutdown();
	}
}

IMPLEMENT_MODULE(FDTrackPlugin, DTrackPlugin);
