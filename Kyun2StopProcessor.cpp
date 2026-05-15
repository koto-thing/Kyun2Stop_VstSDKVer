#include "pluginterfaces/vst/ivstparameterchanges.h"

#include "Kyun2StopFuid.h"
#include "Kyun2StopProcessor.h"
#include "Kyun2StopDefine.h"

namespace Steinberg {
namespace Vst {
	/**
	 * コンストラクタ 
	 */
	Kyun2StopProcessor::Kyun2StopProcessor()
		: volume(1.0) {
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
			addAudioInput(STR16("AudioOutput"), SpeakerArr::kStereo);
		}

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
		// inputとoutputの数が1で、両方ともステレオであるなら
		if (numIns == 1 && numOuts == 1 && inputs[0] == SpeakerArr::kStereo && outputs[0] == SpeakerArr::kStereo) {
			return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
		}
		// inputとoutputの数が1で、入力がモノラルで出力がステレオであるなら
		else if (numIns == 1 && numOuts == 1 && inputs[0] == SpeakerArr::kMono && outputs[0] == SpeakerArr::kStereo) {
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
					int32 tag = queue->getParameterId();

					// 変更された回数を取得
					int32 valueChangeCount = queue->getPointCount();
					ParamValue value;
					int32 sampleOffset;

					// 最後に変更された値を取得する
					if (queue->getPoint(valueChangeCount - 1, sampleOffset, value) == kResultTrue) {
						switch (tag) {
						case PARAM1_TAG:
							volume = value;
							break;
						}
					}
				}
			}
		}

		// 入力と出力のバス数をチェック
		if (data.numInputs == 1 && data.numOutputs == 1) {
			// 入力と出力がステレオなら
			if (data.inputs[0].numChannels == 2 && data.outputs[0].numChannels == 2) {
				Sample32* inL = data.inputs[0].channelBuffers32[0];
				Sample32* inR = data.inputs[0].channelBuffers32[1];
				Sample32* outL = data.outputs[0].channelBuffers32[0];
				Sample32* outR = data.outputs[0].channelBuffers32[1];

				// サンプル数分、入力をそのまま出力にコピーする
				for (int32 i = 0; i < data.numSamples; ++i) {
					outL[i] = inL[i] * volume;
					outR[i] = inR[i] * volume;
				}

				return kResultTrue;
			}
			// 入力がモノラルで出力がステレオなら
			else if (data.inputs[0].numChannels == 1 && data.outputs[0].numChannels == 2) {
				Sample32* in = data.inputs[0].channelBuffers32[0];
				Sample32* outL = data.outputs[0].channelBuffers32[0];
				Sample32* outR = data.outputs[0].channelBuffers32[1];

				// サンプル数分、入力を左右両方の出力にコピーする
				for (int32 i = 0; i < data.numSamples; ++i) {
					outL[i] = in[i] * volume;
					outR[i] = in[i] * volume;
				}

				return kResultTrue;
			}
		}

		return kResultTrue;
	}

} }