#pragma once
#include <vector>

class DelayLine {
public:
    DelayLine(float maxSec, float sampleRate);
    
    void Reset();
    void Write(size_t index, float value);
    float Read(double index) const;
    
private:
    std::vector<float> data;
    size_t mask;
};
