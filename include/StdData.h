
#ifndef __STANDARD_DATA__
#define __STANDARD_DATA__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <fcntl.h>
#include "misc.h"
#include "NodeType.h"

class AStdData;
extern AStdData *Stdin;
extern AStdData *Stdout;
extern AStdData *Stderr;
extern AStdData *DbgOut;
extern AStdData *DbgErr;

#ifdef putc
#undef putc
#endif
#ifdef getc
#undef getc
#endif
#ifdef feof
#define NEED_FEOF
#undef feof
#endif
#ifdef ferror
#define NEED_FERROR
#undef ferror
#endif

/*--------------------------------------------------------------------------------*/
/** Base class for all AStdxxx classes
 *
 *  Provides FILE * type handling for various objects
 *  Based on a process of open, read/write data and close
 */
/*--------------------------------------------------------------------------------*/
class AStdData : public ANodeType {
public:
	AStdData();
	virtual ~AStdData();

	enum {
		SWAP_NEVER = 0,			// never swap bytes
		SWAP_ALWAYS,			// always swap bytes
		SWAP_FOR_LE,			// swap such that bytes make sense on little-endian architectures
		SWAP_FOR_BE,			// swap such that bytes make sense on big-endian architectures

		SWAP_ITEMS,
	};

	/*--------------------------------------------------------------------------------*/
	/** close()
	 *
	 * @return 0 if successful, -1 otherwise
	 */
	/*--------------------------------------------------------------------------------*/
	virtual sint_t  close() {return -1;}

	/*--------------------------------------------------------------------------------*/
	/** Returns whether this object is 'open'
	 */
	/*--------------------------------------------------------------------------------*/
	virtual bool    isopen() const {return false;}

