#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"

#include <algorithm>
#include <cmath>

#include "Kyun2StopFuid.h"
#include "Kyun2StopProcessor.h"
#include "Kyun2StopDefine.h"

namespace Steinberg {
namespace Vst {
	/**
	 * コンストラクタ 
	 */
	Kyun2StopProcessor::Kyun2StopProcessor()
		: tapeStopEngine(nullptr) {
		// コントローラーのFUIDを設定
		setControllerClass(Kyun2StopControllerFuid);
	}
	
	/**
	 * クラスの初期化を行う関数
	 * @param [in] context コンテキスト
	 */
	tresult PLUGIN_API Kyun2StopProcessor::initialize(FUnknown* context) {
		// 基底クラスの初期化を実行
		tresult result = AudioEffect::initialize(context);
		if (result == kResultTrue) {
			addAudioInput(STR16("AudioInput"), SpeakerArr::kStereo);
			addAudioOutput(STR16("AudioOutput"), SpeakerArr::kStereo);
		}

		return result;
	}

	tresult PLUGIN_API Kyun2StopProcessor::setupProcessing(ProcessSetup& newSetup) {
		tresult result = AudioEffect::setupProcessing(newSetup);
		if (result != kResultTrue)
			return result;

		// Engine is stereo-out based; mono input is duplicated to R in process()
		constexpr float kMaxDelaySeconds = 8.0f;
		tapeStopEngine = std::make_unique<TapeStopEngine>(static_cast<float>(newSetup.sampleRate), kMaxDelaySeconds, 2);
		tapeStopEngine->Reset();

		return result;
	}

	/**
	 * バス構成が変更されたときに呼び出される関数
	 * 例：入力バスがモノラルからステレオに変更されたときなど
	 * @param [in] inputs 入力バスのスピーカー配置の配列
	 * @param [in] numIns 入力バスの数
	 * @param [in] outputs 出力バスのスピーカー配置の配列
	 * @param [in] numOuts 出力バスの数
	 */
	tresult PLUGIN_API Kyun2StopProcessor::setBusArrangements(SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts) {
		// mono->mono / stereo->stereo を受け付ける
		if (numIns == 1 && numOuts == 1 &&
			((inputs[0] == SpeakerArr::kMono && outputs[0] == SpeakerArr::kMono) ||
			 (inputs[0] == SpeakerArr::kStereo && outputs[0] == SpeakerArr::kStereo))) {
			return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
		}

		// 対応していないバス構成の場合はエラーを返す。
		return kResultFalse;
	}

	/**
	 * 実際に音声の処理を行う関数
	 * @param [in] data 処理に必要なデータをまとめた構造体
	 */
	tresult PLUGIN_API Kyun2StopProcessor::process(ProcessData& data) {
		// パラメータの変更がある場合は処理する
		if (data.inputParameterChanges != NULL) {
			// 与えられたパラメータの変更数を取得する
			int32 paramChangeCount = data.inputParameterChanges->getParameterCount();

			// 与えられたパラメータ分、ループして処理する
			for (int32 i = 0; i < paramChangeCount; ++i) {
				// パラメータ変更のキューを取得する
				IParamValueQueue* queue = data.inputParameterChanges->getParameterData(i);
				if (queue != NULL) {
					// 変更されたパラメータのタグを取得する
					ParamID tag = queue->getParameterId();

					// 変更された回数を取得
					int32 valueChangeCount = queue->getPointCount();
					ParamValue value;
					int32 sampleOffset;

					// 最後に変更された値を取得する
					if (queue->getPoint(valueChangeCount - 1, sampleOffset, value) == kResultTrue) {
						switch (tag) {
						case PARAM_TRIGGER_TAG:
							// Bool (0.0 or 1.0)
							triggerFlag = (value > 0.5);
							break;
						case PARAM_USE_SYNC_TAG:
							useSync = (value > 0.5);
							break;
						case PARAM_STOP_TIME_TAG:
							// normalized [0,1] -> [0.1, 2.0]
							stopTimeSec = 0.1f + static_cast<float>(value) * (2.0f - 0.1f);
							break;
						case PARAM_SYNC_BEAT_TAG:
							syncBeat = static_cast<int32>(std::lround(value * 4.0));
							break;
						case PARAM_START_TIME_TAG:
							// normalized [0,1] -> [0.1, 2.0]
							startTimeSec = 0.1f + static_cast<float>(value) * (2.0f - 0.1f);
							break;
						case PARAM_CURVE_TAG:
							curve = static_cast<int32>(std::lround(value * 3.0));
							break;
						case PARAM_ENABLE_FILTER_TAG:
							enableFilter = (value > 0.5);
							break;
						default:
							break;
						}
					}
				}
			}
		}

		if (!tapeStopEngine) {
			constexpr float kFallbackSampleRate = 48000.0f;
			constexpr float kMaxDelaySeconds = 8.0f;
			tapeStopEngine = std::make_unique<TapeStopEngine>(kFallbackSampleRate, kMaxDelaySeconds, 2);
		}

		double bpm = 120.0;
		if (data.processContext && (data.processContext->state & ProcessContext::kTempoValid)) {
			bpm = data.processContext->tempo;
		}

		// mono/stereo 共通処理（monoはLのみ書き戻す）
		if (data.numInputs == 1 && data.numOutputs == 1 && data.outputs[0].numChannels >= 1) {
			Sample32* outL = data.outputs[0].channelBuffers32[0];
			Sample32* outR = (data.outputs[0].numChannels >= 2) ? data.outputs[0].channelBuffers32[1] : nullptr;
			if (!outL)
				return kResultTrue;

			Sample32* inL = data.inputs[0].channelBuffers32[0];
			if (!inL)
				return kResultTrue;

			const bool stereoIn = (data.inputs[0].numChannels >= 2 && data.inputs[0].channelBuffers32[1] != nullptr);
			const bool stereoOut = (outR != nullptr);
			Sample32* inR = stereoIn ? data.inputs[0].channelBuffers32[1] : inL;

			TapeCurve curveType = static_cast<TapeCurve>(std::clamp(curve, 0, 3));
			SyncBeat beatType = static_cast<SyncBeat>(std::clamp(syncBeat, 0, 4));

			for (int32 i = 0; i < data.numSamples; ++i) {
				float inFrame[2] = { inL[i], inR[i] };
				float outFrame[2] = { 0.0f, 0.0f };
				tapeStopEngine->Process(
					inFrame,
					outFrame,
					2,
					triggerFlag,
					stopTimeSec,
					startTimeSec,
					curveType,
					useSync,
					beatType,
					bpm,
					enableFilter
				);

				outL[i] = outFrame[0];
				if (stereoOut)
					outR[i] = outFrame[1];
			}

			return kResultTrue;
		}

		return kResultTrue;
	}

} }