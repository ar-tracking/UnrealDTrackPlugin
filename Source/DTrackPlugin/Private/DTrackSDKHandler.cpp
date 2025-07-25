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

#include "DTrackSDKHandler.h"
#include "DTrackPlugin.h"

#include "DTrackLiveLinkSource.h"
#include "HAL/RunnableThread.h"
#include "Math/UnrealMathUtility.h"


/**
 * FDTrackSDKHandler static const variable initialization
 */

const FMatrix FDTrackSDKHandler::m_trafo_normal = FMatrix(
	  FPlane( 1.0f,  0.0f, 0.0f, 0.0f )
	, FPlane( 0.0f, -1.0f, 0.0f, 0.0f )
	, FPlane( 0.0f,  0.0f, 1.0f, 0.0f )
	, FPlane( 0.0f,  0.0f, 0.0f, 1.0f ));

const FMatrix FDTrackSDKHandler::m_trafo_normal_transposed(m_trafo_normal.GetTransposed());

const FMatrix FDTrackSDKHandler::m_trafo_powerwall = FMatrix(
	  FPlane( 1.0f, 0.0f,  0.0f, 0.0f )
	, FPlane( 0.0f, 0.0f,  1.0f, 0.0f )
	, FPlane( 0.0f, 1.0f,  0.0f, 0.0f )
	, FPlane( 0.0f, 0.0f,  0.0f, 1.0f ));

const FMatrix FDTrackSDKHandler::m_trafo_powerwall_transposed(m_trafo_powerwall.GetTransposed());


/**
 * FDTrackSDKHandler
 */

FDTrackSDKHandler::FDTrackSDKHandler(FDTrackLiveLinkSource* n_livelink_source)
	: m_livelink_source(n_livelink_source)
	, m_frame_worldtime(-1.0)
	, m_frame_timestamp_seconds(-1.0)
{
}

FDTrackSDKHandler::~FDTrackSDKHandler() {

	stop_listening();
}

bool FDTrackSDKHandler::start_listening(const FDTrackServerSettings& n_server_settings) {

	stop_listening();

	m_server_settings = n_server_settings;

#if 0
	switch (m_server_settings.m_coordinate_system) {
	case EDTrackCoordinateSystemType::CST_Normal:
		UE_LOG( LogDTrackPlugin, Warning, TEXT("EDTrackCoordinateSystemType::CST_Normal") );
		break;
	case EDTrackCoordinateSystemType::CST_Powerwall:
		UE_LOG( LogDTrackPlugin, Warning, TEXT("EDTrackCoordinateSystemType::CST_Powerwall") );
	default:
		UE_LOG( LogDTrackPlugin, Warning, TEXT("No suitable EDTrackCoordinateSystemType") );
	}
#endif

	m_thread.Reset(FRunnableThread::Create(this, TEXT("DTrackSDKHandler")));
	return true;
}

void FDTrackSDKHandler::stop_listening() {

	if (m_thread) {
		if (m_is_active) {
			Stop();
			m_thread->Kill(true);
		}

		m_thread.Reset();
	}
}

bool FDTrackSDKHandler::is_active() const {
	return m_is_active;
}

FString FDTrackSDKHandler::get_status() const {

	if (m_is_connecting) {
		return FString(TEXT("Connecting"));
	}
	else if (m_is_active) {

		const FString error = get_last_error_string();
		if (error.IsEmpty())
		{
			/*if(!m_dtrack->getLastDataError())

				return FString(TEXT("Invalid DataPort"));*/
			return FString(TEXT("Active"));
		}
		else {
			return error;
		}
	}
	else {
		return FString(TEXT("Not connected"));
	}
}

void FDTrackSDKHandler::update_frametime() {

	m_frame_worldtime = FPlatformTime::Seconds();
	m_frame_timestamp_seconds = m_dtrack->getTimeStamp();
}

void FDTrackSDKHandler::handle_bodies() {

	const DTrack_Body_Type_d *body = nullptr;
	for (int i = 0; i < m_dtrack->getNumBody(); i++) { 

		body = m_dtrack->getBody(i);
		checkf(body, TEXT("DTrack API error, body address null"));

		const FVector translation = from_dtrack_location(body->loc);
		const FRotator rotation = from_dtrack_rotation(body->rot);
		m_livelink_source->handle_body_data_anythread(m_frame_worldtime, m_frame_timestamp_seconds, body->id, body->quality, translation, rotation);
	}
}

