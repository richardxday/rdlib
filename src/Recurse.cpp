
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef __LINUX__
// Linux/Cygwin
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(ANDROID) || defined(__IPHONEOS__)
#include <dirent.h>
#endif

#if defined(__CYGWIN__) || defined(__IPHONEOS__)
extern "C" {
#include <sys/dirent.h>
};
#elif !defined(ANDROID)
#include <sys/dir.h>
#endif
#endif

#ifdef _WIN32
// Windows
#include <windows.h>
#endif

#include "Recurse.h"
#include "Regex.h"

/* end of includes */

bool Recurse(const AString& PathPattern, uint_t nSubDirs, bool (*fn)(const FILE_FIND *file, void *Context), void *Context)
{
	return ::Recurse(PathPattern.PathPart(), PathPattern.FilePart(), nSubDirs, fn, Context);
}

#ifdef __LINUX__
static bool SetFileData(const AString& path, const struct dirent& finddata, FILE_FIND *file)
{
	struct stat statdata;
	bool success = false;

	file->FileName  = path.CatPath(finddata.d_name);
	file->ShortName = file->FileName.FilePart();
	file->Attrib    = 0;

	if (stat(file->FileName, &statdata) == 0) {
		file->FileSize = statdata.st_size;
		file->CreateTime.fromtime(statdata.st_mtime);
		file->AccessTime.fromtime(statdata.st_atime);
		file->WriteTime.fromtime(statdata.st_mtime);
		
		if (statdata.st_mode & S_IRUSR) file->Attrib |= FILE_FLAG_READABLE;
		if (statdata.st_mode & S_IWUSR) file->Attrib |= FILE_FLAG_WRITABLE;
		if (statdata.st_mode & S_IXUSR) file->Attrib |= FILE_FLAG_EXECUTABLE;
		
		if (statdata.st_mode & S_IRGRP) file->Attrib |= FILE_FLAG_READABLE << 3;
		if (statdata.st_mode & S_IWGRP) file->Attrib |= FILE_FLAG_WRITABLE << 3;
		if (statdata.st_mode & S_IXGRP) file->Attrib |= FILE_FLAG_EXECUTABLE << 3;
		
		if (statdata.st_mode & S_IROTH) file->Attrib |= FILE_FLAG_READABLE << 6;
		if (statdata.st_mode & S_IWOTH) file->Attrib |= FILE_FLAG_WRITABLE << 6;
		if (statdata.st_mode & S_IXOTH) file->Attrib |= FILE_FLAG_EXECUTABLE << 6;
		
		if (S_ISDIR(statdata.st_mode))  file->Attrib |= FILE_FLAG_IS_DIR;

		success = true;
	}

	return success;
}

bool Recurse(const AString& Path, const AString& Pattern, uint_t nSubDirs, bool (*fn)(const FILE_FIND *file, void *Context), void *Context)
{
	AString   pattern = ParsePathRegex(Pattern);
	FILE_FIND file;
	DIR		  *handle;
	bool      ok = true, any = IsRegexAnyPattern(pattern);

	if ((handle = opendir(Path.Valid() ? Path : ".")) != NULL) {
		struct dirent *ent;

		while (ok && ((ent = readdir(handle)) != NULL)) {
			if (SetFileData(Path, *ent, &file)) {
				if ((file.ShortName != ".") && (file.ShortName != "..")) {
					bool done = false;

					if (any || MatchPathRegex(file.ShortName, pattern)) {
						ok = (*fn)(&file, Context);
						if (!ok) break;
						done = true;
					}
					if ((nSubDirs > 0) && (file.Attrib & FILE_FLAG_IS_DIR)) {
						if (!done) {
							ok = (*fn)(&file, Context);
							if (!ok) break;
						}
						
						ok = Recurse(file.FileName, Pattern, nSubDirs - 1, fn, Context);
						if (!ok) break;
					}
				}
			}
		}

		closedir(handle);
	}

	return ok;
}

bool GetFileDetails(const AString& FileName, FILE_FIND *file)
{
	struct dirent ent;

	strcpy(ent.d_name, FileName.FilePart());

	return SetFileData(FileName.PathPart(), ent, file);
}
#else

static uint64_t ConvertTime(const FILETIME& ft)
{
	FILETIME ft1;
	uint64_t t;

	::FileTimeToLocalFileTime(&ft, &ft1);
	t  = ((((uint64_t)ft1.dwHighDateTime) << 32) | ft1.dwLowDateTime) / 10000;
	t -= (uint64_t)ADateTime::DaysSince1601 * (uint64_t)MS_PER_DAY;

	return t;
}

