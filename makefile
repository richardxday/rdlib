
all: default-build

MAKEFILEDIR=makefiles

include $(MAKEFILEDIR)/makefile.init

LIBRARY:=rdlib

LIBRARY_VERSION_MAJOR:=0
LIBRARY_VERSION_MINOR:=1
LIBRARY_VERSION_RELEASE:=0
LIBRARY_VERSION_BUILD:=0
LIBRARY_DESCRIPTION:=Utility library

OBJECTS:=										\
	3DTrans.o									\
	authorize.o									\
	blowfish.o									\
	blowfish_imp.o								\
	BMPImage.o									\
	cfft.o										\
	crc32.o										\
	DataList.o									\
	DateObjectPair.o							\
	DateTime.o									\
	EvalNumber.o								\
	Evaluator.o									\
	exif.o										\
	fastcomplex.o								\
	fastfft.o									\
	Hash.o										\
	HTMLTags.o									\
	HTTPRequest.o								\
	jpeginfo.o									\
	ListNode.o									\
	Logger.o									\
	Matrix.o									\
	md5.o										\
	MessageResponseHandler.o					\
	misc.o										\
	mt19937ar.o									\
	NodeType.o									\
	PointSizeRect.o								\
	PostgresDatabase.o							\
	QuitHandler.o								\
	Recurse.o									\
	Regex.o										\
	SettingsHandler.o							\
	simpleeval.o								\
	SocketServer.o								\
	StdData.o									\
	StdDev.o									\
	StdFile.o									\
	StdMemFile.o								\
	StdSerial.o									\
	StdSocket.o									\
	StdUri.o									\
	StringList.o								\
	strsup.o									\
	StructList.o								\
	StructuredNode.o							\
	tag.o										\
	TextServer.o								\
	Thread.o									\
	ThreadLock.o								\
	UDPServer.o									\
	XMLDecode.o

HEADERS:=										\
	3DTrans.h									\
	3DWorld.h									\
	Allocator.h									\
	authorize.h									\
	blowfish.h									\
	blowfish_imp.h								\
	BMPImage.h									\
	cfft.h										\
	Complex.h									\
	crc32.h										\
	Database.h									\
	DataList.h									\
	DateObjectPair.h							\
	DateTime.h									\
	ErrorHandler.h								\
	EvalNumber.h								\
	Evaluator.h									\
	exif.h										\
	fastcomplex.h								\
	fastfft.h									\
	FileHash.h									\
	Hash.h										\
	HTMLTags.h									\
	HTTPRequest.h								\
	Interpolation.h								\
	jpegfix.h									\
	jpeginfo.h									\
	ListNode.h									\
	Logger.h									\
	Matrix.h									\
	md5.h										\
	MessageResponseHandler.h					\
	misc.h										\
	mt19937ar.h									\
	NodeType.h									\
	pg_types.h									\
	PointSizeRect.h								\
	PostgresDatabase.h							\
	QuitHandler.h								\
	Recurse.h									\
	Regex.h										\
	SettingsHandler.h							\
	simpleeval.h								\
	SocketServer.h								\
	SQLQuery.h									\
	StdData.h									\
	StdDev.h									\
	StdFile.h									\
	StdMemFile.h								\
	StdSerial.h									\
	StdSocket.h									\
	StdUri.h									\
	StringList.h								\
	strsup.h									\
	StructList.h								\
	StructuredNode.h							\
	tag.h										\
	TextServer.h								\
	Thread.h									\
	ThreadLock.h								\
	types.h										\
	UDPServer.h									\
	wxsup.h										\
	XMLDecode.h

GLOBAL_CFLAGS := -D__LINUX__ -D_FILE_OFFSET_BITS=64 -fsigned-char
GLOBAL_CFLAGS += $(shell gcc -Wall -o arch src/arch.c && ./arch)

EXTRA_CFLAGS += $(shell pkg-config --cflags libpq)
EXTRA_CXXFLAGS += -std=c++11

GLOBAL_LIBS := -lm -lrt
GLOBAL_LIBS += $(shell ./arch)
GLOBAL_LIBS += $(shell pkg-config --libs libpq) -lpthread -ljpeg

include $(MAKEFILEDIR)/makefile.lib

LOCAL_SHARE_FILES := $(shell find makefiles) makefiles/copyifnewer makefiles/library.pc.in
INSTALLEDSHAREFILES += $(LOCAL_SHARE_FILES:%=$(INSTALLSHAREDST)/%)

$(INSTALLSHAREDST)/makefiles/%: makefiles/%
	@$(SUDO) $(MAKEFILEDIR)/copyifnewer "$(INSTALLSHAREDST)/makefiles" "$<" "$@"

include $(MAKEFILEDIR)/makefile.post
