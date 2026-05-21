#include "OnePoleLowpass.h"

#include <cmath>
#include <algorithm>
#include <numbers>

/**
 * コンストラクタ
 */
OnePoleLowpass::OnePoleLowpass()
    : prevOutput(0.0f), alpha(0.0f) {}

/**
 * カットオフ周波数とサンプリングレートを設定してフィルタの係数を計算する
 * @param cutoffFreq カットオフ周波数（Hz）
 * @param sampleRate サンプリングレート（Hz）
 */
void OnePoleLowpass::SetCutoff(float cutoffFreq, float sampleRate) {
    if (sampleRate <= 0.0f) {
        alpha = 1.0f;
        return;
    }
    
    const float coeff = -2.0f * std::numbers::pi_v<float> * cutoffFreq / sampleRate;
    const float y = 1.0f - std::expf(coeff);
    alpha = std::clamp(y, 0.0f, 1.0f);
}

/**
 * 1ポールローパスフィルタの処理
 * @param input 入力サンプル
 * @return フィルタ処理された出力サンプル
 */
float OnePoleLowpass::Process(float input) {
    const float output = prevOutput + alpha * (input - prevOutput);
    prevOutput = output;
    return output;
}

/**
 * フィルタの状態をリセット
 */
void OnePoleLowpass::Reset() {
    prevOutput = 0.0f;
    alpha = 1.0f;
}