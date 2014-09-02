#ifndef __STANDARD_SERIAL__
#define __STANDARD_SERIAL__

#include "misc.h"
#include "StdData.h"
#include "ListNode.h"

/*--------------------------------------------------------------------------------*/
/** AStdSerial - an AStdData interface to serial comms
 *
 *  See StdData.h for details of interface
 */
/*--------------------------------------------------------------------------------*/
class AStdSerial : public AStdData {
public:
	AStdSerial();
	virtual ~AStdSerial();

	/*--------------------------------------------------------------------------------*/
	/** serial comms data formats in the form DataMode_<bits><parity><stop-bits>
	 */
	/*--------------------------------------------------------------------------------*/
	enum {
		DataMode_8N1 = 0,
		DataMode_8E1,
		DataMode_8O1,

		DataMode_8N2,
		DataMode_8E2,
		DataMode_8O2,
	};

	/*--------------------------------------------------------------------------------*/
	/** Open serial port
	 *
	 * @param name name of serial port (see below)
	 * @param baudrate serial baudrate in bps
	 * @param flow hardware flow control enable
	 * @param datamode serial data mode (see above)
	 *
	 * @return true if port open successful
	 *
	 * @note for win32 systems, name should be 'COM<port>'
	 * @note for linux/BSD systems, name should be '/dev/tty<port>'
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool open(const char *name, uint_t baudrate = 57600, bool flow = false, uint_t datamode = DataMode_8N1);

	/*--------------------------------------------------------------------------------*/
	/** Close serial port
	 */
	/*--------------------------------------------------------------------------------*/
	virtual sint_t close();
	
	virtual bool isopen() const;

	/*--------------------------------------------------------------------------------*/
	/** Return number of bytes in serial input buffer
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t bytesavailable();

	/*--------------------------------------------------------------------------------*/
	/** Return number of bytes in serial output buffer
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t bytesqueued();

	/*--------------------------------------------------------------------------------*/
	/** Flush (delete) data in serial input/output buffers
	 */
	/*--------------------------------------------------------------------------------*/
	virtual sint_t  flush();

	NODETYPE(AStdSerial, AStdData);

	/*--------------------------------------------------------------------------------*/
	/** Create a list of USB serial ports in Linux/BSD systems
	 *
	 * @param list list to receive list of port names that can be used in open()
	 * @param maxports maximum number of ports to search for (Windows only)
	 *
	 * @return number of serial ports found
	 *
	 * @note relies on the naming convention of USB to serial port convertors being ttyUSB* (Linux) or tty.usbserial* (BSD)
	 */
	/*--------------------------------------------------------------------------------*/
	static uint_t findusbserialports(AList& list, uint_t maxports = 256);

protected:
	/*--------------------------------------------------------------------------------*/
	/** The following functions are documented in StdData.h
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t readdata(void *buf, size_t bytes);
	virtual slong_t writedata(const void *buf, size_t bytes);

protected:
#if defined(__LINUX__) || defined(__CYGWIN__)
	int fd;
#endif
#ifdef _WIN32
	HANDLE file;
#endif

	NODETYPE_DEFINE(AStdSerial);
};

#endif
