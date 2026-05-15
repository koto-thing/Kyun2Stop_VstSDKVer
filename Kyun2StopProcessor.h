#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"

namespace Steinberg {
namespace Vst {

	class Kyun2StopProcessor : public AudioEffect {
	protected:
		ParamValue volume;

	public:
		// コンストラクタ
		Kyun2StopProcessor();

		// クラスを初期化する関数
		tresult PLUGIN_API initialize(FUnknown* context);

		// パスを設定する関数
		tresult PLUGIN_API setBusArrangements(SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts);

		// 音声信号を処理する関数
		tresult PLUGIN_API process(ProcessData& data);

		// VST3プロセッサークラスのインスタンスを作成するための関数
		static FUnknown* createInstance(void*) { return (IAudioProcessor*)new Kyun2StopProcessor(); }
	};

} }