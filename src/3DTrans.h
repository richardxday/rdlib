#ifndef __3D_TRANSFORM__
#define __3D_TRANSFORM__

#include "misc.h"
#include "Matrix.h"

/*
 * A 3D positioning system
 *
 * Supports 3D transforms: rotations, translations and perspective
 *
 * Also provides calculation of plane and line intersections
 *
 */

class A3DPoint;			//< basic point in 3D space

/// 3D transformation class 
class A3DTransform {
public:
	// ixy is rotation in X-Y plane (around Z axis)
	// iyz is rotation in Y-Z plane (around X axis)
	// ixz is rotation in X-Z plane (around Y axis)
	// iper is perspective length (vanishing distance)
	// izoom is zoom factor
	// ihtrans is translation in X axis only (used for simulating left-eye, right-eye separation)
	A3DTransform(double ixy = 0.0, double iyz = 0.0, double ixz = 0.0, double iper = 0.0, double izoom = 1.0, double ihtrans = 0.0) {
		xy = ixy; yz = iyz; xz = ixz; per = iper; zoom = izoom; htrans = ihtrans;
	}
	// copy constructor
	A3DTransform(const A3DTransform& object) {
		xy = object.xy; yz = object.yz; xz = object.xz; per = object.per; zoom = object.zoom; htrans = object.htrans;
	}

	// assignment operator
	A3DTransform& operator = (const A3DTransform& object) {
		xy = object.xy; yz = object.yz; xz = object.xz; per = object.per; zoom = object.zoom; htrans = object.htrans;
		return *this;
	}

#define TRANSFORM_3D_OP(op)												\
	friend A3DTransform  operator op (const A3DTransform& object1, const A3DTransform& object2) { \
		return A3DTransform(object1.xy op object2.xy, object1.yz op object2.yz, object1.xz op object2.xz, object1.per, object1.zoom); \
	}																	\
	A3DTransform  operator op    (const A3DTransform& object) {			\
		return A3DTransform(xy op object.xy, yz op object.yz, xz op object.xz, per, zoom); \
	}																	\
	A3DTransform& operator op##= (const A3DTransform& object) {			\
		xy op##= object.xy; yz op##= object.yz; xz op##= object.xz;		\
		return *this;													\
	}

	// arithmetic operators on rotational angles ONLY!
	TRANSFORM_3D_OP(+);
	TRANSFORM_3D_OP(-);
	TRANSFORM_3D_OP(*);
	TRANSFORM_3D_OP(/);

	// apply rotation, horizontal translation, perspective and then zoom to point 
	// bWrap applies perspective
	void Transform(A3DPoint& a, bool bWarp = true) const;

	// apply inverse transforms in reverse order
	// bWrap removes perspective
	void InverseTransform(A3DPoint& a, bool bWarp = true) const;

	// exponential interpolation of parameters
	bool Interpolate(const A3DTransform& target, double coeff = .1, bool circular = true);

	// 2nd order interpolation (giving bounce) of parameters
	bool Interpolate2ndOrder(A3DTransform& velocity, const A3DTransform& target, double coeff = .1, double speed = 1.0, bool circular = true);

	// wrap rotations to angles to +/- 180 degrees
	void LimitAngles();
	
	double xy, yz, xz;
	double per, zoom;
	double htrans;
};

// 3D point object with x, y, z co-ordinates as doubles
class A3DPoint {
public:
	A3DPoint(double ix = 0.0, double iy = 0.0, double iz = 0.0) {x = ix; y = iy; z = iz;}
	A3DPoint(double vec[3]) {x = vec[0]; y = vec[1]; z = vec[2];}
	A3DPoint(const A3DPoint& val) {x = val.x; y = val.y; z = val.z;}

	const A3DPoint& operator = (const A3DPoint& a)   {x = a.x; y = a.y; z = a.z; return *this;}
	const A3DPoint& operator = (const double vec[3]) {x = vec[0]; y = vec[1]; z = vec[2]; return *this;}

	// simple addition
	friend A3DPoint operator + (const A3DPoint& a, const A3DPoint& b) {return A3DPoint(a.x + b.x, a.y + b.y, a.z + b.z);}

	// simple subtraction
	friend A3DPoint operator - (const A3DPoint& a, const A3DPoint& b) {return A3DPoint(a.x - b.x, a.y - b.y, a.z - b.z);}

	// negation
	A3DPoint operator - () const {return A3DPoint(-x, -y, -z);}

