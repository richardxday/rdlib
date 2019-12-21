
#ifndef __JPEG_INFO__
#define __JPEG_INFO__

#include "DateTime.h"

typedef struct {
    AString   CameraMake;
    AString   CameraModel;
    int       Width;
    int       Height;
    uint_t    Orientation;
    ADateTime DateTime;
    bool      bDateValid;
    AString   Comments;
} JPEG_INFO;

extern bool ReadJPEGInfo(const char *filename, JPEG_INFO& info);

#if 0
extern bool LoadJPEG(ACanvas *pC, const char *filename, ACanvas *pC1 = &Screen);
extern bool SaveJPEG(ACanvas *pC, const char *filename, int quality = 85);
#endif

#endif