void FDTrackSDKHandler::handle_flysticks() 
{
#if 0
	// DBG
	UE_LOG( LogDTrackPlugin, Warning, TEXT("#flysticks %d"), m_dtrack->getNumFlyStick() );
	for ( int i = 0;  i < m_dtrack->getNumFlyStick();  ++i) {
		const DTrack_FlyStick_Type_d *fs = m_dtrack->getFlyStick(i);
		UE_LOG( LogDTrackPlugin, Warning, TEXT("flystick %d #buttons %d #joysticks %d"), i, fs->num_button, fs->num_joystick);
	}
#endif

	const DTrack_FlyStick_Type_d *flystick = nullptr;
	for (int i = 0; i < m_dtrack->getNumFlyStick(); i++) {
		flystick = m_dtrack->getFlyStick(i);
		checkf(flystick, TEXT("DTrack API error, flystick address null"));

		const FVector translation = from_dtrack_location(flystick->loc);
		const FRotator rotation = from_dtrack_rotation(flystick->rot);

		// create a state vector for the button states
		TArray<bool> buttons;
		buttons.SetNumZeroed(flystick->num_button);
		for (int idx = 0; idx < flystick->num_button; idx++) {
			buttons[idx] = flystick->button[idx] == 1;
		}

		// create a state vector for the joystick states
		TArray<float> joysticks;  // have to use float as blueprints don't support TArray<double>
		joysticks.SetNumZeroed(flystick->num_joystick);
		for (int idx = 0; idx < flystick->num_joystick; idx++) {
			joysticks[idx] = static_cast<float>(flystick->joystick[idx]);
		}

		m_livelink_source->handle_flystick_data_anythread( m_frame_worldtime, m_frame_timestamp_seconds, 
			flystick->id, flystick->quality, translation, rotation, buttons, joysticks);
	}
}


void FDTrackSDKHandler::handle_hands()
{
	const DTrackHand *hand = nullptr;

#if 0
	UE_LOG (LogDTrackPlugin, Warning, TEXT("DTrackSDKHandler: NumHand:   %d"),  m_dtrack->getNumHand() );
#endif

	for ( int i = 0; i < m_dtrack->getNumHand(); ++i )
	{
		hand = m_dtrack->getHand(i);
		checkf(hand, TEXT("DTrack API error, hand address is null"));

		FVector location  = from_dtrack_location( hand->loc );
		FRotator rotation = from_dtrack_rotation( hand->rot );
		FVector scale     = FVector( 1.0f, 1.0f, 1.0f );

		TArray<FDTrackFinger> fingers;
		fingers.Reserve( hand->nfinger );

		TArray<EDTrackFingerType> fingers_type;
		fingers_type.SetNumUninitialized( hand->nfinger );

		FTransform handTransform;
		handTransform.SetComponents( rotation.Quaternion(), location, scale );

		for (int j = 0; j < hand->nfinger; ++j)
		{
			FDTrackFinger finger;

			switch (j) 
			{
			// More expressive in Blueprints than just indices
			case 0: fingers_type[j] = EDTrackFingerType::FT_Thumb;  break;
			case 1: fingers_type[j] = EDTrackFingerType::FT_Index;  break;
			case 2: fingers_type[j] = EDTrackFingerType::FT_Middle; break;
			case 3: fingers_type[j] = EDTrackFingerType::FT_Ring;   break;
			case 4: fingers_type[j] = EDTrackFingerType::FT_Pinky;  break;
			}

			// In hand space coordinate (see DTrack2 Manual Technical Appendix)
			finger.m_tip_transform.SetComponents(
				from_dtrack_rotation( hand->finger[j].rot ).Quaternion(),
				from_dtrack_location( hand->finger[j].loc ),
				scale );
			finger.m_tip_radius                 = hand->finger[j].radiustip;
			finger.m_inner_phalanx_length       = hand->finger[j].lengthphalanx[2];
			finger.m_middle_phalanx_length      = hand->finger[j].lengthphalanx[1];
			finger.m_outer_phalanx_length       = hand->finger[j].lengthphalanx[0];
			finger.m_inner_middle_phalanx_angle = hand->finger[j].anglephalanx[1];
			finger.m_middle_outer_phalanx_angle = hand->finger[j].anglephalanx[0];


			// Left-Hand is 0, Right-Hand is 1
			if ( hand->lr == 0 )
			{
				compute_finger_joint_pose( handTransform, finger,  0.f,  0.f,  -90.f );
				//compute_finger_joint_pose( handTransform, finger,  0.f,  0.f,  0.f );
			}
			else
			{   // 180 for "x" in HandSpace is "z" -> RightHandSpace-Z is flipped by 180 compared to LeftHandSpace-Z
				compute_finger_joint_pose( handTransform, finger,  180.f,  0.f, -90.f );
			}
			fingers.Add( MoveTemp(finger) );
		}
		
		if ( hand->lr == 0 )
		{
			rotation.Add( 0.f, 0.f, -90.f );
		}
		else
		{
			// mirror X-Axis because Unreal-Right-Hand-Skeletons X direction is negative compared to the left hand
			rotation.SetComponentForAxis( EAxis::X,  -1.0f * rotation.GetComponentForAxis(EAxis::X) );
			rotation.Add( 180.f, 0.f, 90.f );
		}

		handTransform.SetRotation( rotation.Quaternion() );

		m_livelink_source->handle_hand_data_anythread(
			m_frame_worldtime, m_frame_timestamp_seconds, hand->id, hand->quality, 
			(hand->lr == 1), handTransform, fingers_type, fingers);
	}
}



