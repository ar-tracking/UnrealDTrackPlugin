/* DTrackSDK: C++ source file, A.R.T. GmbH
 *
 * DTrackSDK: functions to receive and process DTrack UDP packets (ASCII protocol), as
 * well as to exchange DTrack2/DTrack3 TCP command strings.
 *
 * Copyright (c) 2007-2020, Advanced Realtime Tracking GmbH
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
 * Version v2.6.0
 *
 * Purpose:
 *  - receives DTrack UDP packets (ASCII protocol) and converts them into easier to handle data
 *  - sends and receives DTrack2/DTrack3 commands (TCP)
 *  - DTrack network protocol according to:
 *    'DTrack2 User Manual, Technical Appendix' or 'DTrack3 Programmer's Guide'
 */

#include <sstream>

#include "DTrackSDK.hpp"
#include "DTrackParse.hpp"

#include <cstring>
#include <cstdlib>
#include <clocale>

// use Visual Studio specific method to avoid warnings
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
	#define strdup _strdup
#endif

using namespace DTrackNet;
using namespace DTrackSDK_Parse;


/*
 * Universal constructor. Can be used for any mode.
 */
DTrackSDK::DTrackSDK( const std::string& connection )
{
	std::string host;
	std::istringstream portstream;

	size_t ind = connection.find_last_of( ':' );
	if ( ind != std::string::npos )
	{
		host = connection.substr( 0, ind );
		portstream.str( connection.substr( ind + 1 ) );
	}
	else
	{
		portstream.str( connection );
	}

	int port;
	portstream >> port;  // data port
	if ( portstream.fail() || port < 0 || port >= 65536 )  return;  // invalid port number

	if ( host.empty() )
	{
		init( "", 0, port, SYS_DTRACK_UNKNOWN );
	}
	else
	{
		init( host, DTRACK2_PORT_COMMAND, port, SYS_DTRACK_2 );
	}
}


/*
 * Constructor. Use for pure listening mode.
 */
DTrackSDK::DTrackSDK(unsigned short data_port)
    : DTrackParser()
{
	init("", 0, data_port, SYS_DTRACK_UNKNOWN);
}


/*
 * Constructor. Use for communicating mode with DTrack2/DTrack3.
 */
DTrackSDK::DTrackSDK(const std::string& server_host, unsigned short data_port)
    : DTrackParser()
{
	init( server_host, DTRACK2_PORT_COMMAND, data_port, SYS_DTRACK_2 );
}


/*
 * Constructor. Use for communicating mode with DTrack1.
 */
DTrackSDK::DTrackSDK(const std::string& server_host, unsigned short server_port, unsigned short data_port)
    : DTrackParser()
{
	if ( server_port == DTRACK2_PORT_COMMAND )
	{
		init( server_host, server_port, data_port, SYS_DTRACK_UNKNOWN );  // due to compatibility to DTrackSDK v2.0.0 
	}
	else
	{
		init( server_host, server_port, data_port, SYS_DTRACK );
	}
}


/*
 * General constructor. DEPRECATED.
 */
DTrackSDK::DTrackSDK(const std::string& server_host, unsigned short server_port, unsigned short data_port,
                     RemoteSystemType remote_type, int data_bufsize, int data_timeout_us, int srv_timeout_us)
    : DTrackParser()
{
	init( server_host, server_port, data_port, remote_type );

	setDataTimeoutUS( data_timeout_us );
	setCommandTimeoutUS( srv_timeout_us );
	setDataBufferSize( data_bufsize );  // updates also UDP buffer
}


/*
 * Private init called by constructor.
 */
