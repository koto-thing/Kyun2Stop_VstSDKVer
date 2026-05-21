#include "DelayLine.h"

#include <bit>
#include <cmath>
#include <algorithm>

/**
 * コンストラクタ
 * @param maxSec 最大遅延時間（秒）
 * @param sampleRate サンプリングレート（Hz）
 */
DelayLine::DelayLine(float maxSec, float sampleRate) {
    auto size_f = std::ceil(maxSec * sampleRate);
    auto size = static_cast<size_t>(size_f);
    auto power_of_two_size = std::bit_ceil(size ? size : 1u);

    data.resize(power_of_two_size);
    mask = power_of_two_size - 1;
    std::fill(data.begin(), data.end(), 0.0f);
}

/**
 * バッファをリセット
 */
void DelayLine::Reset() {
    std::fill(data.begin(), data.end(), 0.0f);
}

/**
 * 新しいサンプルをバッファに追加する
 * @param index 書き込む位置のインデックス
 * @param value 書き込むサンプルの値
 */
void DelayLine::Write(size_t index, float value) {
    data[index & mask] = value;
}

/**
 * 指定した位置のサンプルを4点エルミート補間で読み取る
 * @param index 読み取る位置のインデックス（小数点以下も指定可能）
 * @return 読み取ったサンプルの値
 */
float DelayLine::Read(double index) const {
    // 整数部と小数部
    const size_t idx_i = static_cast<size_t>(std::floor(index));
    const float frac = static_cast<float>(index - static_cast<double>(idx_i));
    
    // 4点のサンプルを取得
    const size_t p0 = (idx_i - 1) & mask;
    const size_t p1 = idx_i & mask;
    const size_t p2 = (idx_i + 1) & mask;
    const size_t p3 = (idx_i + 2) & mask;
    
    const float s0 = data[p0];
    const float s1 = data[p1];
    const float s2 = data[p2];
    const float s3 = data[p3];
    
    // 4点エルミート補間
    const float c0 = s1;
    const float c1 = 0.5f * (s2 - s0);
    const float c2 = s0 - 2.5f * s1 + 2.0f * s2 - 0.5f * s3;
    const float c3 = 0.5f * (s3 - s0) + 1.5f * (s1 - s2);

    return ((c3 * frac + c2) * frac + c1) * frac + c0;
}