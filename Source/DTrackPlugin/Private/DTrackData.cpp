/* DTrackData: C++ source file
 *
 * DTrackSDK: data helper routines.
 *
 * Copyright 2020-2021, Advanced Realtime Tracking GmbH & Co. KG
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Version v2.7.0
 * 
 */

#include <cmath>

#include "DTrackDataTypes.hpp"

namespace DTrackSDK_Datatypes {

// -----------------------------------------------------------------------------------------------------

/*
 * Helper to convert a rotation matrix into a quaternion.
 */
DTrackQuaternion rot2quat( const double rot[ 9 ] )
{
	DTrackQuaternion quat;
	double tr, s;

	tr = rot[ 0 ] + rot[ 4 ] + rot[ 8 ];

	if ( tr > 0 )
	{
		s = std::sqrt( 1.0 + tr );

		quat.w = 0.5 * s;

		s = 0.5 / s;  // = 1 / (4 * w)

		quat.x = ( rot[ 5 ] - rot[ 7 ] ) * s ;
		quat.y = ( rot[ 6 ] - rot[ 2 ] ) * s ;
		quat.z = ( rot[ 1 ] - rot[ 3 ] ) * s ;
	}
	else
	{
		if ( ( rot[ 0 ] > rot[ 4 ] ) && ( rot[ 0 ] > rot[ 8 ] ) )
		{
			s = std::sqrt( 1.0 + rot[ 0 ] - rot[ 4 ] - rot[ 8 ] );

			quat.x = 0.5 * s;

			s = 0.5 / s;  // = 1 / (4 * x)

			quat.y = ( rot[ 1 ] + rot[ 3 ] ) * s ;
			quat.z = ( rot[ 2 ] + rot[ 6 ] ) * s ;
			quat.w = ( rot[ 5 ] - rot[ 7 ] ) * s ;
		}
		else if ( rot[ 4 ] > rot[ 8 ] )
		{
			s = std::sqrt( 1.0 - rot[ 0 ] + rot[ 4 ] - rot[ 8 ] );

			quat.y = 0.5 * s;

			s = 0.5 / s;  // = 1 / (4 * y)

			quat.x = ( rot[ 1 ] + rot[ 3 ] ) * s ;
			quat.z = ( rot[ 5 ] + rot[ 7 ] ) * s ;
			quat.w = ( rot[ 6 ] - rot[ 2 ] ) * s ;
		}
		else
		{
			s = std::sqrt( 1.0 - rot[ 0 ] - rot[ 4 ] + rot[ 8 ] );

			quat.z = 0.5 * s;

			s = 0.5 / s;  // = 1 / (4 * z)

			quat.x = ( rot[ 2 ] + rot[ 6 ] ) * s ;
			quat.y = ( rot[ 5 ] + rot[ 7 ] ) * s ;
			quat.w = ( rot[ 1 ] - rot[ 3 ] ) * s ;
		}
	}

	return quat;
}


}  // namespace DTrackSDK_Datatypes

