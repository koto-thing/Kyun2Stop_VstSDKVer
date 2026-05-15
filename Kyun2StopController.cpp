#include "Kyun2StopDefine.h"
#include "Kyun2StopFuid.h"
#include "Kyun2StopController.h"

namespace Steinberg {
namespace Vst {

	tresult PLUGIN_API Kyun2StopController::initialize(FUnknown* context) {
		// 基底クラスの初期化を実行
		tresult result = EditController::initialize(context);
		if (result == kResultTrue) {
			parameters.addParameter(
				STR16("param1"),                        // パラメータの名前
				STR16("..."),                           // パラメータの単位
				0,                                      // 何段階のパラメータか
				1,                                      // デフォルト値
				ParameterInfo::kCanAutomate,            // パラメータのフラグ
				PARAM1_TAG                              // パラメータのタグ
			);
		}

		result = kResultTrue;
		return result;
	}

} }