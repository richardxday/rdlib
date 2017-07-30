
#ifndef __STANDARD_MEMORY_FILE__
#define __STANDARD_MEMORY_FILE__

#include "misc.h"
#include "StdData.h"

class AString;
class AStdMemFile : public AStdData {
public:
	AStdMemFile();
	virtual ~AStdMemFile();

	virtual bool    open(const char *mode = "w", const void *data = NULL, size_t length = 0);
	virtual sint_t  close();
	
	virtual bool    isopen() const {return (pData != NULL);}

	virtual slong_t tell()   {return isopen() ? (slong_t)Pos : -1;}
	virtual sint_t  seek(slong_t offset, uint_t origin = SEEK_SET);
	virtual sint_t  flush()  {return isopen() ? 0 : -1;}
	virtual void    rewind() {if (isopen()) Pos = 0;}

	virtual sint_t  eof()    {return isopen() ? (Pos >= Length) : 0;}

	bool			AllocData(size_t bytes = 0);

	void			AddBytes(size_t bytes);
	void			RemoveBytes(size_t bytes);
	const uint8_t   *GetData()  	   const {return pData;}
	uint8_t         *GetData()  	         {return pData;}
	const uint8_t   *GetCurrentData()  const {return pData + Pos;}
	uint8_t         *GetCurrentData()        {return pData + Pos;}
	size_t          GetPos()    	   const {return Pos;}
	size_t		    GetLength() 	   const {return Length;}
	AString 		Base64Encode() const;
	bool    		Base64Decode(const AString& str);

	operator AString() const;
	
	NODETYPE(AStdMemFile, AStdData);

protected:
	virtual slong_t readdata(void *buf, size_t bytes);
	virtual slong_t writedata(const void *buf, size_t bytes);

protected:
	uint8_t *pData;
	size_t  Pos, Length, AllocatedLength, SizeInc;
	bool    bAllocated;

	static uint32_t DefaultSizeInc;

	NODETYPE_DEFINE(AStdMemFile);
};

#endif
