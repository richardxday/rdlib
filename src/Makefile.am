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
	BMPImage.cpp								\
	DataList.cpp								\
	DateObjectPair.cpp							\
	DateTime.cpp								\
	EvalNumber.cpp								\
	Evaluator.cpp								\
	HTMLTags.cpp								\
	HTTPRequest.cpp								\
	Hash.cpp									\
	JSONParser.cpp								\
	ListNode.cpp								\
	Logger.cpp									\
	Matrix.cpp									\
	MessageResponseHandler.cpp					\
	NodeType.cpp								\
	PointSizeRect.cpp							\
	QuitHandler.cpp								\
	Recurse.cpp									\
	Regex.cpp									\
	SettingsHandler.cpp							\
	SocketServer.cpp							\
	StdData.cpp									\
	StdDev.cpp									\
	StdFile.cpp									\
	StdMemFile.cpp								\
	StdSerial.cpp								\
	StdSocket.cpp								\
	StdUri.cpp									\
	StringList.cpp								\
	StructList.cpp								\
	StructuredNode.cpp							\
	TextServer.cpp								\
	Thread.cpp									\
	ThreadLock.cpp								\
	UDPServer.cpp								\
	XMLDecode.cpp								\
	authorize.cpp								\
	blowfish.cpp								\
	blowfish_imp.c								\
	cfft.cpp									\
	crc32.cpp									\
	exif.cpp									\
	fastcomplex.cpp								\
	fastfft.cpp									\
	jpeginfo.cpp								\
	md5.c										\
	misc.cpp									\
	mt19937ar.c									\
	simpleeval.cpp								\
	strsup.cpp									\
	tag.cpp

pkginclude_HEADERS = 							\
	3DTrans.h									\
	3DWorld.h									\
	Allocator.h									\
	BMPImage.h									\
	Complex.h									\
	DataList.h									\
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
	JSONParser.h								\
	ListNode.h									\
	Logger.h									\
	Matrix.h									\
	MessageResponseHandler.h					\
	NodeType.h									\
	PointSizeRect.h								\
	QuitHandler.h								\
	Recurse.h									\
	Regex.h										\
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
	simpleeval.h								\
	strsup.h									\
	tag.h										\
	types.h										\
	wxsup.h

noinst_HEADERS =

nodist_pkginclude_HEADERS =

DISTCLEANFILES =
