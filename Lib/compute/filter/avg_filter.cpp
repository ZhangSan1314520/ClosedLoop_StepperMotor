#include "ex_math.hpp"

#include "FreeRTOS.h"

bool AvgFilter::init(uint8_t win_size)
{
    if (win_size < 2)
        return false;

    _sum = 0.f;
    _size = win_size;
    _index = 0;
    _count = 0;

    pData = (float *)pvPortMalloc(sizeof(float) * _size);
    if (pData == NULL)
        return false;

    return true;
}

float AvgFilter::filter(float value)
{
    _count = _count < _size ? _count + 1 : _size;
    _sum = _sum - pData[_index] + value;

    pData[_index] = value;
    _index = (_index + 1) % _size;

    _max = _min = value;

    if (_count == _size)
    {
        for (int i = 0; i < _size; i++)
        {
            if (pData[i] > _max)
                _max = pData[i];
            if (pData[i] < _min)
                _min = pData[i];
        }
        _avg = (_sum - _max - _min) / (_size - 2);
    }
    else
    {
        _avg = _sum / _count;
    }

    return _avg;
}

void AvgFilter::clear(void)
{
    _index = 0;
    _count = 0;
    _sum = 0.f;
    _max = 0.f;
    _min = 0.f;
}