static void SetFileData(const AString& path, const WIN32_FIND_DATA& finddata, FILE_FIND *file)
{
	file->FileName   = path.CatPath(finddata.cFileName);
	file->ShortName  = file->FileName.FilePart();
	file->FileSize   = (uint64_t)finddata.nFileSizeLow + ((uint64_t)finddata.nFileSizeHigh << 32);
	file->CreateTime = ConvertTime(finddata.ftCreationTime);
	file->AccessTime = ConvertTime(finddata.ftLastAccessTime);
	file->WriteTime  = ConvertTime(finddata.ftLastWriteTime);
	file->Attrib     = FILE_FLAG_NORMAL;
	
	if (finddata.dwFileAttributes & FILE_ATTRIBUTE_READONLY)  file->Attrib &= ~FILE_FLAG_WRITABLE;
	
	file->Attrib |= (file->Attrib << 3);
	file->Attrib |= (file->Attrib << 3);
	
	if (finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)    file->Attrib |= FILE_FLAG_HIDDEN;
	if (finddata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)    file->Attrib |= FILE_FLAG_SYSTEM;
	if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) file->Attrib |= FILE_FLAG_IS_DIR;
}

bool Recurse(const AString& Path, const AString& Pattern, uint_t nSubDirs, bool (*fn)(const FILE_FIND *file, void *Context), void *Context)
{
	WIN32_FIND_DATA finddata;
	AString   pattern, pattern1 = ParsePathRegex(Pattern);
	FILE_FIND file;
	HANDLE    handle;
	bool      ok = true, any = IsRegexAnyPattern(pattern1);

	pattern = Path.CatPath("*");
	if ((handle = ::FindFirstFile(pattern, &finddata)) != INVALID_HANDLE_VALUE) {
		do {
			SetFileData(Path, finddata, &file);

			if ((file.ShortName != ".") && (file.ShortName != "..")) {
				bool done = false;

				if (any || MatchPathRegex(file.ShortName, pattern1)) {
					ok = (*fn)(&file, Context);
					if (!ok) break;
					done = true;
				}
				if ((nSubDirs > 0) && (file.Attrib & FILE_FLAG_IS_DIR)) {
					if (!done) {
						ok = (*fn)(&file, Context);
						if (!ok) break;
					}

					ok = Recurse(file.FileName, Pattern, nSubDirs - 1, fn, Context);
					if (!ok) break;
				}
			}
		} while (ok && ::FindNextFile(handle, &finddata));

		::FindClose(handle);
	}

	return ok;
}

bool GetFileDetails(const AString& FileName, FILE_FIND *file)
{
	WIN32_FIND_DATA finddata;
	HANDLE handle;
	bool ok = false;

	if ((handle = ::FindFirstFile(FileName, &finddata)) != INVALID_HANDLE_VALUE) {
		if (file) {
			SetFileData(FileName.PathPart(), finddata, file);
		}

		::FindClose(handle);

		ok = true;
	}

	return ok;
}
#endif

typedef struct {
	AString 	 Pattern;
	AList   	 *pList;
	AQuitHandler *pQuitHandler;
	uint16_t		 AttribMask;
	uint16_t		 AttribCompare;
	bool		 bAnyName;
} COLLECT_CONTEXT;

static bool __CollectFiles(const FILE_FIND *file, void *Context)
{
	COLLECT_CONTEXT *p = (COLLECT_CONTEXT *)Context;

	if ((file->Attrib & p->AttribMask) == p->AttribCompare) {
#if 0
		debug("Matching '%s' against '%s': %u\n",
			  file->FileName.FilePart().str(),
			  p->Pattern.str(),
			  (uint_t)MatchPathRegex(file->FileName.FilePart(), p->Pattern));
#endif

		if (p->bAnyName || MatchPathRegex(file->FileName.FilePart(), p->Pattern)) {
			AString *str = new AString(file->FileName);
			if (str) p->pList->Add(str, &AString::AlphaCompareNoCase);
		}
	}

	return p->pQuitHandler ? !p->pQuitHandler->HasQuit() : true;
}

bool CollectFiles(const AString& Path, const AString& Pattern, uint_t nSubdirs, AList& list, uint16_t attribMask, uint16_t attribCmp, AQuitHandler *pQuitHandler)
{
	COLLECT_CONTEXT context;

	context.Pattern 	  = ParsePathRegex(Pattern);
	context.pList   	  = &list;
	context.pQuitHandler  = pQuitHandler;
	context.AttribMask    = attribMask;
	context.AttribCompare = attribCmp;
	context.bAnyName      = IsRegexAnyPattern(context.Pattern);

	return ::Recurse(Path.CatPath("*"), nSubdirs, &__CollectFiles, &context);
}

