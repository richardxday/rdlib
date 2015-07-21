pkgincludedir = $(includedir)/rdlib-@RDLIB_MAJORMINOR@/rdlib

lib_LTLIBRARIES = librdlib-@RDLIB_MAJORMINOR@.la

librdlib_@RDLIB_MAJORMINOR@_la_LDFLAGS =		\
	-no-undefined								\
	-version-info $(RDLIB_LIBVERSION)			\
     $(PTHREAD_LDFLAGS)							\
     $(RDLIB_LDFLAGS)

librdlib_@RDLIB_MAJORMINOR@_la_LIBADD =			\
    $(PTHREAD_LIBS)								\
	$(JPEG_LIBS)

librdlib_@RDLIB_MAJORMINOR@_la_CPPFLAGS =		\
	$(RDLIB_CFLAGS)								\
	$(PTHREAD_CFLAGS)							\
	$(JPEG_CGLAGS)

librdlib_@RDLIB_MAJORMINOR@_la_SOURCES =		\
	3DTrans.cpp									\
	authorize.cpp								\
	blowfish.cpp								\
	blowfish_imp.c								\
	BMPImage.cpp								\
	cfft.cpp									\
	crc32.cpp									\
	DataList.cpp								\
	DateObjectPair.cpp							\
	DateTime.cpp								\
	EvalNumber.cpp								\
	Evaluator.cpp								\
	exif.cpp									\
	fastcomplex.cpp								\
	fastfft.cpp									\
	Hash.cpp									\
	HTMLTags.cpp								\
	jpeginfo.cpp								\
	JSONParser.cpp								\
	ListNode.cpp								\
	Logger.cpp									\
	Matrix.cpp									\
	md5.c										\
	MessageResponseHandler.cpp					\
	misc.cpp									\
	mt19937ar.c									\
	NodeType.cpp								\
	PointSizeRect.cpp							\
	QuitHandler.cpp								\
	Recurse.cpp									\
	Regex.cpp									\
	SettingsHandler.cpp							\
	simpleeval.cpp								\
	SocketServer.cpp							\
	StdData.cpp									\
	StdDev.cpp									\
	StdFile.cpp									\
	StdMemFile.cpp								\
	StdSerial.cpp								\
	StdSocket.cpp								\
	StdUri.cpp									\
	StringList.cpp								\
	strsup.cpp									\
	StructList.cpp								\
	tag.cpp										\
	TextServer.cpp								\
	Thread.cpp									\
	ThreadLock.cpp								\
	UDPServer.cpp

pkginclude_HEADERS = 							\
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
	Interpolation.h								\
	jpegfix.h									\
	jpeginfo.h									\
	JSONParser.h								\
	ListNode.h									\
	Logger.h									\
	Matrix.h									\
	md5.h										\
	MessageResponseHandler.h					\
	misc.h										\
	mt19937ar.h									\
	NodeType.h									\
	PointSizeRect.h								\
	QuitHandler.h								\
	Recurse.h									\
	Regex.h										\
	SettingsHandler.h							\
	simpleeval.h								\
	SocketServer.h								\
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
	tag.h										\
	TextServer.h								\
	Thread.h									\
	ThreadLock.h								\
	types.h										\
	UDPServer.h									\
	wxsup.h

noinst_HEADERS =

nodist_pkginclude_HEADERS =

DISTCLEANFILES =