	// scalar multiply
	friend A3DPoint operator * (const A3DPoint& a, double X) {return A3DPoint(a.x * X, a.y * X, a.z * X);}
	friend A3DPoint operator * (double X, const A3DPoint& a) {return A3DPoint(a.x * X, a.y * X, a.z * X);}

	// multiply each co-ordinate by its partner
	friend A3DPoint operator * (const A3DPoint& a, const A3DPoint& b) {return A3DPoint(a.x * b.x, a.y * b.y, a.z * b.z);}

	// apply 3D transform
	friend A3DPoint operator * (const A3DPoint& a, const A3DTransform& b) {
		A3DPoint res = a;
		b.Transform(res);
		return res;
	}
	// scalar divide
	friend A3DPoint operator / (const A3DPoint& a, double X) {return A3DPoint(a.x / X, a.y / X, a.z / X);}

	// divide each co-ordinate by its partner
	friend A3DPoint operator / (const A3DPoint& a, const A3DPoint& b) {return A3DPoint(a.x / b.x, a.y / b.y, a.z / b.z);}

	// remove 3D transform
	friend A3DPoint operator / (const A3DPoint& a, const A3DTransform& b) {
		A3DPoint res = a;
		b.InverseTransform(res);
		return res;
	}

	friend int operator == (const A3DPoint& a, const A3DPoint& b) {return  ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));}
	friend int operator != (const A3DPoint& a, const A3DPoint& b) {return !((a.x == b.x) && (a.y == b.y) && (a.z == b.z));}
	friend int operator <  (const A3DPoint& a, const A3DPoint& b) {return  ((a.x <  b.x) && (a.y <  b.y) && (a.z <  b.z));}
	friend int operator <= (const A3DPoint& a, const A3DPoint& b) {return  ((a.x <= b.x) && (a.y <= b.y) && (a.z <= b.z));}
	friend int operator >  (const A3DPoint& a, const A3DPoint& b) {return  ((a.x >  b.x) && (a.y >  b.y) && (a.z >  b.z));}
	friend int operator >= (const A3DPoint& a, const A3DPoint& b) {return  ((a.x >= b.x) && (a.y >= b.y) && (a.z >= b.z));}

	// auto-increment operate on x co-ordinate only!
	A3DPoint  operator++ ()    {A3DPoint res = *this; x++; return res;}
	A3DPoint& operator++ (int) {x++; return *this;}
	A3DPoint  operator-- ()    {A3DPoint res = *this; x--; return res;}
	A3DPoint& operator-- (int) {x--; return *this;}

	// negation
	A3DPoint  operator- ()     {return A3DPoint(-x, -y, -z);}

	// more arithmetic
	A3DPoint& operator += (double X)   	   		  {x += X; return *this;}
	A3DPoint& operator += (const A3DPoint& a) 	  {x += a.x; y += a.y; z += a.z; return *this;}
	A3DPoint& operator -= (double X)   	   		  {x -= X; return *this;}
	A3DPoint& operator -= (const A3DPoint& a) 	  {x -= a.x; y -= a.y; z -= a.z; return *this;}
	A3DPoint& operator *= (double X)   	   		  {x *= X; y *= X; z *= X; return *this;}
	A3DPoint& operator *= (const A3DPoint& a) 	  {x *= a.x; y *= a.y; z *= a.z; return *this;}
	A3DPoint& operator *= (const A3DTransform& a) {a.Transform(*this); return *this;}
	A3DPoint& operator /= (double X)       		  {x /= X; y /= X; z /= X; return *this;}
	A3DPoint& operator /= (const A3DPoint& a)     {x /= a.x; y /= a.y; z /= a.z; return *this;}
	A3DPoint& operator /= (const A3DTransform& a) {a.InverseTransform(*this); return *this;}

	// mod and arg functions
	double mod() const {return sqrt(x * x + y * y + z * z);}
	void arg(double& xy, double& yz, double& xz) const {
		xy = atan2(y, x);
		yz = atan2(z, y);
		xz = atan2(z, x);
	}

	// normalize to a length of 1.0
	void normalize() {
		double len = mod();
		if (len > 0.0) {len = 1.0 / len; x *= len; y *= len; z *= len;} 
	}

	// dot product
	double dot(const A3DPoint& pt) const {return x * pt.x + y * pt.y + z * pt.z;}

	// convert to screen co-ordinates
	void ToScreenCoords(int cx, int cy, int& sx, int& sy, double scale = 1.0) const {
		sx = cx + (int)(x * scale + .5);
		sy = cy - (int)(y * scale + .5);
	}

	// convert from screen co-ordinates
	void FromScreenCoords(int cx, int cy, int sx, int sy, double scale = 1.0) {
		x = (double)(sx - cx) / scale;
		y = (double)(cy - sy) / scale;
	}

	// rotation around axes
	void RotateXY(double angle) {rotate(x, y, angle);}
	void RotateYZ(double angle) {rotate(y, z, angle);}
	void RotateXZ(double angle) {rotate(x, z, angle);}

	// apply perspective
	void ApplyPerspective(double per) {
		if ((per != 0.0) && (z != -per)) {
			double mult = per / (per + z);
			x *= mult;
			y *= mult;
		}
	}

	// remove perspective
	void RemovePerspective(double per) {
		if ((per != 0.0) && (z != -per)) {
			double mult = (per + z) / per;
			x *= mult;
			y *= mult;
		}
	}

	// exponential interpolation of co-ordinates
	bool Interpolate(const A3DPoint& target, double coeff = .1);

	// 2nd order interpolation of co-ordinates
	bool Interpolate2ndOrder(A3DPoint& velocity, const A3DPoint& target, double coeff = .1, double speed = 1.0);

	// return a pointer to the start of the co-ordinates
	operator double *() {return &x;}
	operator const double *() const {return &x;}

	double x, y, z;

