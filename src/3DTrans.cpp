
#include <stdio.h>

#include "3DTrans.h"

/* end of includes */

void A3DTransform::Transform(A3DPoint& a, bool bWarp) const
{
    a.RotateXY(xy);
    a.RotateYZ(yz);
    a.RotateXZ(xz);
    a.x += htrans;
    if (bWarp) a.ApplyPerspective(per);
    a.x *= zoom;
    a.y *= zoom;
    a.z *= zoom;
}

void A3DTransform::InverseTransform(A3DPoint& a, bool bWarp) const
{
    if (zoom != 0.0) {
        a.z /= zoom;
        a.y /= zoom;
        a.x /= zoom;
    }
    a.x -= htrans;
    if (bWarp) a.RemovePerspective(per);
    a.RotateXZ(-xz);
    a.RotateYZ(-yz);
    a.RotateXY(-xy);
}

bool A3DTransform::Interpolate(const A3DTransform& target, double coeff, bool circular)
{
    bool updated = false;

    if (circular) {
        updated = ::InterpolateAngle(xy, target.xy, coeff);
        updated = ::InterpolateAngle(yz, target.yz, coeff) || updated;
        updated = ::InterpolateAngle(xz, target.xz, coeff) || updated;
    }
    else {
        updated = ::Interpolate(xy, target.xy, coeff);
        updated = ::Interpolate(yz, target.yz, coeff) || updated;
        updated = ::Interpolate(xz, target.xz, coeff) || updated;
    }
    updated = ::Interpolate(per, target.per, coeff) || updated;
    updated = ::Interpolate(zoom, target.zoom, coeff) || updated;
    updated = ::Interpolate(htrans, target.htrans, coeff) || updated;

    return updated;
}

bool A3DTransform::Interpolate2ndOrder(A3DTransform& velocity, const A3DTransform& target, double coeff, double speed, bool circular)
{
    bool updated = false;

    if (circular) {
        updated = ::Interpolate2ndOrderAngle(xy, velocity.xy, target.xy, coeff, speed);
        updated = ::Interpolate2ndOrderAngle(yz, velocity.yz, target.yz, coeff, speed) || updated;
        updated = ::Interpolate2ndOrderAngle(xz, velocity.xz, target.xz, coeff, speed) || updated;
    }
    else {
        updated = ::Interpolate2ndOrder(xy, velocity.xy, target.xy, coeff, speed);
        updated = ::Interpolate2ndOrder(yz, velocity.yz, target.yz, coeff, speed) || updated;
        updated = ::Interpolate2ndOrder(xz, velocity.xz, target.xz, coeff, speed) || updated;
    }
    updated = ::Interpolate2ndOrder(per, velocity.per, target.per, coeff, speed) || updated;
    updated = ::Interpolate2ndOrder(zoom, velocity.zoom, target.zoom, coeff, speed) || updated;
    updated = ::Interpolate2ndOrder(htrans, velocity.htrans, target.htrans, coeff, speed) || updated;

    return updated;
}

void A3DTransform::LimitAngles()
{
    LimitAngle(xy);
    LimitAngle(yz);
    LimitAngle(xz);
}

/*----------------------------------------------------------------------------------------------------*/

bool A3DPoint::Interpolate(const A3DPoint& target, double coeff)
{
    bool updated = false;

    updated = ::Interpolate(x, target.x, coeff);
    updated = ::Interpolate(y, target.y, coeff) || updated;
    updated = ::Interpolate(z, target.z, coeff) || updated;

    return updated;
}

bool A3DPoint::Interpolate2ndOrder(A3DPoint& velocity, const A3DPoint& target, double coeff, double speed)
{
    bool updated = false;

    updated = ::Interpolate2ndOrder(x, velocity.x, target.x, coeff, speed);
    updated = ::Interpolate2ndOrder(y, velocity.y, target.y, coeff, speed) || updated;
    updated = ::Interpolate2ndOrder(z, velocity.z, target.z, coeff, speed) || updated;

    return updated;
}

/*----------------------------------------------------------------------------------------------------*/

