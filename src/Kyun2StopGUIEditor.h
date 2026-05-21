#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif

#include "public.sdk/source/vst/vstguieditor.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include "pluginterfaces/vst/ivstplugview.h"

#include "Kyun2StopDefine.h"

namespace VSTGUI {
class CVSTGUITimer;
}

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;


class Kyun2StopGUIEditor : public VSTGUIEditor, public IControlListener {
public:
	// コンストラクタ
	Kyun2StopGUIEditor(void* controllerPtr);

	// GUIウィンドウを開いたときに呼び出される関数
	virtual bool PLUGIN_API open(void* parent, const PlatformType& platformType = PlatformType::kDefaultNative) override;

	// GUIウィンドウを閉じたときに呼び出される関数
	virtual void PLUGIN_API close() override;

	// GUIウィンドウのコントローラを操作したときに呼び出される関数
	void valueChanged(CControl* pControl) override;

private:
	void sendParamNormalized(ParamID tag, ParamValue value);
	void updateLabels();

	CTextLabel* stopTimeLabel = nullptr;
	CTextLabel* startTimeLabel = nullptr;
	CTextLabel* curveLabel = nullptr;
	CTextLabel* syncBeatLabel = nullptr;
	CView* backgroundView = nullptr;
	VSTGUI::CVSTGUITimer* uiTimer = nullptr;

	float stopTimeSec = 0.5f;
	float startTimeSec = 0.5f;
	int32 curveIndex = 0;
	int32 syncBeatIndex = 1;

	// VSTGUIEditorクラスの各種設定を自作GUIクラスに置き換えるマクロ
	DELEGATE_REFCOUNT(VSTGUIEditor)
};

} }