	/*--------------------------------------------------------------------------------*/
	/** Return number of bytes that can be read/are available in the input buffer
	 *
	 * @return number of bytes or -1 for an error
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t bytesavailable() {return isopen() ? MAX_SIGNED(slong_t) : -1;}

	/*--------------------------------------------------------------------------------*/
	/** Return number of bytes that are waiting to be written/are in the output buffer
	 *
	 * @return number of bytes or -1 for an error
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t bytesqueued()    {return isopen() ? 0 : -1;}

	/*--------------------------------------------------------------------------------*/
	/** Read data (life fread())
	 *
	 * @param buf destination buffer
	 * @param size item size
	 * @param count item count
	 * @param swap how to swap bytes (within each item)
	 *
	 * @return number of items read or <0 for error/end of file
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t read(void *buf, size_t size, size_t count, uint_t swap = SWAP_NEVER);

	/*--------------------------------------------------------------------------------*/
	/** Write data (life fwrite())
	 *
	 * @param buf source buffer
	 * @param size item size
	 * @param count item count
	 * @param swap how to swap bytes (within each item)
	 *
	 * @return number of items written or <0 for error
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t write(const void *buf, size_t size, size_t count, uint_t swap = SWAP_NEVER);

	/*--------------------------------------------------------------------------------*/
	/** Read data (life fread()) into a little-endian buffer
	 *
	 * @param buf destination buffer
	 * @param size item size
	 * @param count item count
	 *
	 * @return number of items read or <0 for error/end of file
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t readLE(void *buf, size_t size, size_t count) {return read(buf, size, count, SWAP_FOR_LE);}

	/*--------------------------------------------------------------------------------*/
	/** Read data (life fread()) into a big-endian buffer
	 *
	 * @param buf destination buffer
	 * @param size item size
	 * @param count item count
	 *
	 * @return number of items read or <0 for error/end of file
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t readBE(void *buf, size_t size, size_t count) {return read(buf, size, count, SWAP_FOR_BE);}

	/*--------------------------------------------------------------------------------*/
	/** Write data (life fwrite()) from little-endian buffer
	 *
	 * @param buf source buffer
	 * @param size item size
	 * @param count item count
	 *
	 * @return number of items written or <0 for error
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t writeLE(const void *buf, size_t size, size_t count) {return write(buf, size, count, SWAP_FOR_LE);}

	/*--------------------------------------------------------------------------------*/
	/** Write data (life fwrite()) from big-endian buffer
	 *
	 * @param buf source buffer
	 * @param size item size
	 * @param count item count
	 *
	 * @return number of items written or <0 for error
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t writeBE(const void *buf, size_t size, size_t count) {return write(buf, size, count, SWAP_FOR_BE);}

	/*--------------------------------------------------------------------------------*/
	/** Read raw bytes
	 *
	 * @param buf destination buffer
	 * @param bytes number of bytes to read
	 * @param swap how to swap bytes (buffer is assumed to be a single 'item')
	 *
	 * @return number of bytes read or <0 for error/end of file
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t readbytes(void *buf, size_t bytes, uint_t swap = SWAP_NEVER) {return read(buf, 1, bytes, swap);}

	/*--------------------------------------------------------------------------------*/
	/** Write raw bytes
	 *
	 * @param buf source buffer
	 * @param bytes number of bytes to read
	 * @param swap how to swap bytes (buffer is assumed to be a single 'item')
	 *
	 * @return number of bytes written or <0 for error
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t writebytes(const void *buf, size_t bytes, uint_t swap = SWAP_NEVER) {return write(buf, 1, bytes, swap);}

	/*--------------------------------------------------------------------------------*/
	/** Read raw bytes into little-endian buffer
	 *
	 * @param buf destination buffer
	 * @param bytes number of bytes to read
	 *
	 * @return number of bytes read or <0 for error/end of file
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t readbytesLE(void *buf, size_t bytes) {return read(buf, 1, bytes, SWAP_FOR_LE);}

	/*--------------------------------------------------------------------------------*/
	/** Read raw bytes into big-endian buffer
	 *
	 * @param buf destination buffer
	 * @param bytes number of bytes to read
	 *
	 * @return number of bytes read or <0 for error/end of file
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t readbytesBE(void *buf, size_t bytes) {return read(buf, 1, bytes, SWAP_FOR_BE);}

	/*--------------------------------------------------------------------------------*/
	/** Write raw bytes from little-endian buffer
	 *
	 * @param buf source buffer
	 * @param bytes number of bytes to read
	 * @param swap how to swap bytes (buffer is assumed to be a single 'item')
	 *
	 * @return number of bytes written or <0 for error
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t writebytesLE(const void *buf, size_t bytes) {return write(buf, 1, bytes, SWAP_FOR_LE);}

	/*--------------------------------------------------------------------------------*/
	/** Write raw bytes from big-endian buffer
	 *
	 * @param buf source buffer
	 * @param bytes number of bytes to read
	 * @param swap how to swap bytes (buffer is assumed to be a single 'item')
	 *
	 * @return number of bytes written or <0 for error
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t writebytesBE(const void *buf, size_t bytes) {return write(buf, 1, bytes, SWAP_FOR_BE);}

	/*--------------------------------------------------------------------------------*/
	/** Specific type handling - read and write different types, all with swap support
	 *
	 * @param val datatype to read/write
	 * @param swap how to swap bytes
	 *
	 * @return either true/false for success/failure or number of items read/written or -1 for error
	 */
	/*--------------------------------------------------------------------------------*/
#define TYPED_ITEMS_HANDLING(type)										\
	virtual bool   	readitem(type& val, uint_t swap = SWAP_NEVER) 		  		  	   	{return (read(&val,  sizeof(val),  1, swap)        == 1);} \
	virtual bool   	readitemLE(type& val) 					   		  		  		   	{return (read(&val,  sizeof(val),  1, SWAP_FOR_LE) == 1);} \
	virtual bool   	readitemBE(type& val) 					   		  		  		   	{return (read(&val,  sizeof(val),  1, SWAP_FOR_BE) == 1);} \
	virtual bool   	writeitem(const type& val, uint_t swap = SWAP_NEVER) 		  	   	{return (write(&val, sizeof(val),  1, swap)        == 1);} \
	virtual bool   	writeitemLE(const type& val) 					  		  		   	{return (write(&val, sizeof(val),  1, SWAP_FOR_LE) == 1);} \
	virtual bool   	writeitemBE(const type& val) 					  		  		   	{return (write(&val, sizeof(val),  1, SWAP_FOR_BE) == 1);} \
	virtual slong_t readitems(type *buf, size_t n = 1, uint_t swap = SWAP_NEVER) 	   	{return read(buf,    sizeof(*buf), n, swap);} \
	virtual slong_t readitemsLE(type *buf, size_t n = 1) 					  		   	{return read(buf,    sizeof(*buf), n, SWAP_FOR_LE);} \
	virtual slong_t readitemsBE(type *buf, size_t n = 1) 					  		   	{return read(buf,    sizeof(*buf), n, SWAP_FOR_BE);} \
	virtual slong_t writeitems(const type *buf, size_t n = 1, uint_t swap = SWAP_NEVER) {return write(buf,   sizeof(*buf), n, swap);} \
	virtual slong_t writeitemsLE(const type *buf, size_t n = 1) 						{return write(buf,   sizeof(*buf), n, SWAP_FOR_LE);} \
	virtual slong_t writeitemsBE(const type *buf, size_t n = 1) 						{return write(buf,   sizeof(*buf), n, SWAP_FOR_BE);}

