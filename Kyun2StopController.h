#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"

#include "Kyun2StopFuid.h"

namespace Steinberg {
namespace Vst {

	class Kyun2StopController : public EditController {
	public:
		// クラスを初期化する関数
		tresult PLUGIN_API initialize(FUnknown* context);

		// Kyun2StopControllerクラスのインスタンスを作成するための関数
		static FUnknown* createInstance(void*) { return (IEditController*)new Kyun2StopController(); }
	};

} }