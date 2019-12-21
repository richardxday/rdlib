#ifndef __FFT__
#define __FFT__

#include "misc.h"
#include "DataList.h"

class AFFT;
typedef struct {
    const AFFT *fft;
    float  *real;
    float  *imag;
    float  Scale;
    double wr, wi;
    int    Index1, Count1;
    int    Index2, Count2;
    int    Pos;
    int    BRPos;
    int    Butterflies;
    int    ReorderCount;
} FFT_STATE;

#define FFTSCALE 1.0
#define IFFTSCALE (1.0 / (double)Points)

class AFFT {
public:
    AFFT(int iPoints);
    ~AFFT();

    bool Valid() const {return (Bitrev && Twiddle);}

    float  *Create() const;
    double *CreateDouble() const;
    float  *Create(float *) const {return Create();}
    double *Create(double *) const {return CreateDouble();}

    int   GetPoints() const {return Points;}
    int   GetTotalOperations() const {return LOGN(Points, 2) * (Points / 2) + Points;}

    void  AddUsage() {UsageCount++;}
    bool  RemoveUsage() {return ((--UsageCount) == 0);}

    float *GetReal(float *data) const {return data;}
    float *GetImag(float *data) const {return data + Points;}
    const float *GetReal(const float *data) const {return data;}
    const float *GetImag(const float *data) const {return data + Points;}

    double *GetReal(double *data) const {return data;}
    double *GetImag(double *data) const {return data + Points;}
    const double *GetReal(const double *data) const {return data;}
    const double *GetImag(const double *data) const {return data + Points;}

    void Clear(float *data) const {memset(data, 0, 2 * Points * sizeof(*data));}
    void ClearReal(float *data) const {memset(GetReal(data), 0, Points * sizeof(*data));}
    void ClearImag(float *data) const {memset(GetImag(data), 0, Points * sizeof(*data));}
    void Clear(double *data) const {memset(data, 0, 2 * Points * sizeof(*data));}
    void ClearReal(double *data) const {memset(GetReal(data), 0, Points * sizeof(*data));}
    void ClearImag(double *data) const {memset(GetImag(data), 0, Points * sizeof(*data));}

    void Forward(float *data) const {
        fft(GetReal(data), GetImag(data), (float)FFTSCALE);
    }
    void Inverse(float *data) const {
        fft(GetImag(data), GetReal(data), (float)IFFTSCALE);
    }
    void Forward(double *data) const {
        fft(GetReal(data), GetImag(data), (double)FFTSCALE);
    }
    void Inverse(double *data) const {
        fft(GetImag(data), GetReal(data), (double)IFFTSCALE);
    }

    void Magnitude(float *data, bool Full = false, bool Phase = false) const;
    void InverseMagnitude(float *data) const;

    void Magnitude(double *data, bool Full = false, bool Phase = false) const;
    void InverseMagnitude(double *data) const;

    void MultiplyFFTs(float *data, const float *data1, const float *data2) const;
    void MultiplyFFTs(double *data, const double *data1, const double *data2) const;

    void BeginForward(FFT_STATE& state, float *data) const {
        InitState(state, GetReal(data), GetImag(data), (float)FFTSCALE);
    }
    void BeginInverse(FFT_STATE& state, float *data) const {
        InitState(state, GetImag(data), GetReal(data), (float)IFFTSCALE);
    }

    void Process(FFT_STATE& state, int nOperations);

#define PACK_FUNCTIONS_DEFINE(type)                                                                     \
    void Interleave(type *dst, const type *src, int offset, int n) const;                               \
    void DeInterleave(type *dst, const type *src, int offset, int n) const;                             \
    void Unpack(type *dst1, type *dst2, const type *src) const;                                         \
    void Pack(type *dst, const type *src1, const type *src2) const;                                     \
    void UnpackAndInterleave(type *dst1, type *dst2, const type *src, int offset, int n) const;         \
    void DeInterleaveAndPack(type *dst, const type *src1, const type *src2, int offset, int n) const;   \
    void Convolve(type *dst, const type *src1, const type *src2) const;                                 \
    void ConvolveAdd(type *dst, const type *src1, const type *src2) const;                              \
    void DeConvolve(type *dst, const type *src1, const type *src2) const;

    PACK_FUNCTIONS_DEFINE(float);
    PACK_FUNCTIONS_DEFINE(double);

protected:
    void fft(float *real, float *imag, float Scale) const;
    void fft(double *real, double *imag, double Scale) const;

    void InitState(FFT_STATE& state, float *real, float *imag, float Scale) const;

protected:
    double *Twiddle;
    int    *Bitrev;
    int    Points, PointsDiv4;
    int    UsageCount;
};

class AFFTList {
public:
    AFFTList();
    ~AFFTList();

    AFFT *Create(int Points);
    void Delete(AFFT *fft);

protected:
    ADataList List;
};

extern AFFTList FFTList;

#endif
