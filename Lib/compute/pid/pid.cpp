#include "pid.hpp"

#include "ex_math.hpp"

PID::PID()
{
    _kp = _ki = _kd = _dt = _max = _min = _i_max = 0.0f;
}

PID::PID(float kp, float ki, float kd, float dt, float max, float min, float i_max)
{
    _kp = kp;
    _ki = ki;
    _kd = kd;
    _dt = dt;
    _max = max;
    _min = min;
    _i_max = i_max;
}

void PID::operator()(float kp, float ki, float kd, float dt, float max, float min, float i_max)
{
    _kp = kp;
    _ki = ki;
    _kd = kd;
    _dt = dt;
    _max = max;
    _min = min;
    _i_max = i_max;
}

float PID::update(float error, float nudge)
{
    _error = error;
    _nudge = nudge;

    // Proportional term
    _P = _kp * error;

    // Integral term
    // _integral += error * _dt;
    _update_I(error);

    // Derivative term
    // float derivative = (error - _prev_error) / _dt;
    _D = _kd * (error - _prev_error) / _dt;

    // Calculate output
    _output = _P + _I - _D + nudge;

    // Clamp output
    if (_output > _max)
        _output = _max;
    else if (_output < _min)
        _output = _min;

    // Save error to previous error
    _prev_error = error;

    return _output;
}

void PID::reset(void)
{
    _reset_I();
    _prev_error = 0.0f;
    _P = _I = _D = _error = _nudge = _output = 0.0f;
}

void PID::_update_I(float error)
{
    if ((_I > 0.0f && error < 0.0f) ||
        (_I < 0.0f && error > 0.0f))
    {
        _I += _ki * error * _dt * 1.4f;
    }
    else
    {
        _I += _ki * error * _dt;
    }
    _I = constraint_value(_I, -_i_max, _i_max);
}

void PID::_reset_I(void)
{
    _I = 0.0f;
}