void FDTrackSDKHandler::compute_finger_joint_pose(
	const FTransform& n_hand_transform, FDTrackFinger& out_finger, const float finger_x_rotation, 
	const float finger_y_rotation, const float finger_z_rotation)
{
	double dtrack_finger_space_loc[3];

	// calculating the position for tip and all joints of the fingers in world space

	// first joint (in finger coordinate system)
	dtrack_finger_space_loc[0] = -out_finger.m_outer_phalanx_length;
	dtrack_finger_space_loc[1] = dtrack_finger_space_loc[2] = 0.0;
	const FVector finger_space_location_outerphalanx = from_dtrack_location(dtrack_finger_space_loc);

	//Convert outer phalanx to hand space
	const FVector hand_space_outerphalanx_location = out_finger.m_tip_transform.TransformPosition(finger_space_location_outerphalanx);
	const FQuat hand_space_outerphalanx_rotation = out_finger.m_tip_transform.GetRotation() * FRotator(finger_x_rotation, finger_y_rotation, finger_z_rotation).Quaternion();

	//Convert outer phalanx to world space
	FTransform world_outer_phalanx_transform;
	world_outer_phalanx_transform.SetLocation(n_hand_transform.TransformPosition(hand_space_outerphalanx_location));
	world_outer_phalanx_transform.SetRotation(n_hand_transform.TransformRotation(hand_space_outerphalanx_rotation));
	out_finger.m_outer_phalanx_transform = MoveTemp(world_outer_phalanx_transform);


	// second joint (in finger coordinate system)
	dtrack_finger_space_loc[0] = -out_finger.m_outer_phalanx_length - out_finger.m_middle_phalanx_length * cos(out_finger.m_middle_outer_phalanx_angle * PI / 180.0);
	dtrack_finger_space_loc[1] = 0.0;
	dtrack_finger_space_loc[2] = out_finger.m_middle_phalanx_length * sin(out_finger.m_middle_outer_phalanx_angle * PI / 180.0);
	const FVector finger_space_location_middlephalanx = from_dtrack_location(dtrack_finger_space_loc);
	const FQuat finger_space_rotation_middlephalanx = FQuat(FVector(0.0f, 1.0f, 0.0f), out_finger.m_middle_outer_phalanx_angle * PI / 180.f).Rotator().Add(finger_x_rotation, finger_y_rotation, finger_z_rotation).Quaternion();

	//Convert middle phalanx to hand space
	const FVector hand_space_middlephalanx_location = out_finger.m_tip_transform.TransformPosition(finger_space_location_middlephalanx);
	const FQuat hand_space_middlephalanx_rotation = out_finger.m_tip_transform.TransformRotation(finger_space_rotation_middlephalanx);

	//Convert middle phalanx to world space
	FTransform world_middle_phalanx_transform;
	world_middle_phalanx_transform.SetLocation(n_hand_transform.TransformPosition(hand_space_middlephalanx_location));
	world_middle_phalanx_transform.SetRotation(n_hand_transform.TransformRotation(hand_space_middlephalanx_rotation));
	out_finger.m_middle_phalanx_transform = MoveTemp(world_middle_phalanx_transform);


	// third joint (in finger coordinate system)
	dtrack_finger_space_loc[0] = -out_finger.m_outer_phalanx_length
		- out_finger.m_middle_phalanx_length * cos(out_finger.m_middle_outer_phalanx_angle * PI / 180.0)
		- out_finger.m_inner_phalanx_length * cos((out_finger.m_middle_outer_phalanx_angle + out_finger.m_inner_middle_phalanx_angle) * PI / 180.0);
	dtrack_finger_space_loc[1] = 0.0;
	dtrack_finger_space_loc[2] = out_finger.m_middle_phalanx_length * sin(out_finger.m_middle_outer_phalanx_angle * PI / 180.0)
		+ out_finger.m_inner_phalanx_length * sin((out_finger.m_middle_outer_phalanx_angle + out_finger.m_inner_middle_phalanx_angle) * PI / 180.0);
	const FVector finger_space_location_innerphalanx = from_dtrack_location(dtrack_finger_space_loc);
	const FQuat finger_space_rotation_innerphalanx = FQuat(FVector(0.0f, 1.0f, 0.0f), (out_finger.m_middle_outer_phalanx_angle + out_finger.m_inner_middle_phalanx_angle) * PI / 180.0).Rotator().Add(finger_x_rotation, finger_y_rotation, finger_z_rotation).Quaternion();
	
	// Convert inner phalanx to hand space
	const FVector hand_space_innerphalanx_location = out_finger.m_tip_transform.TransformPosition(finger_space_location_innerphalanx);
	const FQuat hand_space_innerphalanx_rotation = out_finger.m_tip_transform.TransformRotation(finger_space_rotation_innerphalanx);

	// Convert inner phalanx to world space
	FTransform world_inner_phalanx_transform;
	world_inner_phalanx_transform.SetLocation(n_hand_transform.TransformPosition(hand_space_innerphalanx_location));
	world_inner_phalanx_transform.SetRotation(n_hand_transform.TransformRotation(hand_space_innerphalanx_rotation));
	out_finger.m_inner_phalanx_transform = MoveTemp(world_inner_phalanx_transform);

	// Finger tip is already in hand space and hand is in world space
	out_finger.m_tip_transform = out_finger.m_tip_transform * n_hand_transform;
}


