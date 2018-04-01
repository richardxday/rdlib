
#ifndef __RECURSE__
#define __RECURSE__

#include "misc.h"
#include "strsup.h"
#include "DataList.h"
#include "DateTime.h"
#include "QuitHandler.h"

enum {
	FILE_FLAG_EXECUTABLE = 0x1,
	FILE_FLAG_WRITABLE   = 0x2,
	FILE_FLAG_READABLE   = 0x4,
	FILE_FLAG_NORMAL     = FILE_FLAG_EXECUTABLE | FILE_FLAG_WRITABLE | FILE_FLAG_READABLE,
	FILE_FLAG_READ_ONLY  = FILE_FLAG_EXECUTABLE | FILE_FLAG_READABLE,
	FILE_FLAG_HIDDEN     = 0x2000,
	FILE_FLAG_SYSTEM     = 0x4000,
	FILE_FLAG_IS_DIR     = 0x8000,
};

typedef struct {
	AString   FileName;
	AString   ShortName;
	uint64_t  FileSize;
	ADateTime CreateTime;
	ADateTime AccessTime;
	ADateTime WriteTime;
	uint16_t  Attrib;
} FILE_INFO;

#define RECURSE_ALL_SUBDIRS (MAX_UNSIGNED(uint_t))
#define RECURSE_SUBDIRS(b) ((b) ? RECURSE_ALL_SUBDIRS : 0)

extern bool Recurse(const AString& PathPattern, uint_t nSubDirs, bool (*fn)(const FILE_INFO *file, void *Context), void *Context = NULL);
extern bool Recurse(const AString& Path, const AString& Pattern, uint_t nSubDirs, bool (*fn)(const FILE_INFO *file, void *Context), void *Context = NULL);

extern bool GetFileInfo(const AString& Filename, FILE_INFO *file);

extern int CompareWriteTimes(const AString& Filename1, const AString& Filename2);
extern bool FileNewerThan(const AString& Filename1, const AString& Filename2);
extern bool FileOlderThan(const AString& Filename1, const AString& Filename2);

extern bool CollectFiles(const AString& Path, const AString& Pattern, uint_t nSubdirs, AList& list, uint16_t attribMask = FILE_FLAG_IS_DIR, uint16_t attribCmp = 0, AQuitHandler *pQuitHandler = NULL);
extern bool TraverseFiles(const AList& list, bool (*fn)(const FILE_INFO *file, void *context), void *context = NULL);

#ifdef CreateDirectory
#undef CreateDirectory
#endif

extern bool CreateDirectory(const AString& dir);
extern bool ChangeDirectory(const AString& dir);
extern AString GetCurrentDirectory();

class AFileNode : public AListNode {
public:
	AFileNode(const AString& filename);
	AFileNode(const FILE_INFO& file);
	virtual ~AFileNode();

	LIST_FUNCTIONS(AFileNode);

	enum {
		SORT_FLAG_REVERSE = 0x1000,
		SORT_FLAG_MIXED   = 0x2000,
		SORT_FLAG_MASK	  = 0xf000,
	};

	const AString& 	 GetFilename()  const {return FileData.FileName;}
	const AString& 	 GetShortName() const {return FileData.ShortName;}
	uint64_t		   	 GetFileSize()  const {return FileData.FileSize;}
	const ADateTime& GetFileDate()  const {return FileData.WriteTime;}

	bool IsDir() const {return ((FileData.Attrib & FILE_FLAG_IS_DIR) != 0);}

	enum {
		SORT_BY_NAME = 0,
		SORT_BY_PATH,
		SORT_BY_EXT,
		SORT_BY_DATE,
		SORT_BY_SIZE,

		SORT_BY_ITEMS,
		SORT_BY_MASK = 0xf,
	};

	static sint_t CompareByName(const AListNode *pNode1, const AListNode *pNode2, void *context);
	static sint_t CompareByPath(const AListNode *pNode1, const AListNode *pNode2, void *context);
	static sint_t CompareByExt(const AListNode *pNode1, const AListNode *pNode2, void *context);
	static sint_t CompareByDate(const AListNode *pNode1, const AListNode *pNode2, void *context);
	static sint_t CompareBySize(const AListNode *pNode1, const AListNode *pNode2, void *context);

	static COMPARE_FUNC GetSortFunction(uint_t n);

protected:
	FILE_INFO FileData;

	NODETYPE_DEFINE(AFileNode);
};

extern bool CollectFilesEx(const AString& Path, const AString& Pattern, uint_t nSubdirs, AList& list, uint_t sortFlags = 0, uint16_t attribSet = 0, uint16_t attribClr = 0, AQuitHandler *pQuitHandler = NULL);
extern bool TraverseFiles(const AList& list, bool (*fn)(const AFileNode& file, void *context), void *context = NULL);

#endif