A3DPoint operator * (const A3DPoint& vec, const A3DMatrix& mat)
{
    double vec1[4], vec2[] = {vec.x, vec.y, vec.z, 1.0};
    MatrixVecMul(vec1, mat, vec2, 4);
    double mul = (vec1[3] != 0.0) ? 1.0 / vec1[3] : 1.0;
    return A3DPoint(vec1[0] * mul, vec1[1] * mul, vec2[2] * mul);
}

A3DPoint operator * (const A3DMatrix& mat, const A3DPoint& vec)
{
    double vec1[4], vec2[] = {vec.x, vec.y, vec.z, 1.0};
    MatrixVecMul(vec1, mat, vec2, 4);
    double mul = (vec1[3] != 0.0) ? 1.0 / vec1[3] : 1.0;
    return A3DPoint(vec1[0] * mul, vec1[1] * mul, vec2[2] * mul);
}

A3DMatrix operator * (const A3DMatrix& mat1, const A3DMatrix& mat2)
{
    A3DMatrix mat;
    MatrixMul(mat, mat1, mat2, 4);
    mat.Normalise();
    return mat;
}

A3DMatrix& A3DMatrix::operator *= (const A3DMatrix& mat)
{
    MatrixMul(*this, *this, mat, 4);
    Normalise();
    return *this;
}

A3DMatrix& A3DMatrix::operator *= (double val)
{
    if (val != 1.0) {
        A3DMatrix mat;
        mat.a11 = mat.a22 = mat.a33 = val;
        MatrixMul(*this, *this, mat, 4);
        Normalise();
    }
    return *this;
}

A3DMatrix& A3DMatrix::operator /= (double val)
{
    return operator *= (1.0 / val);
}

A3DMatrix& A3DMatrix::operator += (const A3DPoint& pt)
{
    a14 += pt.x;
    a24 += pt.y;
    a34 += pt.z;
    return *this;
}

A3DMatrix& A3DMatrix::operator -= (const A3DPoint& pt)
{
    a14 -= pt.x;
    a24 -= pt.y;
    a34 -= pt.z;
    return *this;
}

A3DMatrix& A3DMatrix::Rotate(double x, double y, double z, uint_t order)
{
    switch (order) {
        default:
        case ORDER_XYZ:
            RotateX(x);
            RotateY(y);
            RotateZ(z);
            break;

        case ORDER_XZY:
            RotateX(x);
            RotateZ(z);
            RotateY(y);
            break;

        case ORDER_YXZ:
            RotateY(y);
            RotateX(x);
            RotateZ(z);
            break;

        case ORDER_YZX:
            RotateY(y);
            RotateZ(z);
            RotateX(x);
            break;

        case ORDER_ZXY:
            RotateZ(z);
            RotateX(x);
            RotateY(y);
            break;

        case ORDER_ZYX:
            RotateZ(z);
            RotateY(y);
            RotateX(x);
            break;
    }

    return *this;
}

A3DMatrix& A3DMatrix::SetPerspective(double d)
{
    if (d == 0.0) a43 = 0.0;
    else          a43 = 1.0 / d;
    return *this;
}

A3DMatrix& A3DMatrix::Normalise()
{
    if ((a44 != 0.0) && (a44 != 1.0)) MatrixDiv(*this, *this, a44, 4);
    return *this;
}

/*----------------------------------------------------------------------------------------------------*/

double DotProduct(const A3DPoint& pt1, const A3DPoint& pt2)
{
    return pt1.dot(pt2);
}

double UnitAngle(const A3DPoint& pt1, const A3DPoint& pt2)
{
    double res  = pt1.dot(pt2);
    double len1 = pt1.mod();
    double len2 = pt2.mod();

    if (len1 > 0.0) res /= len1;
    if (len2 > 0.0) res /= len2;

    if (res >= 1.0) res = 0.0;
    else            res = acos(res);

    return res * 180.0 / M_PI;
}

double QuickAngle(const A3DPoint& pt1, const A3DPoint& pt2)
{
    return acos(pt1.dot(pt2)) * 180.0 / M_PI;
}

A3DPoint CrossProduct(const A3DPoint& pt1, const A3DPoint& pt2)
{
    A3DPoint pt;

    pt.x =  (pt1.y * pt2.z - pt2.y * pt1.z);
    pt.y = -(pt1.x * pt2.z - pt2.x * pt1.z);
    pt.z =  (pt1.x * pt2.y - pt2.x * pt1.y);

    return pt;
}

