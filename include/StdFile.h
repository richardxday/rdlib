
#ifndef __STANDARD_FILE__
#define __STANDARD_FILE__

#include "misc.h"
#include "StdData.h"
#include "strsup.h"

/*--------------------------------------------------------------------------------*/
/** AStdFile - abstraction of FILE support
 *
 *  See StdData.h for details of interface
 */
/*--------------------------------------------------------------------------------*/
class AStdFile : public AStdData {
public:
    /*--------------------------------------------------------------------------------*/
    /** Open file
     *
     * @param filename filename
     * @param mode fopen() type mode ('r', 'w', 'r+', etc.)
     */
    /*--------------------------------------------------------------------------------*/
    AStdFile(const char *filename = NULL, const char *mode = "r");
    /*--------------------------------------------------------------------------------*/
    /** Utilise FILE * pointer to perform file functions
     *
     * @param p pointer to valid FILE
     * @param filename original filename (or NULL)
     * @param binary true if file was opened in binary mode
     *
     * @note the file is NOT closed on close() or destruction!
     */
    /*--------------------------------------------------------------------------------*/
    AStdFile(FILE *p, const char *filename = NULL, bool binary = false);
    virtual ~AStdFile();

    /*--------------------------------------------------------------------------------*/
    /** Open file
     *
     * @param filename filename
     * @param mode fopen() type mode ('r', 'w', 'r+', etc.)
     *
     * @return true if file open successful
     */
    /*--------------------------------------------------------------------------------*/
    virtual bool open(const char *filename, const char *mode = "r");

    /*--------------------------------------------------------------------------------*/
    /** Utilise FILE * pointer to perform file functions
     *
     * @param p pointer to valid FILE
     * @param filename original filename (or NULL)
     * @param binary true if file was opened in binary mode
     *
     * @return true if file open successful (i.e. p is non-null and this object is not already 'open')
     *
     * @note the file is NOT closed on close() or destruction!
     */
    /*--------------------------------------------------------------------------------*/
    virtual bool open(FILE *p, const char *filename = NULL, bool binary = false);

    /*--------------------------------------------------------------------------------*/
    /** Return whether this file was opened by this object
     */
    /*--------------------------------------------------------------------------------*/
    bool wasopened() const {return ((flags & Flag_Opened) != 0);}

    /*--------------------------------------------------------------------------------*/
    /** Return whether file was opened in binary mode
     */
    /*--------------------------------------------------------------------------------*/
    bool binarymode() const {return ((flags & Flag_Binary) != 0);}

    /*--------------------------------------------------------------------------------*/
    /** Duplicate this file handle - i.e. provide an additional, read-only, handle to the file
     */
    /*--------------------------------------------------------------------------------*/
    virtual AStdFile *dup() const;

    /*--------------------------------------------------------------------------------*/
    /** The following functions are documented in StdData.h
     */
    /*--------------------------------------------------------------------------------*/
    virtual sint_t  close();
    virtual bool    isopen() const {return (fp != NULL);}

    virtual slong_t bytesavailable();

    virtual sint_t  vprintf(const char *format, va_list ap);

    virtual sint_t  putc(sint_t c);
    virtual sint_t  getc();

    virtual sint_t  puts(const char *str);
    virtual char    *gets(char *str, sint_t n);

    virtual slong_t tell();
    virtual sint_t  seek(slong_t offset, uint_t origin = SEEK_SET);
    virtual sint_t  flush();
    virtual void    rewind();

    virtual sint_t  eof();
    virtual sint_t  error();

    virtual sint_t  setvbuf(char *buffer, int mode, size_t size);

    virtual sint_t  setbinarymode(bool binary = true);

    /*--------------------------------------------------------------------------------*/
    /** Get raw FILE object
     *
     * @return FILE object
     */
    /*--------------------------------------------------------------------------------*/
    virtual FILE    *GetFile() {return fp;}

    /*--------------------------------------------------------------------------------*/
    /** Return whether file exists or not
     *
     * @param filename filename
     *
     * @return true if file was successfully opened (for win32 systems) or stat() is successful (Linux/BSD systems)
     */
    /*--------------------------------------------------------------------------------*/
    static  bool    exists(const char *filename);

    NODETYPE(AStdFile, AStdData);

protected:
    /*--------------------------------------------------------------------------------*/
    /** The following functions are documented in StdData.h
     */
    /*--------------------------------------------------------------------------------*/
    virtual slong_t readdata(void *buf, size_t bytes);
    virtual slong_t writedata(const void *buf, size_t bytes);

    enum {
        Flag_Opened = 1,
        Flag_Binary = 2,
    };

protected:
    AString filename;
    FILE    *fp;
    uint8_t flags;

    NODETYPE_DEFINE(AStdFile);
};

#endif