FString FDTrackSDKHandler::fingerName( int i )
{
	switch (i) 
	{
	case 0: return FString("Thumb");  break;
	case 1: return FString("Index");  break;
	case 2: return FString("Middle"); break;
	case 3: return FString("Ring");   break;
	case 4: return FString("Pinky");  break;
	}
	return FString("Unknown");
}



uint32 FDTrackSDKHandler::Run()
{
	const FDTrackServerSettings CopiedSettings = m_server_settings;

	// Initialization part

	m_is_connecting = true;

	if (CopiedSettings.m_dtrack_start_mea || CopiedSettings.m_dtrack_tactile_fingers) {
		UE_LOG(LogDTrackPlugin, Verbose, TEXT("Connecting to DTrack2 server with IP '%s' on port '%d'."), *CopiedSettings.m_dtrack_server_ip, m_server_settings.m_dtrack_server_port);
		m_dtrack = MakeUnique<DTrackSDK>(TCHAR_TO_UTF8(*CopiedSettings.m_dtrack_server_ip), CopiedSettings.m_dtrack_server_port);
	}
	else {
		UE_LOG(LogDTrackPlugin, Verbose, TEXT("Connecting to DTrack server on port '%d'."), CopiedSettings.m_dtrack_server_port);
		m_dtrack = MakeUnique<DTrackSDK>(CopiedSettings.m_dtrack_server_port);
	}
	
	if (m_dtrack->isLocalDataPortValid()) {

		// start the tracking via tcp route if dtrack2 mode is enabled
		if (CopiedSettings.m_dtrack_start_mea) {
			UE_LOG(LogDTrackPlugin, Verbose, TEXT("Starting DTrack2 measurement."));
			if (start_measurement()) {
				m_is_measuring = true;
				m_is_active = true;
			}
			else {

				const FString error = get_last_error_string();
				UE_LOG(LogDTrackPlugin, Error, TEXT("Could not start measurement : '%s'"), *error);
			}
		}
		else {
			m_is_active = true;
		}
	}
	else {

		UE_LOG(LogDTrackPlugin, Error, TEXT("Could not start tracking, port '%d' is not usable."), CopiedSettings.m_dtrack_server_port);
	}

	m_is_connecting = false;


	while (m_is_active)	{
		if (m_dtrack->receive()) {
			update_frametime();

			handle_bodies();
			handle_flysticks();
			handle_hands();
		}
	}

	if (CopiedSettings.m_dtrack_start_mea && m_is_measuring) {
		UE_LOG(LogDTrackPlugin, Verbose, TEXT("Stopping DTrack2 measurement."));
		if (!stop_measurement()) {
			UE_LOG(LogDTrackPlugin, Warning, TEXT("Could not stop DTrack2 measurement : '%s'."), *get_last_error_string());
		}

		m_is_measuring = false;
	}

	m_dtrack.Reset();
	
	UE_LOG(LogDTrackPlugin, VeryVerbose, TEXT("Workerthread stopped polling sdk."));

	return 0;
}

