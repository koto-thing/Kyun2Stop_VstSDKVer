#pragma once

#include <memory>

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"

#include "Engine/TapeStopEngine.h"

namespace Steinberg {
namespace Vst {

	class Kyun2StopProcessor : public AudioEffect {
	protected:
		std::unique_ptr<TapeStopEngine> tapeStopEngine;
		bool triggerFlag = false;
		bool useSync = false;
		float stopTimeSec = 0.5f;
		int32 syncBeat = 1;
		float startTimeSec = 0.5f;
		int32 curve = 0;
		bool enableFilter = true;

	public:
		// コンストラクタ
		Kyun2StopProcessor();

		// クラスを初期化する関数
		tresult PLUGIN_API initialize(FUnknown* context);

		// パスを設定する関数
		tresult PLUGIN_API setBusArrangements(SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts);

		// サンプルレート変更時にエンジンを再生成
		tresult PLUGIN_API setupProcessing(ProcessSetup& newSetup);

		// 音声信号を処理する関数
		tresult PLUGIN_API process(ProcessData& data);

		// VST3プロセッサークラスのインスタンスを作成するための関数
		static FUnknown* createInstance(void*) { return (IAudioProcessor*)new Kyun2StopProcessor(); }
	};

} }