void DTrackSDK::init( const std::string& server_host, unsigned short server_port, unsigned short data_port,
                      RemoteSystemType remote_type )
{
	setlocale( LC_NUMERIC, "C" );
	
	rsType = remote_type;
	
	d_udp = NULL;
	d_tcp = NULL;
	d_udpbuf = NULL;
	d_udpbufsize = 0;
	
	lastDataError = ERR_NONE;
	lastServerError = ERR_NONE;
	setLastDTrackError();

	setDataTimeoutUS( 0 );
	setCommandTimeoutUS( 0 );
	setDataBufferSize( 0 );  // creates also UDP buffer

	d_remoteport = server_port;
	
	net_init();
	
	// parse remote address if available
	d_remote_ip = 0;
	if (!server_host.empty()) {
		d_remote_ip = ip_name2ip(server_host.c_str());
	}

	bool isMulticast = false;
	if ( ( d_remote_ip & 0xf0000000 ) == 0xe0000000 )  // check if multicast IP
		isMulticast = true;

	// create UDP socket:
	
	if ( isMulticast )  // listen to multicast case
	{
		d_udp = new DTrackNet::UDP( data_port, d_remote_ip );
	} else { // normal case
		d_udp = new DTrackNet::UDP( data_port );
	}
	if ( ! d_udp->isValid() )
		return;

	if (d_remote_ip) {
		if ( isMulticast )  // multicast
		{
			d_remoteport = 0;
		} else {
			if (rsType != SYS_DTRACK) {
				d_tcp = new DTrackNet::TCP( d_remote_ip, server_port );
				if ( ! d_tcp->isValid() )  // no connection to DTrack2/DTrack3 server
				{
					// on error assuming DTrack if system is unknown
					if (rsType == SYS_DTRACK_UNKNOWN)
					{
						rsType = SYS_DTRACK;
						// DTrack will not listen to tcp port 50105 -> ignore tcp connection
					}
				} else {
					// TCP connection up, should be DTrack2
					rsType = SYS_DTRACK_2;
				}
			}
		}
	}

	d_message_origin = "";
	d_message_status = "";
	d_message_framenr = 0;
	d_message_errorid = 0;
	d_message_msg = "";
}


/*
 * Destructor.
 */
DTrackSDK::~DTrackSDK()
{
	// release buffer
	free(d_udpbuf);
	
	// release sockets & net
	delete d_udp;
	delete d_tcp;
	net_exit();
}


/*
 * Returns if UDP socket is open to receive tracking data on local machine.
 */
bool DTrackSDK::isDataInterfaceValid() const
{
	if ( d_udp == NULL )  return false;

	return d_udp->isValid();
}


/*
 * Get UDP data port where tracking data is received.
 */
unsigned short DTrackSDK::getDataPort() const
{
	return d_udp->getPort();
}


/*
 * Returns if TCP connection for DTrack2/DTrack3 commands is active.
 */
bool DTrackSDK::isCommandInterfaceValid() const
{
	if ( d_tcp == NULL )  return false;

	return d_tcp->isValid();
}


/*
 * Get current remote system type (e.g. DTrack1, DTrack2/DTrack3).
 */
DTrackSDK::RemoteSystemType DTrackSDK::getRemoteSystemType() const
{
	return rsType;
}


/*
 * Set UDP timeout for receiving tracking data.
 */
bool DTrackSDK::setDataTimeoutUS( int timeout )
{
	if ( timeout <= 0 )
	{
		d_udptimeout_us = DEFAULT_UDP_TIMEOUT;
	}
	else
	{
		d_udptimeout_us = timeout;
	}
	return true;
}


/*
 * Set TCP timeout for exchanging commands with Controller.
 */
bool DTrackSDK::setCommandTimeoutUS( int timeout )
{
	if ( timeout <= 0 )
	{
		d_tcptimeout_us = DEFAULT_TCP_TIMEOUT;
	}
	else
	{
		d_tcptimeout_us = timeout;
	}
	return true;
}


/*
 * Set UDP buffer size for receiving tracking data.
 */
bool DTrackSDK::setDataBufferSize( int bufSize )
{
	int newBufSize;
	if ( bufSize <= 0 )
	{
		newBufSize = DEFAULT_UDP_BUFSIZE;
	}
	else
	{
		newBufSize = bufSize;
	}

	if ( newBufSize != d_udpbufsize )
	{
		free( d_udpbuf );

		d_udpbufsize = newBufSize;
		d_udpbuf = (char *)malloc( d_udpbufsize );
	}
	return ( d_udpbuf != NULL );
}


/*
 * Receive and process one tracking data packet.
 */
bool DTrackSDK::receive()
{
	char* s;
	int len;
	
	lastDataError = ERR_NONE;
	lastServerError = ERR_NONE;

	if ( ! isDataInterfaceValid() )
	{
		lastDataError = ERR_NET;
		return false;
	}

	// defaults:
	startFrame();
	
	// receive UDP packet:
	len = d_udp->receive( d_udpbuf, d_udpbufsize - 1, d_udptimeout_us );
	if (len == -1) {
		lastDataError = ERR_TIMEOUT;
		return false;
	}
	
	if (len <= 0) {
		lastDataError = ERR_NET;
		return false;
	}
	
	s = d_udpbuf;
	s[len] = '\0';
	
	// process lines:
	lastDataError = ERR_PARSE;
	
	do {
		if (!parseLine(&s))
			return false;
	} while( (s = string_nextline(d_udpbuf, s, d_udpbufsize)) != 0 );
	
	endFrame();
	
	lastDataError = ERR_NONE;
	return true;
}