protected:
	void rotate(double& x, double& y, double ang) const {
		if (ang != 0.0) {
			ang *= M_PI / 180.0;
			double x1 = x * cos(ang) - y * sin(ang);
			double y1 = x * sin(ang) + y * cos(ang);
			x = x1; y = y1;
		}
	}
};

// 3D homogenous matrix object
class A3DMatrix {
public:
	A3DMatrix() {Identity();}
	A3DMatrix(double x, double y, double z) {CalcForward(x, y, z);}
	A3DMatrix(const A3DMatrix& mat) {MatrixCopy(*this, mat, 4);}

	A3DMatrix& Identity() {MatrixIdentity(*this, 4); return *this;}

	enum {
		ORDER_XYZ = 0,
		ORDER_XZY,
		ORDER_YXZ,
		ORDER_YZX,
		ORDER_ZXY,
		ORDER_ZYX,
	};

	// calculate rotational matracies for either forward or reverse rotations in 3D space
	A3DMatrix& CalcForward(double x, double y, double z) {Identity(); Rotate( x,  y,  z, ORDER_XYZ); return *this;}
	A3DMatrix& CalcInverse(double x, double y, double z) {Identity(); Rotate(-x, -y, -z, ORDER_ZYX); return *this;}

	A3DMatrix& operator = (const A3DMatrix& mat) {MatrixCopy(*this, mat, 4); return *this;}

	friend A3DPoint  operator * (const A3DPoint& vec, const A3DMatrix& mat);
	friend A3DPoint  operator * (const A3DMatrix& mat, const A3DPoint& vec);
	friend A3DMatrix operator * (const A3DMatrix& mat1, const A3DMatrix& mat2);

	A3DMatrix& operator *= (const A3DMatrix& mat);
	A3DMatrix& operator *= (double val);
	A3DMatrix& operator /= (double val);
	A3DMatrix& operator += (const A3DPoint& pt);
	A3DMatrix& operator -= (const A3DPoint& pt);

	A3DMatrix  Transpose() const {A3DMatrix mat; MatrixTranspose(mat, *this, 4); return mat;}
	A3DMatrix& RotateX(double ang) {MatrixRotate(*this, *this, 1, 2, ang, 4); return *this;}
	A3DMatrix& RotateY(double ang) {MatrixRotate(*this, *this, 0, 2, ang, 4); return *this;}
	A3DMatrix& RotateZ(double ang) {MatrixRotate(*this, *this, 0, 1, ang, 4); return *this;}

	A3DMatrix& Rotate(double x, double y, double z, uint_t order = ORDER_XYZ);
	A3DMatrix& Move(const A3DPoint& pt) {return operator += (pt);}
	A3DMatrix& SetPerspective(double d);
	A3DMatrix& Normalise();

	void RotateXY(double& x, double& y, double z) const {
		double x1 = x * a11 + y * a12 + z * a13;
		double y1 = x * a21 + y * a22 + z * a23;
		x = x1; y = y1;
	}

