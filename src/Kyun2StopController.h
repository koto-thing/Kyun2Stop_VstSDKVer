#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"

#include "Kyun2StopFuid.h"

namespace Steinberg {
namespace Vst {

	class Kyun2StopGUIEditor;

	class Kyun2StopController : public EditController {
	public:
		// クラスを初期化する関数
		tresult PLUGIN_API initialize(FUnknown* context) override;

		// GUIエディタを作成
		IPlugView* PLUGIN_API createView(FIDString name) SMTG_OVERRIDE;

		// パラメータ値が変更されたときに呼び出される関数
		tresult PLUGIN_API setParamNormalized(ParamID tag, ParamValue value) override;

		// 現在アクティブなエディタへのポインタ
		Kyun2StopGUIEditor* editor = nullptr;

		// Kyun2StopControllerクラスのインスタンスを作成するための関数
		static FUnknown* createInstance(void*) { return (IEditController*)new Kyun2StopController(); }
	};

} }