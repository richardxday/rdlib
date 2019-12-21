
#ifndef __3D_WORLD__
#define __3D_WORLD__

#include "3DTrans.h"
#include "PointSizeRect.h"

/*
 * A simple stereo vision class
 *
 * using a single A3DTransform and a horizontal offset represents the difference in view between the two eyes of the viewer
 */

class A3DWorld {
public:
    A3DWorld() : Perspective(100.0),
                 Separation(1.0),
                 Scale(1.0),
                 ScreenSeparation(100) {
    }

    A3DTransform& GetTransform() {return Trans;}

    void   SetSeparation(double sep)    {Separation = sep;}
    double GetSeparation() const        {return Separation;}

    void   SetScreenSeparation(int sep) {ScreenSeparation = sep;}
    double SetScreenSeparation() const  {return ScreenSeparation;}

    void   SetPerspective(double per)   {Perspective = per;}
    double GetPerspective() const       {return Perspective;}

    void          SetCentre(const APoint& pt) {Centre = pt;}
    const APoint& GetCentre() const           {return Centre;}

    void   SetScale(double scale = 1.0) {Scale = scale;}
    double GetScale() const             {return Scale;}

    void            SetOffset(const A3DPoint& offset) {Offset = offset;}
    const A3DPoint& GetOffset() const                 {return Offset;}

    void Convert(const A3DPoint& pt, APoint& spt, bool left) {
        A3DPoint pt1 = pt;
        Trans.Transform(pt1, false);
        pt1 += Offset;
        pt1.RotateXZ(left ? -Separation : Separation);
        pt1.ApplyPerspective(Perspective);
        pt1.ToScreenCoords(Centre.x + (left ? -ScreenSeparation : ScreenSeparation), Centre.y, spt.x, spt.y, Scale);
    }
    void Convert(const A3DPoint& pt, APoint& spt1, APoint& spt2) {
        A3DPoint pt1 = pt, pt2;
        Trans.Transform(pt1, false);
        pt1 += Offset;
        pt2 = pt1;
        pt1.RotateXZ(-Separation);
        pt2.RotateXZ(Separation);
        pt1.ApplyPerspective(Perspective);
        pt2.ApplyPerspective(Perspective);
        pt1.ToScreenCoords(Centre.x - ScreenSeparation, Centre.y, spt1.x, spt1.y, Scale);
        pt2.ToScreenCoords(Centre.x + ScreenSeparation, Centre.y, spt2.x, spt2.y, Scale);
    }

protected:
    A3DTransform Trans;
    A3DPoint     Offset;
    double       Perspective;
    double       Separation;
    APoint       Centre;
    double       Scale;
    int          ScreenSeparation;
};

#endif
