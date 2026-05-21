#pragma once
#include <vector>

#include "DelayLine.h"
#include "OnePoleLowpass.h"
#include "../Kyun2StopParams.h"

class TapeStopEngine {
public:
    TapeStopEngine(float sampleRate, float maxSec, size_t channels);
    
    void Reset();
    void Process(const float* input,
        float* output,
        std::size_t channels,
        bool trigger,
        float stop_time_sec,
        float start_time_sec,
        TapeCurve curve_type,
        bool use_sync,
        SyncBeat sync_beat,
        double bpm,
        bool enable_filter
    );
    
    
private:
    std::vector<DelayLine> buffers;      // チャンネルごとの遅延バッファ
    std::vector<OnePoleLowpass> filters; // チャンネルごとのフィルタ
    float sampleRate;                    // サンプルレート
    
    size_t writePos;                     // 書き込み位置
    double readPos;                      // 読み込み位置
    
    double phase;                        // 1.0 -> 0.0の進行度
    double currentSpeed;                 // phaseとcurveから計算された実際の速度
    float crossFadeGain;                 // テープ音とリアルタイム音のクロスフェードゲイン
};