	TYPED_ITEMS_HANDLING(sint8_t);
	TYPED_ITEMS_HANDLING(uint8_t);
	TYPED_ITEMS_HANDLING(sint16_t);
	TYPED_ITEMS_HANDLING(uint16_t);
	TYPED_ITEMS_HANDLING(sint32_t);
	TYPED_ITEMS_HANDLING(uint32_t);
	TYPED_ITEMS_HANDLING(sint64_t);
	TYPED_ITEMS_HANDLING(uint64_t);
	TYPED_ITEMS_HANDLING(float);
	TYPED_ITEMS_HANDLING(double);
#undef TYPED_ITEMS_HANDLING

	/*--------------------------------------------------------------------------------*/
	/** printf - like printf but writes data to the stream
	 */
	/*--------------------------------------------------------------------------------*/
	virtual sint_t  printf(const char *format, ...) PRINTF_FORMAT_METHOD;

	/*--------------------------------------------------------------------------------*/
	/** vprintf - like vprintf but writes data to the stream
	 */
	/*--------------------------------------------------------------------------------*/
	virtual sint_t  vprintf(const char *format, va_list ap);

	/*--------------------------------------------------------------------------------*/
	/** file functions, replicating fxxx() functions
	 */
	/*--------------------------------------------------------------------------------*/
	virtual sint_t  putc(sint_t c) {char c1 = (char)c; return (writedata(&c1, sizeof(c1)) == sizeof(c1)) ? c : EOF;}
	virtual sint_t  getc()         {char c;            return (readdata(&c,   sizeof(c))  == sizeof(c))  ? c : EOF;}

	virtual sint_t  puts(const char *str) {return writedata(str, strlen(str));}
	virtual char    *gets(char *str, sint_t n);

	virtual slong_t tell() {return -1;}
	virtual sint_t  seek(slong_t offset, uint_t origin = SEEK_SET);
	virtual sint_t  flush() {return -1;}
	virtual void    rewind() {}

	virtual sint_t  eof()   {return 0;}
	virtual sint_t  error() {return 0;}

	virtual sint_t  setvbuf(char *buffer, int mode, size_t size) {UNUSED(buffer); UNUSED(mode); UNUSED(size); return -1;}

	virtual sint_t  setbinarymode(bool binary = true) {UNUSED(binary); return -1;}

	sint_t Put(sint_t c) {return putc(c);}
	sint_t Get() {return getc();}
	sint_t Flush() {return flush();}
	
	NODETYPE(AStdData, ANodeType);

protected:
	/*--------------------------------------------------------------------------------*/
	/** Actually read data - this is the central workhorse function, all other read functions MUST call this!
	 *
	 * @param buf destination buffer
	 * @param bytes number of bytes to read
	 *
	 * @return number of bytes read or -1 for error/end of file
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t readdata(void *buf, size_t bytes) {UNUSED(buf); UNUSED(bytes); return -1;}

	/*--------------------------------------------------------------------------------*/
	/** Actually write data - this is the central workhorse function, all other write functions MUST call this!
	 *
	 * @param buf source buffer
	 * @param bytes number of bytes to write
	 *
	 * @return number of bytes written or -1 for error
	 */
	/*--------------------------------------------------------------------------------*/
	virtual slong_t writedata(const void *buf, size_t bytes) {UNUSED(buf); UNUSED(bytes); return -1;}

protected:
	static bool SwapNeeded[SWAP_ITEMS];

	NODETYPE_DEFINE(AStdData);
};

/*--------------------------------------------------------------------------------*/
/** Simple stackable StdData ptr swapper (useful for remapping Stdin/Stdout)
 */
/*--------------------------------------------------------------------------------*/
class AStdDataSelector {
public:
	AStdDataSelector(AStdData **ptr, AStdData *newptr) : Ptr(ptr),
														 Original(ptr[0]) {
		Ptr[0] = newptr;
	}
	~AStdDataSelector() {Ptr[0] = Original;}

protected:
	AStdData **Ptr;
	AStdData *Original;
};

#endif