	// return a pointer to the start of the matrix
	operator double *() {return &a11;}
	operator const double *() const {return &a11;}

	double a11, a12, a13, a14;
	double a21, a22, a23, a24;
	double a31, a32, a33, a34;
	double a41, a42, a43, a44;
};

double DotProduct(const A3DPoint& pt1, const A3DPoint& pt2);
double UnitAngle(const A3DPoint& pt1, const A3DPoint& pt2);
double QuickAngle(const A3DPoint& pt1, const A3DPoint& pt2);
A3DPoint CrossProduct(const A3DPoint& pt1, const A3DPoint& pt2);

class A3DPlane;
// 3D line/vector object specified by starting position and vector
class A3DLine {
public:
	A3DLine() {}
	A3DLine(const A3DLine& object) : Start(object.Start),
									 Vector(object.Vector) {
		Vector.normalize();
	}
	A3DLine(const A3DPoint& start, const A3DPoint& vector) : Start(start),
															 Vector(vector) {
		Vector.normalize();
	}
	
	const A3DLine& operator = (const A3DLine& object) {Start = object.Start; Vector = object.Vector; Vector.normalize(); return *this;}

	// return point at time t assuming Vector is velocity
	A3DPoint Point(double t)       const {return Start + t * Vector;}
	A3DPoint operator [](double t) const {return Start + t * Vector;}

	// find intersection of this line and specified line
	bool Intersection(const A3DLine& line, double *t) const;
	bool Intersection(const A3DLine& line, A3DPoint *pt) const;

	// find intersection of this line and specified plane
	bool Intersection(const A3DPlane& plane, double *t) const;
	bool Intersection(const A3DPlane& plane, A3DPoint *pt) const;
	
	// reflection of line through plane
	bool Reflection(const A3DPlane& plane, double *t = NULL, A3DLine *rline = NULL, A3DLine *tline = NULL) const;

	void SetStart(const A3DPoint& pt) {Start = pt;}
	void SetVector(const A3DPoint& vec) {Vector = vec; Vector.normalize();}
	const A3DPoint& GetStart()  const {return Start;}
	const A3DPoint& GetVector() const {return Vector;}

	friend class A3DPlane;

protected:
	A3DPoint Start;
	A3DPoint Vector;
};

// 3D plane specified by point and normal
class A3DPlane {
public:
	A3DPlane() {}
	A3DPlane(const A3DPlane& object) : Point(object.Point),
									   Normal(object.Normal) {
		Normal.normalize();
	}
	A3DPlane(const A3DPoint& point, const A3DPoint& normal) : Point(point),
															  Normal(normal) {
		Normal.normalize();
	}
	
	const A3DPlane& operator = (const A3DPlane& object) {Point = object.Point; Normal = object.Normal; Normal.normalize(); return *this;}

	// find intersection
	bool Intersection(const A3DLine& line, A3DPoint *pt2) const {return line.Intersection(*this, pt2);}
	bool Intersection(const A3DLine& line, double *t)     const {return line.Intersection(*this, t);}
	bool Intersection(const A3DPoint& pt, A3DPoint *pt2)  const {return A3DLine(pt, Normal).Intersection(*this, pt2);}
	bool Intersection(const A3DPoint& pt, double *t)      const {return A3DLine(pt, Normal).Intersection(*this, t);}

	bool Reflection(const A3DLine& line, double *t = NULL, A3DLine *rline = NULL, A3DLine *tline = NULL) const {return line.Reflection(*this, t, rline, tline);}

	A3DPoint Reflection(const A3DPoint& pt) const;
	A3DLine  Reflection(const A3DLine& line) const;
	A3DPlane Reflection(const A3DPlane& plane) const;

	void SetPoint(const A3DPoint& pt)   {Point  = pt;}
	void SetNormal(const A3DPoint& nor) {Normal = nor; Normal.normalize();}

	const A3DPoint& GetPoint()  const {return Point;}
	const A3DPoint& GetNormal() const {return Normal;}

	double DirDistance(const A3DPoint& pt) const {return Normal.dot(pt - Point);}
	double Distance(const A3DPoint& pt) const {return fabs(DirDistance(pt));}

	bool IntersectionInPolygon(const A3DPoint& pt, const A3DPoint *vertices, uint_t nVertices) const;
	bool IntersectionInPolygon(const A3DLine& line, const A3DPoint *vertices, uint_t nVertices) const;
	bool PointInPolygon(const A3DPoint& pt, const A3DPoint *vertices, uint_t nVertices) const;

