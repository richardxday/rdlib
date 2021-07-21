
all: default-build

MAKEFILEDIR := makefiles

LIBRARY	   := rdlib
WINDOWSLIB := winlib

include $(MAKEFILEDIR)/makefile.init
include $(MAKEFILEDIR)/makefile.windowslib

OBJECTS +=										\
	3DTrans.o									\
	BMPImage.o									\
	DataList.o									\
	DateObjectPair.o							\
	DateTime.o									\
	EvalNumber.o								\
	Evaluator.o									\
	HTMLTags.o									\
	HTTPRequest.o								\
	Hash.o										\
	ListNode.o									\
	Logger.o									\
	Matrix.o									\
	MessageResponseHandler.o					\
	NodeType.o									\
	PointSizeRect.o								\
	PostgresDatabase.o							\
	QuitHandler.o								\
	Recurse.o									\
	Regex.o										\
	SettingsHandler.o							\
	SocketServer.o								\
	StdData.o									\
	StdDev.o									\
	StdFile.o									\
	StdMemFile.o								\
	StdSerial.o									\
	StdSocket.o									\
	StdUri.o									\
	StringList.o								\
	StructList.o								\
	StructuredNode.o							\
	TextServer.o								\
	Thread.o									\
	ThreadLock.o								\
	UDPServer.o									\
	XMLDecode.o									\
	authorize.o									\
	blowfish.o									\
	blowfish_imp.o								\
	cfft.o										\
	crc32.o										\
	exif.o										\
	fastcomplex.o								\
	fastfft.o									\
	jpeginfo.o									\
	md5.o										\
	misc.o										\
	mt19937ar.o									\
	printtable.o								\
	simpleeval.o								\
	strsup.o									\
	tag.o

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
	printtable.h								\
	simpleeval.h								\
	strsup.h									\
	tag.h										\
	types.h										\
	wxsup.h

include $(MAKEFILEDIR)/makefile.prebuild

INSTALLEDMAKEFILESDST := $(INSTALLSHAREDST)/makefiles

RDLIB_CONFIG := rdlib-config

GLOBAL_COMMON_FLAGS += -D_FILE_OFFSET_BITS=64 -fsigned-char
GLOBAL_COMMON_FLAGS += $(shell $(CC) $(GLOBAL_COMMON_FLAGS) $(GLOBAL_CFLAGS) "-DRDLIB_ROOTDIR=\"$(ROOTDIR)\"" "-DRDLIB_PREFIX=\"$(PREFIX)\"" "-DRDLIB_MAKEFILES=\"$(INSTALLEDMAKEFILESDST)\"" -o $(RDLIB_CONFIG)$(APPLICATION_SUFFIX) src/$(RDLIB_CONFIG).c && ./$(RDLIB_CONFIG))

EXTRA_COMMON_FLAGS += $(call pkgcflags,libpq)
EXTRA_CFLAGS	   += -std=c99
EXTRA_CXXFLAGS	   += -std=c++11

GLOBAL_LIBS		   += -lm $(shell ./$(RDLIB_CONFIG) --libs)
GLOBAL_LIBS		   += $(call pkglibs,libpq) -lpthread -ljpeg

include $(MAKEFILEDIR)/makefile.lib

LOCAL_SHARE_FILES := $(shell find makefiles) makefiles/copyifnewer makefiles/library.pc.in
INSTALLEDSHAREFILES += $(LOCAL_SHARE_FILES:%=$(INSTALLSHAREDST)/%)

$(INSTALLEDMAKEFILESDST)/%: makefiles/%
	@$(SUDO) $(MAKEFILEDIR)/copyifnewer "$<" "$@"

$(INSTALLBINDST)/$(RDLIB_CONFIG)$(APPLICATION_SUFFIX): $(RDLIB_CONFIG)$(APPLICATION_SUFFIX)
	@$(SUDO) $(MAKEFILEDIR)/copyifnewer "$<" "$@"

INSTALLEDBINARIES += $(INSTALLBINDST)/$(RDLIB_CONFIG)$(APPLICATION_SUFFIX)

CLEANFILES += $(RDLIB_CONFIG)$(APPLICATION_SUFFIX)

include $(MAKEFILEDIR)/makefile.post
