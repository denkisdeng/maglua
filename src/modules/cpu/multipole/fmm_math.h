// Jason Mercer 2012
// Function to help with Fast Multipole Method

#include <complex>
#include <vector>
#include "plm.h"

class monopole
{
public:
	monopole(double _x=0, double _y=0, double _z=0, double _q=0) : x(_x), y(_y), z(_z), q(_q) {calcSpherical();}
	monopole(const monopole& p) : x(p.x), y(p.y), z(p.z), q(p.q) {calcSpherical();}

	void calcSpherical();
	double x, y, z, q;
	double t, p, r;

	monopole&  operator+=(const monopole& rhs);
	monopole&  operator-=(const monopole& rhs);
	monopole&  operator*=(const double rhs);

	const monopole operator+(const monopole& b) const {monopole c(*this); c+=b; return c;}
	const monopole operator-(const monopole& b) const	{monopole c(*this); c-=b; return c;}
	const monopole operator*(const double b) const	{monopole c(*this); c*=b; return c;}
	const monopole operator/(const double b) const	{monopole c(*this); c*=(1.0/b); return c;}
	const monopole operator-() const {monopole c(*this); c*=-1.0; return c;}

	void makeUnit() {if(r != 0) {x/=r; y/=r; z/=r; r=1;} else {x=1; calcSpherical();}}
};


class tensor_transformation
{
public:
    tensor_transformation(double x, double y, double z, int degree);
    ~tensor_transformation();

    void apply(const std::complex<double>* x, std::complex<double>* b) const;

    double rx, ry, rz;
    int degree;
    std::complex<double>* t;
};

std::complex<double> Ylm(const int l, const int m, const double theta, const double phi);

int tensor_element_count(const int order);

// Journal of Computational Physics 227 (2008) 1836–1862
std::complex<double> Inner(const monopole& r, int n, int l);
std::complex<double> Outter(const monopole& r, int n, int l);


std::complex<double>* i2i_trans_mat(const int max_order, const monopole& d);
std::complex<double>* o2o_trans_mat(const int max_order, const monopole& d);

void tensor_mat_mul(const std::complex<double>* A, const std::complex<double>* x, std::complex<double>* b, int max_order);