/*----------------------------------------------------------------------------------------------------*/

/*

  Line - Line intersection

  Line 1
  [x]   [a1]   [b1]
  [y] = [a2] + [b2].t1
  [z]   [a3]   [b3]

  Line 2
  [x]   [c1]   [d1]
  [y] = [c2] + [d2].t2
  [z]   [c3]   [d3]

  [a1]   [b1]      [c1]   [d1]
  [a2] + [b2].t1 = [c2] + [d2].t2
  [a3]   [b3]      [c3]   [d3]

  b1.t1 - d1.t2 = c1 - a1
  b2.t1 - d2.t2 = c2 - a2
  b3.t1 - d3.t2 = c3 - a3

        b2.(c1 - a1) - b1.(c2 - a2)
  t2 = -----------------------------
               b1.d2 - b2.d1

        b3.(c2 - a2) - b2.(c3 - a3)
  t2 = -----------------------------
               b2.d3 - b3.d2

        b3.(c1 - a1) - b1.(c3 - a3)
  t2 = -----------------------------
               b1.d3 - b3.d1


        d1.(d2 - a2) - d2.(c1 - a1)
  t1 = -----------------------------
               d1.b2 - d2.b1

        d1.(d3 - a3) - d3.(c1 - a1)

  t1 = -----------------------------
               d1.b3 - d3.b1

        d2.(d3 - a3) - d3.(c2 - a2)
  t1 = -----------------------------
               d2.b3 - d3.b2


  Line - Plane intersection

  Line
  [x]   [a1] + [b1]
  [y] = [a2] + [b2].t
  [z]   [a3] + [b3]

  Plane
  d1.(x - c1) + d2.(y - c2) + d3.(z - c3) = 0

  d1.(a1 + b1.t - c1) + d2.(a2 + b2.t - c2) + d3.(a3 + b3.t - c3) = 0


         d1.(a1 - c1) + d2.(a2 - c2) + d3.(a3 - c3)
  t = - --------------------------------------------
                  d1.b1 + d2.b2 + d3.b3


*/

bool A3DLine::Intersection(const A3DLine& line, double *t) const
{
    double div, tval = 0.0;
    uint_t   n = 0;

    /*
     *         d1.(d2 - a2) - d2.(c1 - a1)
     *   t1 = -----------------------------
     *                d1.b2 - d2.b1
     */
    div = line.Vector.x * Vector.y - line.Vector.y * Vector.x;
    if (div != 0.0) {
        tval += (line.Vector.x * (line.Vector.y - Start.y) - line.Vector.y * (line.Start.x - Start.x)) / div;
        n++;
    }

    /*
     *         d1.(d3 - a3) - d3.(c1 - a1)
     *   t1 = -----------------------------
     *                d1.b3 - d3.b1
     */
    div = line.Vector.x * Vector.z - line.Vector.z * Vector.x;
    if (div != 0.0) {
        tval += (line.Vector.x * (line.Vector.z - Start.z) - line.Vector.z * (line.Start.x - Start.x)) / div;
        n++;
    }

    /*
     *         d2.(d3 - a3) - d3.(c2 - a2)
     *   t1 = -----------------------------
     *                d2.b3 - d3.b2
     */
    div = line.Vector.y * Vector.z - line.Vector.z * Vector.y;
    if (div != 0.0) {
        tval += (line.Vector.y * (line.Vector.z - Start.z) - line.Vector.z * (line.Start.y - Start.y)) / div;
        n++;
    }

    if (n && t) *t = tval / (double)n;

    return (n > 0);
}

bool A3DLine::Intersection(const A3DLine& line, A3DPoint *pt) const
{
    double t1 = 0.0, t2 = 0.0;
    bool   f1, f2;

    f1 = Intersection(line, &t1);
    f2 = line.Intersection(*this, &t2);

    if (pt) {
        pt->x = pt->y = pt->z = 0.0;
        if (f1) *pt += Start      + t1 * Vector;
        if (f2) *pt += line.Start + t2 * line.Vector;
        if (f1 && f2) *pt *= .5;
    }

    return (f1 || f2);
}

