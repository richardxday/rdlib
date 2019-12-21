#ifndef __INTERPOLATION__
#define __INTERPOLATION__

#include "misc.h"

class Interpolator {
public:
    // create object with specified time constant
    Interpolator(double tc) : coeff(calccoeff(tc)),
                              mul(1.0),
                              ticks(0) {
    }

    // calculate coeff for specified number of ticks (ms)
    double GetCoeff(uint32_t nticks) {
        if (nticks != ticks) {
            ticks = nticks;
            mul   = calcmul(ticks);
        }
        return mul;
    }

    // interpolate value to target using current rate (assumes same inter-sample time)
    bool Interpolate(double& value, double target, double limit = 1.0e-7) {
        return Interpolate(value, target, nticks, limit);
    }

    // interpolate value to target using specified inter-sample time
    bool Interpolate(double& value, double target, uint32_t nticks, double limit = 1.0e-7) {
        bool updated = false;

        if (value != target) {
            // move towards target
            value = target + (value - target) * GetCoeff(nticks);
            // ensure value doesn't go denormal (and slow down the processor in the process)
            value = fix_denormalized(value);
            // if value is close enough to the target, make the value the same as the target
            if (fabs(target - value) <= limit) value = target;
            updated = true;
        }

        return updated;
    }

    // interpolate angle, taking note of circular nature of the values
    bool InterpolateAngle(double& value, double target, double limit = 1.0e-7) {
        return InterpolateAngle(value, target, nticks, limit);
    }

    // interpolate angle, taking note of circular nature of the values
    bool InterpolateAngle(double& value, double target, uint32_t nticks, double limit = 1.0e-7) {
        double diff = value - target;

        LimitAngle(diff);

        value = target + diff * GetCoeff(nticks);
        value = fix_denormalized(value);
        if (fabs(target - value) <= limit) value = target;

        return (diff != 0.0);
    }

protected:
    double calccoeff(double tc) {
        return log(1.0 - 1.0 / tc);
    }

    double calcmul(uint32_t nticks) {
        return exp((double)nticks * coeff);
    }

protected:
    double coeff;
    double mul;
    uint32_t ticks;
};

#endif
