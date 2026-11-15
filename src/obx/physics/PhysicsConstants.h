#pragma once

#include "bx/math.h"
#include <cmath>
namespace obx::physics {

static const bx::Vec3 NULL_VECTOR = {0, 0, 0};

static const double G = 6.67430e-20;        // km^3 / (kg * s^2)
static const double EARTH_MASS = 5.9722e24; // kg
static const double MU = 398600.4418;       // km^3/s^2
constexpr double RE = 6378.137;             // km, WGS84 equatorial
constexpr double J2 = 1.08262668e-3;        // j2 effect
constexpr double FLAT = 1.0 / 298.257223563;
constexpr double OMEGA_E = 7.2921159e-5; // rad/s, earth rotation

struct Force {
    bx::Vec3 vector; // newtons
};

// bx doesn't support double vector only float
struct V3 {
    double x, y, z;
};
inline bx::Vec3 toBx(const V3 &v) {
    return bx::Vec3{static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z)};
}
inline V3 fromBx(const bx::Vec3 &v) { return V3{v.x, v.y, v.z}; }
inline V3 operator+(V3 a, V3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
inline V3 operator*(V3 a, double s) { return {a.x * s, a.y * s, a.z * s}; }
inline double dot(V3 a, V3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline double norm(V3 a) { return std::sqrt(dot(a, a)); }

namespace { // anon wrapper
// exponential atmosphere, base altitude (km), base density (kg/m^3), scale height (km)
// derived from the U.S. Standard Atmosphere, 1976,
// stolen from: Fundamentals of Astrodynamics and Applications
// good read tbh (jk larp)

struct AtmLayer {
    double h0, rho0, H;
};
constexpr AtmLayer ATM[] = {
    {0, 1.225, 7.249},         {25, 3.899e-2, 6.349},    {30, 1.774e-2, 6.682},
    {40, 3.972e-3, 7.554},     {50, 1.057e-3, 8.382},    {60, 3.206e-4, 7.714},
    {70, 8.770e-5, 6.549},     {80, 1.905e-5, 5.799},    {90, 3.396e-6, 5.382},
    {100, 5.297e-7, 5.877},    {110, 9.661e-8, 7.263},   {120, 2.438e-8, 9.473},
    {130, 8.484e-9, 12.636},   {140, 3.845e-9, 16.149},  {150, 2.070e-9, 22.523},
    {180, 5.464e-10, 29.740},  {200, 2.789e-10, 37.105}, {250, 7.248e-11, 45.546},
    {300, 2.418e-11, 53.628},  {350, 9.158e-12, 53.298}, {400, 3.725e-12, 58.515},
    {450, 1.585e-12, 60.828},  {500, 6.967e-13, 63.822}, {600, 1.454e-13, 71.835},
    {700, 3.614e-14, 88.667},  {800, 1.170e-14, 124.64}, {900, 5.245e-15, 181.05},
    {1000, 3.019e-15, 268.00},
};

double density(double altKm) {
    if (altKm < 0.0)
        altKm = 0.0;
    const AtmLayer *L = &ATM[0];
    for (const auto &l : ATM) {
        if (altKm >= l.h0)
            L = &l;
        else
            break;
    }
    return L->rho0 * std::exp(-(altKm - L->h0) / L->H);
}

// stolen off of stack exchange
// approximate geodetic altitude: radius minus the ellipsoid surface radius at this latitude
double altitudeKm(const V3 &r) {
    const double rn = norm(r);
    const double sin2lat = (r.y * r.y) / (rn * rn);
    return rn - RE * (1.0 - FLAT * sin2lat);
}

static const double SPIN = +1.0;

// this is probably wrong too but who cares
V3 accel(const V3 &r, const V3 &v, double cdAOverM, bool useJ2, bool useDrag) {
    const double r2 = dot(r, r), rn = std::sqrt(r2);

    // two-body
    V3 a = r * (-MU / (r2 * rn));

    // J2 oblateness with Y as the pole
    if (useJ2) {
        const double y2r2 = (r.y * r.y) / r2;
        const double k = 1.5 * J2 * MU * RE * RE / (r2 * r2 * rn); // 1/r^5 times x, y or z
        a.x += k * r.x * (5.0 * y2r2 - 1.0);
        a.y += k * r.y * (5.0 * y2r2 - 3.0);
        a.z += k * r.z * (5.0 * y2r2 - 1.0);
    }

    // drag against the co-rotating atmosphere (spin about Y)
    if (useDrag && cdAOverM > 0.0) {
        const V3 vAtm{-SPIN * OMEGA_E * r.z, 0.0, SPIN * OMEGA_E * r.x}; // km/s
        const V3 vrel{v.x - vAtm.x, v.y - vAtm.y, v.z - vAtm.z};
        const double speed = norm(vrel);
        const double rho = density(altitudeKm(r));
        a = a + vrel * (-0.5 * rho * cdAOverM * 1000.0 * speed);
    }
    return a;
}

} // namespace
} // namespace obx::physics