bool A3DLine::Intersection(const A3DPlane& plane, double *t) const
{
    double div;
    bool   success = false;

    /*
     *          d1.(a1 - c1) + d2.(a2 - c2) + d3.(a3 - c3)
     *   t = - --------------------------------------------
     *                   d1.b1 + d2.b2 + d3.b3
     */
    div = plane.Normal.x * Vector.x + plane.Normal.y * Vector.y + plane.Normal.z * Vector.z;
    if (div != 0.0) {
        if (t) *t = -(plane.Normal.x * (Start.x - plane.Point.x) + plane.Normal.y * (Start.y - plane.Point.y) + plane.Normal.z * (Start.z - plane.Point.z)) / div;
        success = true;
    }

    return success;
}

bool A3DLine::Intersection(const A3DPlane& plane, A3DPoint *pt) const
{
    double t;
    bool   success = false;

    success = Intersection(plane, &t);
    if (success && pt) {
        *pt = Start + t * Vector;
#if 0
        printf("Intersection test %0.14le\n",
               (pt->x - plane.GetPoint().x) * plane.GetNormal().x +
               (pt->y - plane.GetPoint().y) * plane.GetNormal().y +
               (pt->z - plane.GetPoint().z) * plane.GetNormal().z);
#endif
    }

    return success;
}

bool A3DLine::Reflection(const A3DPlane& plane, double *t, A3DLine *rline, A3DLine *tline) const
{
    double t0;
    bool   success = false;

    if (t == NULL) t = &t0;

    if (Intersection(plane, t)) {
        /*
         *   [x]   [a1]    2.(d1.(a1 - c1) + d2.(a2 - c2) + d3.(a3 - c3))  [d1]
         *   [y] = [a2] - ------------------------------------------------.[d2]
         *   [z]   [a3]              d1.d1 + d2.d2 + d3.d3                 [d3]
         */

        A3DPoint pt  = Start + (*t) * Vector;
        A3DPoint pt1 = pt  - Vector;
        A3DPoint pt2 = pt1 - 2.0 * plane.DirDistance(pt1) * plane.Normal;

        if (rline) {
            rline->SetStart(pt);
            rline->SetVector(pt - pt2);
        }
        if (tline) {
            tline->SetStart(pt);
            tline->SetVector(Vector);
        }

        success = true;
    }

    return success;
}

A3DPoint A3DPlane::Reflection(const A3DPoint& pt) const
{
    /*
     *   [x]   [a1]    2.(d1.(a1 - c1) + d2.(a2 - c2) + d3.(a3 - c3))  [d1]
     *   [y] = [a2] - ------------------------------------------------.[d2]
     *   [z]   [a3]              d1.d1 + d2.d2 + d3.d3                 [d3]
     */

    return pt - 2.0 * DirDistance(pt) * Normal;
}

A3DLine A3DPlane::Reflection(const A3DLine& line) const
{
    /*
     *   [x]   [a1]    2.(d1.(a1 - c1) + d2.(a2 - c2) + d3.(a3 - c3))  [d1]
     *   [y] = [a2] - ------------------------------------------------.[d2]
     *   [z]   [a3]              d1.d1 + d2.d2 + d3.d3                 [d3]
     */

    return A3DLine(Reflection(line.Start), Reflection(line.Vector + Point) - Point);
}

A3DPlane A3DPlane::Reflection(const A3DPlane& plane) const
{
    /*
     *   [x]   [a1]    2.(d1.(a1 - c1) + d2.(a2 - c2) + d3.(a3 - c3))  [d1]
     *   [y] = [a2] - ------------------------------------------------.[d2]
     *   [z]   [a3]              d1.d1 + d2.d2 + d3.d3                 [d3]
     */

    return A3DPlane(Reflection(plane.Point), Reflection(plane.Normal + Point) - Point);
}

bool A3DPlane::IntersectionInPolygon(const A3DPoint& pt, const A3DPoint *vertices, uint_t nVertices) const
{
    A3DPoint pt1;
    bool     success = false;

    if (Intersection(A3DLine(pt, Normal), &pt1)) {
        success = PointInPolygon(pt1, vertices, nVertices);
    }

    return success;
}