	friend class A3DLine;

protected:
	A3DPoint Point;
	A3DPoint Normal;
};

class A3DPolarPoint {
public:
	A3DPolarPoint(double iradius = 0.0, double iangle1 = 0.0, double iangle2 = 0.0) {
		radius = iradius; angle1 = iangle1; angle2 = iangle2;
	}
	A3DPolarPoint(const A3DPolarPoint& pt) {
		radius = pt.radius; angle1 = pt.angle1; angle2 = pt.angle2;
	}
	A3DPolarPoint(const A3DPoint& pt);

	const A3DPolarPoint& operator = (const A3DPolarPoint& pt) {
		radius = pt.radius; angle1 = pt.angle1; angle2 = pt.angle2;
		return *this;
	}
	const A3DPolarPoint& operator = (const A3DPoint& pt);

	friend A3DPolarPoint operator + (const A3DPolarPoint& a, const A3DPolarPoint& b) {return A3DPolarPoint(a.radius + b.radius, a.angle1 + b.angle1, a.angle2 + b.angle2);}
	friend A3DPolarPoint operator - (const A3DPolarPoint& a, const A3DPolarPoint& b) {return A3DPolarPoint(a.radius - b.radius, a.angle1 - b.angle1, a.angle2 - b.angle2);}
	A3DPolarPoint operator - () const {return A3DPolarPoint(radius, -angle1, -angle2);}
	friend A3DPolarPoint operator * (const A3DPolarPoint& a, double X) {return A3DPolarPoint(a.radius * X, a.angle1, a.angle2);}
	friend A3DPolarPoint operator * (double X, const A3DPolarPoint& a) {return A3DPolarPoint(a.radius * X, a.angle1, a.angle2);}
	friend A3DPolarPoint operator * (const A3DPolarPoint& a, const A3DPolarPoint& b) {return A3DPolarPoint(a.radius * b.radius, a.angle1 + b.angle1, a.angle2 + b.angle2);}
	friend A3DPolarPoint operator / (const A3DPolarPoint& a, double X) {return A3DPolarPoint(a.radius / X, a.angle1, a.angle2);}
	friend A3DPolarPoint operator / (const A3DPolarPoint& a, const A3DPolarPoint& b) {return A3DPolarPoint(a.radius / b.radius, a.angle1 - b.angle1, a.angle2 - b.angle2);}

	A3DPolarPoint& operator += (const A3DPolarPoint& b) {radius += b.radius; angle1 += b.angle1; angle2 += b.angle2; return *this;}
	A3DPolarPoint& operator -= (const A3DPolarPoint& b) {radius -= b.radius; angle1 -= b.angle1; angle2 -= b.angle2; return *this;}
	A3DPolarPoint& operator *= (double X) {radius *= X; return *this;}
	A3DPolarPoint& operator *= (const A3DPolarPoint& b) {radius *= b.radius; angle1 += b.angle1; angle2 += b.angle2; return *this;}
	A3DPolarPoint& operator /= (double X) {radius /= X; return *this;}
	A3DPolarPoint& operator /= (const A3DPolarPoint& b) {radius /= b.radius; angle1 -= b.angle1; angle2 -= b.angle2; return *this;}

	operator A3DPoint() const;

	bool Interpolate(const A3DPolarPoint& target, double coeff = .1, bool circular = true);
	bool Interpolate2ndOrder(A3DPolarPoint& velocity, const A3DPolarPoint& target, double coeff = .1, double speed = 1.0, bool circuler = true);

	void LimitAngles();
	
	double radius, angle1, angle2;
};

class A3DView {
public:
	A3DView();
	A3DView(const A3DView& object);

	A3DView& operator = (const A3DView& object);

	bool Interpolate(const A3DView& target, double coeff = .1, bool circular = true);
	bool Interpolate2ndOrder(A3DView& velocity, const A3DView& target, double coeff = .1, double speed = 1.0, bool circular = true);

	void LimitAngles() {transform.LimitAngles(); transform2.LimitAngles();}
	
	A3DPoint     translation;
	A3DTransform transform;
	A3DPoint     translation2;
	A3DTransform transform2;

	double		 cx, cy;
	double		 xscale, yscale;
	double		 scale;
};

A3DPoint operator * (const A3DPoint& pt, const A3DView& view);
A3DPoint operator / (const A3DPoint& pt, const A3DView& view);

#endif
