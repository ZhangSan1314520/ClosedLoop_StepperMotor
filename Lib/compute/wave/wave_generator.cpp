#include "wave_generator.hpp"

void WaveGenerator::_update_status(uint16_t delta_tick)
{
    if (_current_tick == 0)
    {
        _wave_type = _new_wave_type;
        _period_tick = _new_period_tick;
        _gain = _new_gain;
        _offset = _new_offset;
    }

    _current_tick = (_current_tick < _period_tick)
                        ? ((_current_tick + delta_tick) % _period_tick)
                        : 0;
}

float WaveGenerator::_generate_sine_wave(void)
{
    float ret_val;

    ret_val = _gain * fast_sin(_current_tick * M_2PI / _period_tick) + _offset;

    return ret_val;
}

float WaveGenerator::_generate_square_wave(void)
{
    float ret_val;

    if (_current_tick < (_period_tick / 2))
    {
        ret_val = -_gain + _offset;
    }
    else
    {
        ret_val = _gain + _offset;
    }

    return ret_val;
}

float WaveGenerator::_generate_triangle_wave(void)
{
    float ret_val;

    if (_current_tick < (_period_tick / 2))
    {
        ret_val = _gain * (4.0f * _current_tick / _period_tick - 1.0f) + _offset;
    }
    else
    {
        ret_val = _gain * (4.0f * (_period_tick - _current_tick) / _period_tick - 1.0f) + _offset;
    }

    return ret_val;
}

float WaveGenerator::_generate_sawtooth_wave(void)
{
    float ret_val;

    ret_val = _gain * (2.0f * _current_tick / _period_tick - 1.0f) + _offset;

    return ret_val;
}

float WaveGenerator::_generate_step_wave(void)
{
    if (_step_table == nullptr || _step_table_size == 0)
    {
        return 0.0f;
    }
    int index = _current_tick / (_period_tick / _step_table_size);

    return _gain * _step_table[index] + _offset;
}

float WaveGenerator::_generate_const_wave(void)
{
    return _gain;
}

float WaveGenerator::next(uint16_t delta_tick)
{
    float ret_val = 0.0f;

    _update_status(delta_tick);

    switch (_wave_type)
    {
    case WaveType::NONE:
        ret_val = 0.0f;
        break;
    case WaveType::SINE:
        ret_val = _generate_sine_wave();
        break;
    case WaveType::SQUARE:
        ret_val = _generate_square_wave();
        break;
    case WaveType::TRIANGLE:
        ret_val = _generate_triangle_wave();
        break;
    case WaveType::SAWTOOTH:
        ret_val = _generate_sawtooth_wave();
        break;
    case WaveType::STEP:
        ret_val = _generate_step_wave();
        break;
    case WaveType::CONST:
        ret_val = _generate_const_wave();
        break;
    default:
        ret_val = 0.0f;
        break;
    }

    return ret_val;
}