
#include <stdio.h>

#include "StdMemFile.h"
#include "strsup.h"

/* end of includes */

NODETYPE_IMPLEMENT(AStdMemFile);

uint32_t AStdMemFile::DefaultSizeInc = 256;

AStdMemFile::AStdMemFile() : AStdData(),
							 pData(NULL),
							 Pos(0),
							 Length(0),
							 AllocatedLength(0),
							 SizeInc(0),
							 bAllocated(false)
{
}

AStdMemFile::~AStdMemFile()
{
	close();
}

bool AStdMemFile::open(const char *mode, const void *data, size_t length)
{
	bool ok = false;
	if (strchr(mode, 'r')) {
		if (data && length) {
			pData   = (uint8_t *)data;
			Pos     = 0;
			Length  = length;
			SizeInc = 0;
			ok      = true;
		}
	}
	else if (strchr(mode, 'w') || strchr(mode, 'a')) {
		SizeInc = DefaultSizeInc;

		Pos = Length = 0;

		if ((pData = (uint8_t *)Allocate(pData, length, AllocatedLength, 1, SizeInc)) != NULL) {
			if (data && length) {
				memcpy(pData, data, length);
				Length = length;
			}
			Pos    	   = strchr(mode, 'a') ? Length : 0;
			bAllocated = true;
			ok         = true;
		}
	}

	return ok;
}

sint_t AStdMemFile::close()
{
	if (isopen()) {
		if (bAllocated) delete[] pData;
		pData = NULL;
	}
	Pos = Length = AllocatedLength = SizeInc = 0;
	bAllocated = false;

	return 0;
}

sint_t AStdMemFile::seek(slong_t offset, uint_t origin)
{
	sint_t res = -1;
	if (isopen()) {
		sint64_t pos = Pos;
		switch (origin) {
			case SEEK_SET: pos =                    offset; break;
			case SEEK_CUR: pos = (sint64_t)Pos    + offset; break;
			case SEEK_END: pos = (sint64_t)Length + offset; break;
			default: assert(false); break;
		}
		if (RANGE(pos, 0, (sint64_t)Length)) {
			Pos = (size_t)pos;
			res = 0;
		}
	}
	return res;
}

slong_t AStdMemFile::readdata(void *buf, size_t bytes)
{
	slong_t bytes1 = -1;

	if (isopen()) {
		bytes = MIN(bytes, Length - Pos);
		memcpy(buf, pData + Pos, bytes);
		Pos   += bytes;
		bytes1 = bytes;
	}

	return bytes1;
}

slong_t AStdMemFile::writedata(const void *buf, size_t bytes)
{
	slong_t bytes1 = -1;

	if (AllocData(bytes)) {
		bytes1 = MIN(bytes, AllocatedLength - Pos);
			
		memcpy(pData + Pos, buf, bytes1);
		
		Pos   += bytes1;
		Length = MAX(Length, Pos);
	}

	return bytes1;
}

bool AStdMemFile::AllocData(size_t bytes)
{
	bool success = false;

	if (isopen() && SizeInc) {
		success = ((pData = (uint8_t *)Allocate(pData, Pos + bytes, AllocatedLength, 1, SizeInc)) != NULL);
	}

	return success;
}

void AStdMemFile::AddBytes(size_t bytes)
{
	bytes   = MIN(bytes, AllocatedLength - Pos);
	Pos    += bytes;
	Length  = MAX(Length, Pos);
}

void AStdMemFile::RemoveBytes(size_t bytes)
{
	bytes   = MIN(bytes, Length);
	Pos     = SUBZ(Pos, bytes);
	Length -= bytes;
	if (Length) memmove(pData, pData + bytes, Length);
}

AStdMemFile::operator AString() const
{
	return AString((const char *)pData, Length);
}

AString AStdMemFile::Base64Encode() const
{
	return pData ? ::Base64Encode(pData, Length) : "";
}

bool AStdMemFile::Base64Decode(const AString& str)
{
	sint_t nbytes;
	bool   success = false;

	Pos = Length = 0;

	if ((nbytes = str.Base64DecodeLength()) > 0) {
		if (AllocData(nbytes)) {
			if ((nbytes = str.Base64Decode(pData, AllocatedLength)) > 0) {
				Length  = Pos = nbytes;
				success = true;
			}
		}
	}

	return success;
}
