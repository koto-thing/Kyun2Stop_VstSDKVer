#include "Kyun2StopDefine.h"
#include "Kyun2StopFuid.h"
#include "Kyun2StopController.h"
#include "Kyun2StopGUIEditor.h"

namespace Steinberg {
namespace Vst {

	tresult PLUGIN_API Kyun2StopController::initialize(FUnknown* context) {
		// 基底クラスの初期化を実行
		tresult result = EditController::initialize(context);
		if (result == kResultTrue) {
			parameters.addParameter(STR16("Trigger"), STR16(""), 1, 0, 0, ParameterInfo::kCanAutomate, PARAM_TRIGGER_TAG);
			parameters.addParameter(STR16("BPM Sync"), STR16(""), 1, 0, 0, ParameterInfo::kCanAutomate, PARAM_USE_SYNC_TAG);
			parameters.addParameter(STR16("Stop Time (Sec)"), STR16("s"), 0, 0.5, 0, ParameterInfo::kCanAutomate, PARAM_STOP_TIME_TAG);
			parameters.addParameter(STR16("Stop Beat"), STR16(""), 4, 1, 0, ParameterInfo::kCanAutomate, PARAM_SYNC_BEAT_TAG);
			parameters.addParameter(STR16("Start Time"), STR16("s"), 0, 0.5, 0, ParameterInfo::kCanAutomate, PARAM_START_TIME_TAG);
			parameters.addParameter(STR16("Curve"), STR16(""), 3, 0, 0, ParameterInfo::kCanAutomate, PARAM_CURVE_TAG);
			parameters.addParameter(STR16("Low-pass Effect"), STR16(""), 1, 1, 0, ParameterInfo::kCanAutomate, PARAM_ENABLE_FILTER_TAG);
		}

		result = kResultTrue;
		return result;
	}

	IPlugView* PLUGIN_API Kyun2StopController::createView(FIDString name) {
		if (name && strcmp(name, ViewType::kEditor) == 0) {
			return new Kyun2StopGUIEditor(this);
		}
		return nullptr;
	}

} }