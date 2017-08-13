
all: default-build

MAKEFILEDIR=makefiles

LIBRARY:=rdlib
WINDOWSLIB:=winlib

include $(MAKEFILEDIR)/makefile.init
include $(MAKEFILEDIR)/makefile.windowslib

OBJECTS +=										\
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

HEADERS +=										\
	3DTrans.h									\
	3DWorld.h									\
	Allocator.h									\
	BMPImage.h									\
	CircularBuffer.h							\
	Complex.h									\
	DataList.h									\
	Database.h									\
	DateObjectPair.h							\
	DateTime.h									\
	ErrorHandler.h								\
	EvalNumber.h								\
	Evaluator.h									\
	FileHash.h									\
	HTMLTags.h									\
	HTTPRequest.h								\
	Hash.h										\
	Interpolation.h								\
	ListNode.h									\
	Logger.h									\
	Matrix.h									\
	MessageResponseHandler.h					\
	NodeType.h									\
	PointSizeRect.h								\
	PostgresDatabase.h							\
	QuitHandler.h								\
	Recurse.h									\
	Regex.h										\
	SQLQuery.h									\
	SettingsHandler.h							\
	SocketServer.h								\
	StdData.h									\
	StdDev.h									\
	StdFile.h									\
	StdMemFile.h								\
	StdSerial.h									\
	StdSocket.h									\
	StdUri.h									\
	StringList.h								\
	StructList.h								\
	StructuredNode.h							\
	TextServer.h								\
	Thread.h									\
	ThreadLock.h								\
	UDPServer.h									\
	XMLDecode.h									\
	authorize.h									\
	blowfish.h									\
	blowfish_imp.h								\
	cfft.h										\
	crc32.h										\
	exif.h										\
	fastcomplex.h								\
	fastfft.h									\
	jpegfix.h									\
	jpeginfo.h									\
	md5.h										\
	misc.h										\
	mt19937ar.h									\
	pg_types.h									\
	simpleeval.h								\
	strsup.h									\
	tag.h										\
	types.h										\
	wxsup.h

GLOBAL_CFLAGS  += -D_FILE_OFFSET_BITS=64 -fsigned-char
GLOBAL_CFLAGS  += $(shell $(CC) -Wall -o arch src/arch.c && ./arch)

EXTRA_CFLAGS   += $(call pkgcflags,libpq)
EXTRA_CXXFLAGS += -std=c++11

GLOBAL_LIBS    += -lm $(shell ./arch --libs)
GLOBAL_LIBS    += $(call pkglibs,libpq) -lpthread -ljpeg

include $(MAKEFILEDIR)/makefile.lib

LOCAL_SHARE_FILES := $(shell find makefiles) makefiles/copyifnewer makefiles/library.pc.in
INSTALLEDSHAREFILES += $(LOCAL_SHARE_FILES:%=$(INSTALLSHAREDST)/%)

$(INSTALLSHAREDST)/makefiles/%: makefiles/%
	@$(SUDO) $(MAKEFILEDIR)/copyifnewer "$<" "$@"

include $(MAKEFILEDIR)/makefile.post