/*
 * Process one tracking packet manually.
 */
bool DTrackSDK::processPacket( const std::string& data )
{
	char* sBuf;
	char* s;
	
	lastDataError = ERR_NONE;
	lastServerError = ERR_NONE;
	
	// defaults:
	startFrame();
	
	if ( data.length() == 0 )
	{
		lastDataError = ERR_PARSE;
		return false;
	}

	sBuf = (char *)malloc( data.length() + 1 );
	if ( sBuf == NULL )  return false;
	
	//strcpy( sBuf, data.c_str() );
	strcpy_s( sBuf, data.length() + 1, data.c_str() );

	s = sBuf;
	
	// process lines:
	lastDataError = ERR_PARSE;
	
	do {
		if (!parseLine(&s))
			return false;

		s = string_nextline( sBuf, s, data.length() );
	} while( s != NULL );

	endFrame();
	
	lastDataError = ERR_NONE;
	return true;
}


/*
 * Get content of the UDP buffer.
 */
std::string DTrackSDK::getBuf() const
{
	if ( d_udpbuf == NULL )
		return std::string( "" );

	return std::string( d_udpbuf );
}


/*
 * Get last error at receiving tracking data (data transmission).
 */
DTrackSDK::Errors DTrackSDK::getLastDataError() const
{
	return lastDataError;
}


/*
 * Get last error at exchanging commands with Controller (command transmission).
 */
DTrackSDK::Errors DTrackSDK::getLastServerError() const
{
	return lastServerError;
}


/*
 * Get last DTrack2/DTrack3 command error code.
 */
int DTrackSDK::getLastDTrackError() const
{
	return lastDTrackError;
}


/*
 * Get last DTrack2/DTrack3 command error description.
 */
std::string DTrackSDK::getLastDTrackErrorDescription() const
{
	return lastDTrackErrorString;
}


/*
 * Set last DTrack2/DTrack3 command error.
 */
void DTrackSDK::setLastDTrackError(int newError, const std::string& newErrorString)
{
	lastDTrackError = newError;
	lastDTrackErrorString = newErrorString;
}


/*
 * Start measurement.
 */
bool DTrackSDK::startMeasurement()
{
	// Check for special DTrack handling
	if (rsType == SYS_DTRACK) {
		return (sendCommand("dtrack 10 3")) && (sendCommand("dtrack 31"));
	}
	
	// start tracking, 1 means answer "dtrack2 ok"
	return (1 == sendDTrack2Command("dtrack2 tracking start"));
}


/*
 * Stop measurement.
 */
bool DTrackSDK::stopMeasurement()
{
	// Check for special DTrack handling
	if (rsType == SYS_DTRACK) {
		return (sendCommand("dtrack 32")) && (sendCommand("dtrack 10 0"));
	}
	
	// stop tracking, 1 means answer "dtrack2 ok"
	return (1 == sendDTrack2Command("dtrack2 tracking stop"));
}


/*
 * Send DTrack1 command via UDP.
 */
bool DTrackSDK::sendCommand(const std::string& command)
{
	int err;

	if ( ! isDataInterfaceValid() )
		return false;

	lastDataError = ERR_NONE;
	// dest is dtrack2
	if (rsType == SYS_DTRACK_2)	{
		// command style is dtrack?
		if (0 == strncmp(command.c_str(), "dtrack ", 7)) {
			std::string c = command.substr(7);
			// start measurement
			if (0 == strncmp(c.c_str(), "10 3",4)) {
				return startMeasurement();
			}
			// stop measurement
			if (	(0 == strncmp(c.c_str(), "10 0",4))
			        ||	(0 == strncmp(c.c_str(), "10 1",4)))
			{
				return stopMeasurement();
			}
			// simulate success of other old commands
			return true;
		}
	}

	err = d_udp->send( (void* )command.c_str(), (int )command.length() + 1, d_remote_ip, d_remoteport, d_udptimeout_us );
	if ( err != 0 )
	{
		lastDataError = ERR_NET;
		return false;
	}
	if (strcmp(command.c_str(), "dtrack 10 3") == 0) {
#ifdef OS_UNIX
		sleep(1);     // some delay (actually only necessary for older DTrack versions...)
#endif
#ifdef OS_WIN
		Sleep(1000);  // some delay (actually only necessary for older DTrack versions...)
#endif
	}
	return true;
}