bool TraverseFiles(const AList& list, bool (*fn)(const FILE_FIND *file, void *context), void *context)
{
	FILE_FIND file;
	const AListNode *node = list.First();
	const AString *str;
	bool  success = true;

	while (node && success) {
		if ((str = AString::Cast(node)) != NULL) {
			success = ::GetFileDetails(*str, &file);
			if (success) success = (*fn)(&file, context);
		}

		node = node->Next();
	}

	return success;
}

bool CreateDirectory(const AString& dir)
{
	AString parentdir = dir.PathPart();
	FILE_FIND file;
	bool success = true;

	if (parentdir.Valid() && (parentdir != "/") && !::GetFileDetails(parentdir, &file)) success = CreateDirectory(parentdir);

	if (success && (dir.FilePart().Valid()) && !::GetFileDetails(dir, &file)) {
#ifdef __LINUX__
		success = (mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH) == 0);
#else
		success = ::CreateDirectoryA(dir.str(), NULL);
#endif
	}

	return success;
}

bool ChangeDirectory(const AString& dir)
{
	bool success = true;

#ifdef __LINUX__
	success = (chdir(dir) == 0);
#else
	success = ::SetCurrentDirectory(dir.str());
#endif

	return success;
}

AString GetCurrentDirectory()
{
	static char dir[FILENAME_MAX];
	
#ifdef __LINUX__
	if (getcwd(dir, sizeof(dir) - 1) != 0) {
		debug("getcwd failed (%s)\n", strerror(errno));
	}
#else
	::GetCurrentDirectory(sizeof(dir) - 1, dir);
#endif

	return AString(dir);
}

/*----------------------------------------------------------------------------------------------------*/

NODETYPE_IMPLEMENT(AFileNode);

AFileNode::AFileNode(const AString& filename) : AListNode()
{
	::GetFileDetails(filename, &FileData);
}

AFileNode::AFileNode(const FILE_FIND& file) : AListNode()
{
	FileData = file;
}

AFileNode::~AFileNode()
{
}

sint_t AFileNode::CompareByName(const AListNode *pNode1, const AListNode *pNode2, void *context)
{
	const AFileNode *fNode1 = AFileNode::Cast(pNode1);
	const AFileNode *fNode2 = AFileNode::Cast(pNode2);
	uint_t flags = context ? *(uint_t *)context : 0;
	sint_t ret = 0, mul = (flags & SORT_FLAG_REVERSE) ? -1 : 1;
	bool mixed = ((flags & SORT_FLAG_MIXED) != 0);

	if (fNode1 && fNode2) {
		if (mixed || (fNode1->IsDir() == fNode2->IsDir())) {
			ret = mul * CompareNoCase(fNode1->GetShortName(), fNode2->GetShortName());
		}
		else if (fNode1->IsDir()) ret = -1;
		else					  ret =  1;
	}

	return ret;
}

sint_t AFileNode::CompareByPath(const AListNode *pNode1, const AListNode *pNode2, void *context)
{
	const AFileNode *fNode1 = AFileNode::Cast(pNode1);
	const AFileNode *fNode2 = AFileNode::Cast(pNode2);
	uint_t flags = context ? *(uint_t *)context : 0;
	sint_t ret = 0, mul = (flags & SORT_FLAG_REVERSE) ? -1 : 1;
	bool mixed = ((flags & SORT_FLAG_MIXED) != 0);

	if (fNode1 && fNode2) {
		if (mixed || (fNode1->IsDir() == fNode2->IsDir())) {
			ret = mul * CompareNoCase(fNode1->GetFilename().PathPart(), fNode2->GetFilename().PathPart());
			if (ret == 0) ret = mul * CompareNoCase(fNode1->GetShortName(), fNode2->GetShortName());
		}
		else if (fNode1->IsDir()) ret = -1;
		else					  ret =  1;
	}

	return ret;
}

sint_t AFileNode::CompareByExt(const AListNode *pNode1, const AListNode *pNode2, void *context)
{
	const AFileNode *fNode1 = AFileNode::Cast(pNode1);
	const AFileNode *fNode2 = AFileNode::Cast(pNode2);
	uint_t flags = context ? *(uint_t *)context : 0;
	sint_t ret = 0, mul = (flags & SORT_FLAG_REVERSE) ? -1 : 1;
	bool mixed = ((flags & SORT_FLAG_MIXED) != 0);

	if (fNode1 && fNode2) {
		if (mixed || (fNode1->IsDir() == fNode2->IsDir())) {
			ret = mul * CompareNoCase(fNode1->GetShortName().Suffix(), fNode2->GetShortName().Suffix());
			if (ret == 0) ret = mul * CompareNoCase(fNode1->GetShortName(), fNode2->GetShortName());
		}
		else if (fNode1->IsDir()) ret = -1;
		else					  ret =  1;
	}

	return ret;
}

