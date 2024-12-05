#pragma once

namespace mys_toolkit {

class TempSensor
{
    int temp_offset_ = 0;
    float temp_gain_ = 1.0;
    int16_t temperature_;
    uint16_t readTemp_();
public:
    TempSensor(int temp_offset = 0, float temp_gain = 1.0);
    void begin();
    void update();
    int16_t getTemp();
    void calibrate(int temp_offset, float temp_gain);
};

} // mys_toolkit