/*
 * Send DTrack2/DTrack3 command to DTrack and receive answer (TCP command interface).
 */
int DTrackSDK::sendDTrack2Command(const std::string& command, std::string* answer)
{
	// Params via TCP are not supported in DTrack
	if (rsType != SYS_DTRACK_2)
		return -2;
	
	// reset dtrack error
	setLastDTrackError();

	// command too long?
	if ( command.length() > DTRACK2_PROT_MAXLEN )
	{
		lastServerError = ERR_NET;
		return -3;
	}

	// connection invalid
	if (!isCommandInterfaceValid()) {
		lastServerError = ERR_NET;
		return -10;
	}
	
	// send TCP command string:
	if ( d_tcp->send( command.c_str(), static_cast< int >( command.length() ) + 1, d_tcptimeout_us ) != 0 )
	{
		lastServerError = ERR_NET;
		return -11;
	}
	
	// receive TCP response string:
	char ans[ DTRACK2_PROT_MAXLEN ];
	int err;
	err = d_tcp->receive( ans, DTRACK2_PROT_MAXLEN, d_tcptimeout_us );
	if ( err < 0 )
	{
		if (err == -1) {	// timeout
			lastServerError = ERR_TIMEOUT;
		}
		else
			if (err == -9) {	// broken connection
				delete d_tcp;
				d_tcp = NULL;
			}
			else
				lastServerError = ERR_NET;	// network error
		
		if (answer)
			*answer = "";
		
		return err;
	}
	
	// parse answer:
	
	// check for "dtrack2 ok" / no error
	if (0 == strcmp(ans, "dtrack2 ok"))
		return 1;
	
	// got error msg?
	if (0 == strncmp(ans, "dtrack2 err ", 12)) {
		char *s = ans + 12;
		int i;
		
		// parse error code
		if ( (s = string_get_i((char *)s, &i)) == 0 ) {
			setLastDTrackError(-1100, "SDK error -1100");
			lastServerError = ERR_PARSE;
			return -1100;
		}
		lastDTrackError = i;
		
		// parse error string
		if ( (s = string_get_quoted_text((char *)s, lastDTrackErrorString)) == 0 ) {
			setLastDTrackError(-1100, "SDK error -1100");
			lastServerError = ERR_PARSE;
			return -1101;
		}
		
		return 2;
	}
	
	// not 'dtrack2 ok'/'dtrack2 err ..' -> return msg
	if (answer)
		*answer = ans;
	
	lastServerError = ERR_NONE;
	return 0;
}


/*
 * Set DTrack2/DTrack3 parameter.
 */
bool DTrackSDK::setParam(const std::string& category, const std::string& name, const std::string& value)
{
	return setParam(category + " " + name + " " + value);
}


/*
 * Set DTrack2/DTrack3 parameter using a string containing parameter category, name and new value.
 */
bool DTrackSDK::setParam(const std::string& parameter)
{
	// send command, 1 means answer "dtrack2 ok"
	return (1 == sendDTrack2Command("dtrack2 set " + parameter));
}


/*
 * Get DTrack2/DTrack3 parameter.
 */
bool DTrackSDK::getParam(const std::string& category, const std::string& name, std::string& value)
{
	return getParam(category + " " + name, value);
}


/*
 * Get DTrack2/DTrack3 parameter using a string containing parameter category and name.
 */
bool DTrackSDK::getParam(const std::string& parameter, std::string& value)
{
	// Params via TCP are not supported in DTrack
	if (rsType != SYS_DTRACK_2)
		return false;
	
	std::string res;
	// expected answer is "dtrack2 set" -> return value 0
	if (0 != sendDTrack2Command("dtrack2 get " + parameter, &res))
		return false;
	
	// parse parameter from answer
	if (0 == strncmp(res.c_str(), "dtrack2 set ", 12)) {
		char *str = strdup(res.c_str() + 12);
		char *s = str;
		if ( (s = string_cmp_parameter(s, parameter.c_str())) == 0 ) {
			free(str);
			lastServerError = ERR_PARSE;
			return false;
		}
		
		// assign result
		value = s;
		free(str);
		return true;
	}
	
	return false;
}


