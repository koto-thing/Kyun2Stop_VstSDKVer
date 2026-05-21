#include "TapeStopEngine.h"

#include <cmath>
#include <algorithm>

TapeStopEngine::TapeStopEngine(float sampleRate, float maxSeconds, std::size_t channels)
    : sampleRate(sampleRate),
      writePos(0),
      readPos(0.0),
      phase(1.0),
      currentSpeed(1.0),
      crossFadeGain(1.0f)
{
    buffers.reserve(channels);
    filters.reserve(channels);
    for (std::size_t i = 0; i < channels; ++i) {
        buffers.emplace_back(maxSeconds, sampleRate);
        filters.emplace_back();
    }
}

void TapeStopEngine::Reset() {
    for (auto& buffer : buffers) 
        buffer.Reset();
    
    for (auto& filter : filters)
        filter.Reset();
    
    writePos = 0;
    readPos = 0.0;
    phase = 1.0;
    currentSpeed = 1.0;
    crossFadeGain = 1.0f;
}

void TapeStopEngine::Process(const float* input,
                             float* output,
                             std::size_t channels,
                             bool trigger,
                             float stop_time_sec,
                             float start_time_sec,
                             TapeCurve curve_type,
                             bool use_sync,
                             SyncBeat sync_beat,
                             double bpm,
                             bool enable_filter)
{
    // 停止時間の計算
    float actual_stop_time = stop_time_sec;
    if (use_sync) {
        double current_bpm = (bpm > 0.0) ? bpm : 120.0;
        double beats = 1.0;
        switch (sync_beat) {
            case SyncBeat::Eight:   beats = 0.5; break;
            case SyncBeat::Quarter: beats = 1.0; break;
            case SyncBeat::Half:    beats = 2.0; break;
            case SyncBeat::OneBar:  beats = 4.0; break;
            case SyncBeat::TwoBars: beats = 8.0; break;
        }
        actual_stop_time = static_cast<float>((60.0 / current_bpm) * beats);
    }

    // フェーズの更新に必要なステップサイズを計算
    const double sr = std::max(1e-12, static_cast<double>(sampleRate));
    const double stop_step = (actual_stop_time > 0.0f) ? (1.0 / (static_cast<double>(actual_stop_time) * sr)) : 1.0;
    const double start_step = (start_time_sec > 0.0f) ? (1.0 / (static_cast<double>(start_time_sec) * sr)) : 1.0;
    const float xfade_step = 1.0f / (0.1f * sampleRate);

    // トリガーに応じてフェーズとクロスフェードゲインを更新
    if (trigger) {
        phase -= stop_step;
        if (phase < 0.0) phase = 0.0;
        crossFadeGain = 0.0f;
    } else {
        if (phase < 1.0) {
            phase += start_step;
            if (phase > 1.0) phase = 1.0;
            crossFadeGain = 0.0f;
        } else {
            if (crossFadeGain < 1.0f) {
                crossFadeGain += xfade_step;
                if (crossFadeGain >= 1.0f) {
                    crossFadeGain = 1.0f;
                    // クロスフェード完了後、読み込み位置を現在の書き込み位置にリセットしてテープ音とリアルタイム音を完全に切り替える
                    readPos = static_cast<double>(writePos);
                }
            }
        }
    }

    // フェーズとカーブから現在の速度を計算
    const double t = phase;
    switch (curve_type) {
        case TapeCurve::Linear:
            currentSpeed = t;
            break;
        case TapeCurve::Smooth:
            currentSpeed = t * t * (3.0 - 2.0 * t);
            break;
        case TapeCurve::SlowStart:
            currentSpeed = 1.0 - std::pow(1.0 - t, 2);
            break;
        case TapeCurve::QuickCut:
            currentSpeed = std::pow(t, 3);
            break;
        default:
            currentSpeed = t;
            break;
    }

    // フィルタのカットオフ周波数を現在の速度に応じて更新
    if (enable_filter) {
        const float min_cut = 200.0f;
        const float max_cut = 20000.0f;
        // 速度が0のときmin_cut、速度が1のときmax_cutになるように指数関数的に変化させる
        const float cutoff = min_cut * std::pow(max_cut / min_cut, static_cast<float>(currentSpeed));
        for (auto& f : filters) {
            f.SetCutoff(cutoff, sampleRate);
        }
    }

    // 各チャンネルの処理
    const std::size_t channelCount = std::min(channels, buffers.size());
    for (std::size_t ch = 0; ch < channelCount; ++ch) {
        const float in_sample = input[ch];
        // 書き込み
        buffers[ch].Write(writePos, in_sample);
        // 読み込み
        float tape_sound = buffers[ch].Read(readPos);
        // フィルタ処理
        if (enable_filter) tape_sound = filters[ch].Process(tape_sound);
        // クロスフェード
        output[ch] = tape_sound * (1.0f - crossFadeGain) + in_sample * crossFadeGain;
    }

    // 書き込み位置と読み込み位置の更新
    writePos = writePos + 1; // サンプル単位で書き込み位置を進める
    readPos += currentSpeed;
}