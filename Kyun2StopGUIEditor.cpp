#include "Kyun2StopGUIEditor.h"

namespace Steinberg {
namespace Vst {

	/**
	 * コンストラクタ
	 * param [in] controller コントローラーのポインタ
	 */
	Kyun2StopGUIEditor::Kyun2StopGUIEditor(void* controller)
		: VSTGUIEditor(controller) {

		// ウィンドウサイズを設定する
		ViewRect viewRect(0, 0, 200, 200);
		setRect(viewRect);
	}

	/**
	 * GUIウィンドウを開いたときに呼び出される関数
	 * ここでつまみやスライダーを作成する
	 * param [in] parent VSTGUI用のウィンドウハンドル
	 * param [in] platformType プラットフォームの種類
	 */
	bool PLUGIN_API Kyun2StopGUIEditor::open(void* parent, const PlatformType& platformType) {
		// フレームがあるかどうかを確認
		if (frame) {
			return false;
		}

		// 作成するフレームサイズを設定する
		CRect size(0, 0, 200, 200);

		// フレームを作成する
		frame = new CFrame(size, this);
		if (frame == NULL) {
			return false;
		}

		// 作成したフレームの背景画像を設定
		CBitmap* cbmp = new CBitmap("background.png"); // .rcファイルから読み込む
		frame->setBackground(cbmp);                    // フレームに背景画像を設定
		cbmp->forget();                                // フレームに設定後、背景画像のハンドルを解放

		// 作成したフレームを開く
		frame->open(parent);

		return true;
	}

	/**
	 * GUIウィンドウを閉じたときに呼び出される関数
	 * ここで作成したフレームの解放を行う
	 */
	void PLUGIN_API Kyun2StopGUIEditor::close() {
		// フレーム解放
		if (frame) {
			frame->forget();
			frame = 0;
		}
	}

	/**
	 * つまみやスライダーを操作したときに呼び出される関数
	 * param [in] pControl 操作されたコントローラーのポインタ
	 */
	void Kyun2StopGUIEditor::valueChanged(CControl* pControl) {

	}
} }