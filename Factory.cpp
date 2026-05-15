#include "public.sdk/source/main/pluginfactory.h"

#include "Kyun2StopFuid.h"
#include "Kyun2StopProcessor.h"

// 制作者の名前を定義
#define KYUN2STOP_VENDOR "koto-thing(Goto Kenta)"

// 制作者のウェブサイトURLを定義
#define KYUN2STOP_URL "https://koto-thing/Portfolio"

// 制作者の連絡先メールアドレスを定義
#define KYUN2STOP_EMAIL "gotoukenta62@gmail.com"

// VST3のプラグイン名を定義
#define KYUN2STOP_NAME "Kyun2Stop"

// VST3のプラグインバージョンを定義
#define KYUN2STOP_VERSION "1.0.0"

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
		INLINE_UID_FROM_FUID(Steinberg::Vst::Kyun2StopControllerFuid), // 音声処理クラス・パラメータ操作クラスのFUID
		PClassInfo::kManyInstances,                                    // PClassInfo::kManyInstancesを指定する。
		kVstAudioEffectClass,                                          // 音声処理クラス: kVstAudioEffectClass, パラメータ操作クラス: kVstAudioEffectClass
		KYUN2STOP_NAME,                                                // VST3プラグイン名
		Vst::kDistributable,                                           // 音声処理クラス: Vst::kDistributable, パラメータ操作クラス: 0
		KYUN2STOP_SUBCATEGORIES,                                       // エフェクタ: kFx, インストゥルメント: kInstrument
		KYUN2STOP_VERSION,                                             // VST3プラグインバージョン
		kVstVersionString,                                             // kVstVersionStringを指定する
		Steinberg::Vst::Kyun2StopProcessor::createInstance             // 音声処理クラス・パラメータ操作クラスの生成関数のポインタ
	)

END_FACTORY