/*
 * Get DTrack2/DTrack3 event message from the Controller.
 */
bool DTrackSDK::getMessage()
{
	// Messages via TCP are not supported in DTrack
	if (rsType != SYS_DTRACK_2)
		return false;
	
	// send request
	std::string res;
	if (0 != sendDTrack2Command("dtrack2 getmsg", &res))
		return false;
	
	// check answer
	if (0 != strncmp(res.c_str(), "dtrack2 msg ", 12))
		return false;
	
	// reset values
	d_message_origin = d_message_msg = d_message_status = "";
	d_message_framenr = d_message_errorid = 0;
	
	// parse message
	const char* s = res.c_str() + 12;
	// get 'origin'
	if ( (s = string_get_word((char *)s, d_message_origin)) == 0 )
		return false;
	
	// get 'status'
	if ( (s = string_get_word((char *)s, d_message_status)) == 0 )
		return false;
	
	unsigned int ui;
	// get 'frame counter'
	if ( (s = string_get_ui((char *)s, &ui)) == 0 )
		return false;
	d_message_framenr = ui;
	
	// get 'error id'
	if ( (s = string_get_ui((char *)s, &ui)) == 0 )
		return false;
	d_message_errorid = ui;
	
	// get 'message'
	if ( (s = string_get_quoted_text((char *)s, d_message_msg)) == 0 )
		return false;
	
	return true;
}


/*
 * Get frame counter of last DTrack2/DTrack3 event message.
 */
unsigned int DTrackSDK::getMessageFrameNr() const
{
	return d_message_framenr;
}


/*
 * Get error id of last DTrack2/DTrack3 event message.
 */
unsigned int DTrackSDK::getMessageErrorId() const
{
	return d_message_errorid;
}


/*
 * Get origin of last DTrack2/DTrack3 event message.
 */
std::string DTrackSDK::getMessageOrigin() const
{
	return d_message_origin;
}


/*
 * Get status of last DTrack2/DTrack3 event message.
 */
std::string DTrackSDK::getMessageStatus() const
{
	return d_message_status;
}


/*
 * Get message text of last DTrack2/DTrack3 event message.
 */
std::string DTrackSDK::getMessageMsg() const
{
	return d_message_msg;
}


/*
 * Send tactile command to set feedback on a specific finger of a specific hand.
 */
bool DTrackSDK::tactileFinger( int handId, int fingerId, double strength )
{
	int err;

	if ( ! isCommandInterfaceValid() )
		return false;

	setLastDTrackError();

	if ( strength > 1.0 || strength < 0.0 )
	{
		lastServerError = ERR_NET;
		return false;
	}

	std::ostringstream os;
	os << "tfb 1 [" << handId << " " << fingerId << " 1.0 " << strength << "]";

	err = d_udp->send( (void* )os.str().c_str(), (int )os.str().length() + 1, d_remote_ip, DTRACK2_PORT_TACTILE, d_udptimeout_us );
	if ( err != 0 )
	{
		lastDataError = ERR_NET;
		return false;
	}

	return true;
}


/*
 * Send tactile command to set tactile feedback on all fingers of a specific hand.
 */
bool DTrackSDK::tactileHand( int handId, const std::vector< double >& strength )
{
	int err;

	if ( ! isCommandInterfaceValid() )
		return false;

	setLastDTrackError();

	std::ostringstream os;
	os << "tfb " << strength.size() << " ";

	for ( size_t i = 0; i < strength.size(); i++ )
	{
		if ( strength[ i ] > 1.0 || strength[ i ] < 0.0 )
		{
			lastServerError = ERR_NET;
			return false;
		}

		os << "[" << handId << " " << i << " 1.0 " << strength[ i ] << "]";
	}

	err = d_udp->send( (void* )os.str().c_str(), (int )os.str().length() + 1, d_remote_ip, DTRACK2_PORT_TACTILE, d_udptimeout_us );
	if ( err != 0 )
	{
		lastDataError = ERR_NET;
		return false;
	}

	return true;
}


/*
 * Send tactile command to turn off tactile feedback on all fingers of a specific hand.
 */
bool DTrackSDK::tactileHandOff( int handId, int numFinger )
{
	std::vector< double > strength( numFinger, 0.0 );

	return tactileHand( handId, strength );
}

