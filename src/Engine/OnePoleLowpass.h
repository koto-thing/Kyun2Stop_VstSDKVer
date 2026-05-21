#pragma once

class OnePoleLowpass {
public:
    OnePoleLowpass();
    
    void SetCutoff(float cutoffFreq, float sampleRate);
    float Process(float input);
    void Reset();
    
private:
    float prevOutput;
    float alpha;
};