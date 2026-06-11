#pragma once

#include "ex_math.hpp"
#include "fast_sin.h"

enum class WaveType
{
    NONE, //无波形
    SINE, //正弦波
    SQUARE, //方波
    TRIANGLE, //三角波
    SAWTOOTH, //锯齿波
    STEP, //步波
    CONST, //常波
};

class WaveGenerator
{
public:
    WaveGenerator()
    {
        _wave_type = WaveType::NONE;
        _period_tick = 1000;
        _gain = 1.0f;
        _offset = 0.0f;

        _new_wave_type = WaveType::NONE;
        _new_period_tick = 1000;
        _new_gain = 1.0f;
        _new_offset = 0.0f;

        _current_tick = 0;
    }

    WaveGenerator(WaveType type, uint16_t period_tick = 1000, float gain = 1.0f, float offset = 0.0f)
    {
        _wave_type = type;
        _period_tick = period_tick;
        _gain = gain;
        _offset = offset;

        _new_wave_type = type;
        _new_period_tick = period_tick;
        _new_gain = gain;
        _new_offset = offset;

        _current_tick = 0;
    }

    void change_wave_type(WaveType type) { _new_wave_type = type; }

    void reset_tick(void) { _current_tick = 0; }

    void set_period_tick(uint16_t period_tick) { _new_period_tick = period_tick; }
    void set_gain(float gain) { _new_gain = gain; }
    void set_offset(float offset) { _new_offset = offset; }

    uint16_t get_current_tick(void) { return _current_tick; }
    uint16_t get_period_tick(void) { return _new_period_tick; }
    float get_gain(void) { return _new_gain; }
    float get_offset(void) { return _new_offset; }

    void add(uint16_t delta_tick = 5) { _update_status(delta_tick); }
    float next(uint16_t delta_tick = 5);

    void config_step(const float *step_table, uint16_t step_table_size)
    {
        _step_table = step_table;
        _step_table_size = step_table_size;
    }

protected:
    uint16_t _current_tick;
    uint16_t _period_tick;
    float _gain;
    float _offset;

    uint16_t _new_period_tick;
    float _new_gain;
    float _new_offset;

private:
    WaveType _wave_type;
    WaveType _new_wave_type;

    const float *_step_table = NULL;
    uint16_t _step_table_size = 0;

    void _update_status(uint16_t delta_tick);

    float _generate_sine_wave(void);
    float _generate_square_wave(void);
    float _generate_triangle_wave(void);
    float _generate_sawtooth_wave(void);
    float _generate_step_wave(void);
    float _generate_const_wave(void);
};