sint_t AFileNode::CompareByDate(const AListNode *pNode1, const AListNode *pNode2, void *context)
{
	const AFileNode *fNode1 = AFileNode::Cast(pNode1);
	const AFileNode *fNode2 = AFileNode::Cast(pNode2);
	uint_t flags = context ? *(uint_t *)context : 0;
	sint_t ret = 0, mul = (flags & SORT_FLAG_REVERSE) ? -1 : 1;
	bool mixed = ((flags & SORT_FLAG_MIXED) != 0);

	if (fNode1 && fNode2) {
		if (mixed || (fNode1->IsDir() == fNode2->IsDir())) {
			ret = mul * COMPARE_ITEMS(fNode1->GetFileDate(), fNode2->GetFileDate());
			if (ret == 0) ret = mul * CompareNoCase(fNode1->GetShortName(), fNode2->GetShortName());
		}
		else if (fNode1->IsDir()) ret = -1;
		else					  ret =  1;
	}

	return -ret;
}


sint_t AFileNode::CompareBySize(const AListNode *pNode1, const AListNode *pNode2, void *context)
{
	const AFileNode *fNode1 = AFileNode::Cast(pNode1);
	const AFileNode *fNode2 = AFileNode::Cast(pNode2);
	uint_t flags = context ? *(uint_t *)context : 0;
	sint_t ret = 0, mul = (flags & SORT_FLAG_REVERSE) ? -1 : 1;
	bool mixed = ((flags & SORT_FLAG_MIXED) != 0);

	if (fNode1 && fNode2) {
		if (mixed || (fNode1->IsDir() == fNode2->IsDir())) {
			ret = mul * COMPARE_ITEMS(fNode1->GetFileSize(), fNode2->GetFileSize());
			if (ret == 0) ret = mul * CompareNoCase(fNode1->GetShortName(), fNode2->GetShortName());
		}
		else if (fNode1->IsDir()) ret = -1;
		else					  ret =  1;
	}

	return ret;
}

COMPARE_FUNC AFileNode::GetSortFunction(uint_t n)
{
	static const COMPARE_FUNC fns[] = {&CompareByName, &CompareByPath, &CompareByExt, &CompareByDate, &CompareBySize};
	n &= SORT_BY_MASK;
	return (n < SORT_BY_ITEMS) ? fns[n] : fns[0];
}

/*----------------------------------------------------------------------------------------------------*/

typedef struct {
	AString 	 Pattern;
	AList   	 *pList;
	AQuitHandler *pQuitHandler;
	uint16_t		 AttribSet;
	uint16_t		 AttribClr;
	bool		 bAnyName;
	COMPARE_FUNC CmpFunc;
	uint_t		 CmpFlags;
} COLLECTEX_CONTEXT;

static bool __CollectFilesEx(const FILE_FIND *file, void *Context)
{
	COLLECTEX_CONTEXT *p = (COLLECTEX_CONTEXT *)Context;

	if (!(file->Attrib & p->AttribClr) && !((~file->Attrib) & p->AttribSet)) {
		if (p->bAnyName || MatchPathRegex(file->FileName.FilePart(), p->Pattern)) {
			AFileNode *node = new AFileNode(*file);
			if (node) p->pList->Add(node, p->CmpFunc, &p->CmpFlags);
		}
	}

	return p->pQuitHandler ? !p->pQuitHandler->HasQuit() : true;
}

bool CollectFilesEx(const AString& Path, const AString& Pattern, uint_t nSubdirs, AList& list, uint_t sortFlags, uint16_t attribSet, uint16_t attribClr, AQuitHandler *pQuitHandler)
{
	COLLECTEX_CONTEXT context;

	context.Pattern 	  = ParsePathRegex(Pattern);
	context.pList   	  = &list;
	context.pQuitHandler  = pQuitHandler;
	context.AttribSet     = attribSet;
	context.AttribClr	  = attribClr;
	context.bAnyName      = IsRegexAnyPattern(context.Pattern);
	context.CmpFunc		  = AFileNode::GetSortFunction(sortFlags);
	context.CmpFlags	  = sortFlags;

	return ::Recurse(Path.CatPath("*"), nSubdirs, &__CollectFilesEx, &context);
}

bool TraverseFiles(const AList& list, bool (*fn)(const AFileNode& file, void *context), void *context)
{
	const AListNode *node = list.First();
	const AFileNode *file;
	bool  success = true;

	while (node && success) {
		if ((file = AFileNode::Cast(node)) != NULL) {
			success = (*fn)(*file, context);
		}

		node = node->Next();
	}

	return success;
}