bool A3DPlane::IntersectionInPolygon(const A3DLine& line, const A3DPoint *vertices, uint_t nVertices) const
{
    A3DPoint pt1;
    bool     success = false;

    if (Intersection(line, &pt1)) {
        success = PointInPolygon(pt1, vertices, nVertices);
    }

    return success;
}

bool A3DPlane::PointInPolygon(const A3DPoint& pt, const A3DPoint *vertices, uint_t nVertices) const
{
    A3DPoint vec1, vec2;
    double   angle = 0.0;
    uint_t     i;
    bool     res;

    for (i = 0; i < nVertices; i++) {
        vec1   = vertices[i] - pt;
        vec2   = vertices[(i + 1) % nVertices] - pt;
        angle += UnitAngle(vec1, vec2);
    }

    angle -= 360.0;
    res    = (fabs(angle) <= 1.0);

    //if (!res) printf("Point (%0.1lf, %0.1lf, %0.1lf) in polygon %0.5le (%u)\n", pt.x, pt.y, pt.z, angle, (uint_t)res);

    return res;
}

/*----------------------------------------------------------------------------------------------------*/

A3DPolarPoint::A3DPolarPoint(const A3DPoint& pt) : radius(0.0),
                                                   angle1(0.0),
                                                   angle2(0.0)
{
    operator = (pt);
}

A3DPolarPoint::operator A3DPoint() const
{
    A3DPoint pt;
    double sin1 = sin(angle1 * M_PI / 180.0);
    double cos1 = cos(angle1 * M_PI / 180.0);
    double sin2 = sin(angle2 * M_PI / 180.0);
    double cos2 = cos(angle2 * M_PI / 180.0);

    pt.x = radius * sin1 * cos2;
    pt.y = radius * cos1 * cos2;
    pt.z = radius * sin2;

    return pt;
}

const A3DPolarPoint& A3DPolarPoint::operator = (const A3DPoint& pt)
{
    radius = sqrt(pt.x * pt.x + pt.y * pt.y + pt.z * pt.z);

    if (radius > 0.0) {
        angle2 = asin(pt.z / radius) * 180.0 / M_PI;
        double cosval = radius * cos(angle2 * M_PI / 180.0);
        if (cosval != 0.0) {
            angle1 = atan2(pt.x / cosval, pt.y / cosval) * 180.0 / M_PI;
        }
    }

    return *this;
}

bool A3DPolarPoint::Interpolate(const A3DPolarPoint& target, double coeff, bool circular)
{
    bool updated = false;

    updated = ::Interpolate(radius, target.radius, coeff);
    if (circular) {
        updated = ::InterpolateAngle(angle1, target.angle1, coeff) || updated;
        updated = ::InterpolateAngle(angle2, target.angle2, coeff) || updated;
    }
    else {
        updated = ::Interpolate(angle1, target.angle1, coeff) || updated;
        updated = ::Interpolate(angle2, target.angle2, coeff) || updated;
    }

    return updated;
}

bool A3DPolarPoint::Interpolate2ndOrder(A3DPolarPoint& velocity, const A3DPolarPoint& target, double coeff, double speed, bool circular)
{
    bool updated = false;

    updated = ::Interpolate2ndOrder(radius, velocity.radius, target.radius, coeff, speed);
    if (circular) {
        updated = ::Interpolate2ndOrderAngle(angle1, velocity.angle1, target.angle1, coeff, speed) || updated;
        updated = ::Interpolate2ndOrderAngle(angle2, velocity.angle2, target.angle2, coeff, speed) || updated;
    }
    else {
        updated = ::Interpolate2ndOrder(angle1, velocity.angle1, target.angle1, coeff, speed) || updated;
        updated = ::Interpolate2ndOrder(angle2, velocity.angle2, target.angle2, coeff, speed) || updated;
    }

    return updated;
}

void A3DPolarPoint::LimitAngles()
{
    LimitAngle(angle1);
    LimitAngle(angle2);
}

/*----------------------------------------------------------------------------------------------------*/

A3DView::A3DView() : translation(),
                     transform(),
                     translation2(),
                     transform2(),
                     cx(0.0),
                     cy(0.0),
                     xscale(1.0),
                     yscale(1.0),
                     scale(1.0)
{
}

