#pragma once

#include "public.sdk/source/vst/vstguieditor.h"
#include "pluginterfaces/vst/ivstplugview.h"

#include "Kyun2StopDefine.h"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class Kyun2StopGUIEditor : public VSTGUIEditor, public IControlListener {
public:
	// コンストラクタ
	Kyun2StopGUIEditor(void* controller);

	// GUIウィンドウを開いたときに呼び出される関数
	virtual bool PLUGIN_API open(void* parent, const PlatformType& platformType = PlatformType::kDefaultNative);

	// GUIウィンドウを閉じたときに呼び出される関数
	virtual void PLUGIN_API close();

	// GUIウィンドウのコントローラを操作したときに呼び出される関数
	void valueChanged(CControl* pControl);

	// VSTGUIEditorクラスの各種設定を自作GUIクラスに置き換えるマクロ
	DELEGATE_REFCOUNT(VSTGUIEditor)
};

} }