void FDTrackSDKHandler::Stop() {
	m_is_active = false;
}

// translate a DTrack body location (translation in mm) into Unreal Location (in cm)
FVector FDTrackSDKHandler::from_dtrack_location(const double(&n_translation)[3]) {

	FVector ret;

	// DTrack coordinates come in mm with either Z or Y being up, which has to be configured by the user.
	// I translate to Unreal's Z being up and cm units.
	switch (m_server_settings.m_coordinate_system) {
		default:
		case EDTrackCoordinateSystemType::CST_Normal:
			ret.X =  n_translation[0]  / 10.0;
			ret.Y = -n_translation[1]  / 10.0;
			ret.Z =  n_translation[2]  / 10.0;
			break;
		case EDTrackCoordinateSystemType::CST_Powerwall:
			ret.X =  n_translation[0]  / 10.0;
			ret.Y =  n_translation[2]  / 10.0;
			ret.Z =  n_translation[1]  / 10.0;
			break;
	}

	return ret;
}

// translate a DTrack 3x3 rotation matrix to Unreal conventions
FRotator FDTrackSDKHandler::from_dtrack_rotation(const double(&n_matrix)[9]) {

	// ( M[RowIndex][ColumnIndex], DTrack matrix comes column-wise )
	FMatrix r;
	r.M[0][0] = n_matrix[0 + 0]; r.M[0][1] = n_matrix[0 + 3]; r.M[0][2] = n_matrix[0 + 6]; r.M[0][3] = 0.0;
	r.M[1][0] = n_matrix[1 + 0]; r.M[1][1] = n_matrix[1 + 3]; r.M[1][2] = n_matrix[1 + 6]; r.M[1][3] = 0.0;
	r.M[2][0] = n_matrix[2 + 0]; r.M[2][1] = n_matrix[2 + 3]; r.M[2][2] = n_matrix[2 + 6]; r.M[2][3] = 0.0;
	r.M[3][0] = 0.0;			 r.M[3][1] = 0.0;			  r.M[3][2] = 0.0;			   r.M[3][3] = 1.0;

	FMatrix r_adapted;

	switch (m_server_settings.m_coordinate_system) {
	default:
	case EDTrackCoordinateSystemType::CST_Normal:
		r_adapted = m_trafo_normal * r * m_trafo_normal_transposed;
		break;

	case EDTrackCoordinateSystemType::CST_Powerwall:
		r_adapted = m_trafo_powerwall * r * m_trafo_powerwall_transposed;
		break;
	}	

	//r_adapted.Mirror(static_cast<EAxis::Type>(m_server_settings.m_mirror_axis), static_cast<EAxis::Type>(m_server_settings.m_flip_axis));
	return r_adapted.GetTransposed().Rotator();
}


bool FDTrackSDKHandler::start_measurement() {

	std::string trackingStatus;

	// check tracking status
	bool success = m_dtrack->getParam("status active", trackingStatus);
	if (success)
	{
		// check the answer (possibilities: none, cal, mea, wait or err)
		if (trackingStatus.compare("mea") == 0) {
			return true;
		}
		else {
			return m_dtrack->startMeasurement();
		}
	}

	return false;
}

bool FDTrackSDKHandler::stop_measurement() {

	std::string trackingStatus;

	// check if measurement is already running
	bool success = m_dtrack->getParam("status active", trackingStatus);
	if (success) {
		// check the answer (possibilities: none, cal, mea, wait or err)
		if (trackingStatus.compare("none") != 0 && trackingStatus.compare("err") != 0)
		{
			return m_dtrack->stopMeasurement();
		}
	}
	return false;
}

FString FDTrackSDKHandler::get_last_error_string() const {

	if (m_dtrack->getLastDTrackError() != 0) {
		return FString(UTF8_TO_TCHAR(m_dtrack->getLastDTrackErrorDescription().c_str()));
	}
	else if (m_dtrack->getLastServerError() == DTrackSDK::ERR_NET) {
		return FString(TEXT("Network error"));
	}
	else if (m_dtrack->getLastDataError() == DTrackSDK::ERR_PARSE) {
		return FString(TEXT("Parsing error"));
	}
	else if (m_dtrack->getLastDataError() == DTrackSDK::ERR_TIMEOUT) {
		return FString(TEXT("Timeout error"));
	}
	else {
		return FString();
	}
}