A3DView::A3DView(const A3DView& object)
{
    operator = (object);
}

A3DView& A3DView::operator = (const A3DView& object)
{
    translation = object.translation;
    transform = object.transform;
    translation2 = object.translation2;
    transform2 = object.transform2;
    cx = object.cx;
    cy = object.cy;
    xscale = object.xscale;
    yscale = object.yscale;
    scale = object.scale;

    return *this;
}

bool A3DView::Interpolate(const A3DView& target, double coeff, bool circular)
{
    bool updated = false;

    updated = ::Interpolate(translation.x, target.translation.x, coeff);
    updated = ::Interpolate(translation.y, target.translation.y, coeff)   || updated;
    updated = ::Interpolate(translation.z, target.translation.z, coeff)   || updated;
    updated = transform.Interpolate(target.transform, coeff, circular)    || updated;
    updated = ::Interpolate(translation2.x, target.translation2.x, coeff) || updated;
    updated = ::Interpolate(translation2.y, target.translation2.y, coeff) || updated;
    updated = ::Interpolate(translation2.z, target.translation2.z, coeff) || updated;
    updated = transform2.Interpolate(target.transform2, coeff, circular)  || updated;
    updated = ::Interpolate(xscale, target.xscale, coeff)                 || updated;
    updated = ::Interpolate(yscale, target.yscale, coeff)                 || updated;
    updated = ::Interpolate(scale,  target.scale,  coeff)                 || updated;
    updated = ::Interpolate(cx,     target.cx, coeff)                     || updated;
    updated = ::Interpolate(cy,     target.cy, coeff)                     || updated;

    return updated;
}

bool A3DView::Interpolate2ndOrder(A3DView& velocity, const A3DView& target, double coeff, double speed, bool circular)
{
    bool updated = false;

    updated = ::Interpolate2ndOrder(translation.x, velocity.translation.x, target.translation.x, coeff, speed);
    updated = ::Interpolate2ndOrder(translation.y, velocity.translation.y, target.translation.y, coeff, speed)    || updated;
    updated = ::Interpolate2ndOrder(translation.z, velocity.translation.z, target.translation.z, coeff, speed)    || updated;
    updated = transform.Interpolate2ndOrder(velocity.transform, target.transform, coeff, speed, circular)         || updated;
    updated = ::Interpolate2ndOrder(translation2.x, velocity.translation2.x, target.translation2.x, coeff, speed) || updated;
    updated = ::Interpolate2ndOrder(translation2.y, velocity.translation2.y, target.translation2.y, coeff, speed) || updated;
    updated = ::Interpolate2ndOrder(translation2.z, velocity.translation2.z, target.translation2.z, coeff, speed) || updated;
    updated = transform2.Interpolate2ndOrder(velocity.transform2, target.transform2, coeff, speed, circular)      || updated;
    updated = ::Interpolate2ndOrder(xscale, velocity.xscale, target.xscale, coeff, speed)                         || updated;
    updated = ::Interpolate2ndOrder(yscale, velocity.yscale, target.yscale, coeff, speed)                         || updated;
    updated = ::Interpolate2ndOrder(scale,  velocity.scale,  target.scale,  coeff, speed)                         || updated;
    updated = ::Interpolate2ndOrder(cx,     velocity.cx,     target.cx,     coeff, speed)                         || updated;
    updated = ::Interpolate2ndOrder(cy,     velocity.cy,     target.cy,     coeff, speed)                         || updated;

    return updated;
}

A3DPoint operator * (const A3DPoint& pt, const A3DView& view)
{
    A3DPoint res = pt;

    res   += view.translation2;
    res   *= view.transform2;
    res   += view.translation;
    res   *= view.transform;
    res.x *= view.xscale * view.scale;
    res.y *= view.yscale * view.scale;
    res.x += view.cx;
    res.y += view.cy;

    return res;
}

A3DPoint operator / (const A3DPoint& pt, const A3DView& view)
{
    A3DPoint res = pt;

    res.y -= view.cy;
    res.x -= view.cx;
    res.y /= view.yscale * view.scale;
    res.x /= view.xscale * view.scale;
    res   /= view.transform;
    res   -= view.translation;
    res   /= view.transform2;
    res   -= view.translation2;

    return res;
}
