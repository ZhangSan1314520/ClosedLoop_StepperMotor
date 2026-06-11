#pragma once

typedef struct
{
    float P;
    float I;
    float D;
    float error;
    float nudge;
    float output;
} PID_Info;

typedef struct
{
    float kp;
    float ki;
    float kd;
    float dt;
    float max;
    float min;
    float i_max;
} PID_Param;

class PID
{
public:
    PID(void);
    PID(float kp, float ki, float kd, float dt, float max, float min, float i_max);

    void operator()(float kp, float ki, float kd, float dt, float max, float min, float i_max);

    float update(float error, float nudge = 0.0f);
    void reset(void);

    void set_kp(float kp) { _kp = kp; }
    void set_ki(float ki) { _ki = ki; }
    void set_kd(float kd) { _kd = kd; }
    void set_dt(float dt) { _dt = dt; }
    void set_max(float max) { _max = max; }
    void set_min(float min) { _min = min; }
    void set_i_max(float i_max) { _i_max = i_max; }

    float get_kp(void) { return _kp; }
    float get_ki(void) { return _ki; }
    float get_kd(void) { return _kd; }
    float get_dt(void) { return _dt; }
    float get_max(void) { return _max; }
    float get_min(void) { return _min; }
    float get_i_max(void) { return _i_max; }

    float *get_kp_ptr(void) { return &_kp; }
    float *get_ki_ptr(void) { return &_ki; }
    float *get_kd_ptr(void) { return &_kd; }
    float *get_dt_ptr(void) { return &_dt; }
    float *get_max_ptr(void) { return &_max; }
    float *get_min_ptr(void) { return &_min; }
    float *get_i_max_ptr(void) { return &_i_max; }

    float get_P(void) { return _P; }
    float get_I(void) { return _I; }
    float get_D(void) { return _D; }
    float get_error(void) { return _error; }
    float get_nudge(void) { return _nudge; }
    float get_output(void) { return _output; }

    PID_Info get_pid_info(void) { return {_P, _I, _D, _error, _nudge, _output}; }
    PID_Param get_pid_param(void) { return {_kp, _ki, _kd, _dt, _max, _min, _i_max}; }

private:
    float _kp, _ki, _kd, _dt, _max, _min, _i_max;
    float _prev_error = 0;
    float _P, _I, _D, _error, _nudge, _output;

    void _update_I(float error);
    void _reset_I(void);
};
