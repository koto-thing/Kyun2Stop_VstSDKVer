#include "public.sdk/source/main/pluginfactory.h"

#include "Kyun2StopFuid.h"
#include "Kyun2StopProcessor.h"
#include "Kyun2StopController.h"

// 制作者の名前を定義
#define KYUN2STOP_VENDOR "koto-thing(Goto Kenta)"

// 制作者のウェブサイトURLを定義
#define KYUN2STOP_URL "https://koto-thing/Portfolio"

// 制作者の連絡先メールアドレスを定義
#define KYUN2STOP_EMAIL "gotoukenta62@gmail.com"

// VST3のプラグイン名を定義
#define KYUN2STOP_NAME "Kyun2Stop_v2"

// VST3のプラグインバージョンを定義
#define KYUN2STOP_VERSION "1.0.2"

// VST3のカテゴリを定義
#define KYUN2STOP_SUBCATEGORIES Vst::PlugType::kFx

/**
 * 生成の定義
 * @param [in] KYUN2STOP_VENDOR 制作者の名前
 * @param [in] KYUN2STOP_URL 制作者のウェブサイトURL
 * @param [in] KYUN2STOP_EMAIL 制作者の連絡先メールアドレス
*/
BEGIN_FACTORY_DEF(KYUN2STOP_VENDOR, KYUN2STOP_URL, KYUN2STOP_EMAIL)

	
	DEF_CLASS2(
		INLINE_UID_FROM_FUID(Steinberg::Vst::Kyun2StopFuid),           // 音声処理クラスのFUID
		PClassInfo::kManyInstances,                                    // PClassInfo::kManyInstancesを指定する。
		kVstAudioEffectClass,                                          // 音声処理クラス
		KYUN2STOP_NAME,                                                // VST3プラグイン名
		Vst::kDistributable,                                           // 音声処理クラス: Vst::kDistributable
		KYUN2STOP_SUBCATEGORIES,                                       // エフェクタ: kFx, インストゥルメント: kInstrument
		KYUN2STOP_VERSION,                                             // VST3プラグインバージョン
		kVstVersionString,                                             // kVstVersionStringを指定する
		Steinberg::Vst::Kyun2StopProcessor::createInstance             // 音声処理クラスの生成関数
	)

	DEF_CLASS2(
		INLINE_UID_FROM_FUID(Steinberg::Vst::Kyun2StopControllerFuid), // パラメータ操作クラスのFUID
		PClassInfo::kManyInstances,
		kVstComponentControllerClass,                                  // パラメータ操作クラス
		KYUN2STOP_NAME "Controller",
		0,
		"",
		KYUN2STOP_VERSION,
		kVstVersionString,
		Steinberg::Vst::Kyun2StopController::createInstance
	)

END_FACTORY