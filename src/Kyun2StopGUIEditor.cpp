#include "Kyun2StopGUIEditor.h"

#include <algorithm>
#include <ctime>
#include <cstdio>

#include "vstgui/lib/controls/cbuttons.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/lib/cvstguitimer.h"

namespace Steinberg {
namespace Vst {

namespace {
	class KyunKawaiiBackgroundView : public CView {
	public:
		explicit KyunKawaiiBackgroundView (const CRect& size) : CView (size) {}

		void draw (CDrawContext* context) override {
			const double time = static_cast<double> (std::clock ()) / static_cast<double> (CLOCKS_PER_SEC);
			const auto r = getViewSize ();

			// Soft pastel base gradient (striped approximation)
			const int strips = 20;
			for (int i = 0; i < strips; ++i) {
				const float t = static_cast<float> (i) / static_cast<float> (strips - 1);
				const uint8_t rr = static_cast<uint8_t> (245 + 10 * std::sin (time * 0.6 + t * 4.0));
				const uint8_t gg = static_cast<uint8_t> (230 + 15 * std::sin (time * 0.5 + t * 3.0 + 1.2));
				const uint8_t bb = static_cast<uint8_t> (245 + 8 * std::cos (time * 0.4 + t * 5.0));
				context->setFillColor (CColor (rr, gg, bb, 255));
				const CCoord y0 = r.top + (r.getHeight () / strips) * i;
				const CCoord y1 = r.top + (r.getHeight () / strips) * (i + 1);
				context->drawRect (CRect (r.left, y0, r.right, y1), kDrawFilled);
			}

			// Floating blobs inspired by the egui prototype
			for (int i = 0; i < 6; ++i) {
				const double seed = i * 1.371;
				const CCoord cx = r.left + r.getWidth () * (0.5 + 0.42 * std::sin (time * 0.19 + seed));
				const CCoord cy = r.top + r.getHeight () * (0.5 + 0.38 * std::cos (time * 0.23 + seed * 2.0));
				const CCoord rad = 28.0 + 10.0 * std::sin (time * 0.7 + seed);
				const uint8_t alpha = static_cast<uint8_t> (60 + i * 20);
				const CColor blobColor (
					static_cast<uint8_t> (255),
					static_cast<uint8_t> (160 + (i * 13) % 70),
					static_cast<uint8_t> (210 + (i * 9) % 40),
					alpha);
				context->setFillColor (blobColor);
				context->setFrameColor (CColor (255, 255, 255, alpha / 2));
				context->drawEllipse (CRect (cx - rad, cy - rad, cx + rad, cy + rad), kDrawFilledAndStroked);
			}

			setDirty (false);
		}

	};

	enum ControlTags : int32 {
		kCurvePrevTag = 2000,
		kCurveNextTag,
		kBeatPrevTag,
		kBeatNextTag,
		kStopMinusTag,
		kStopPlusTag,
		kStartMinusTag,
		kStartPlusTag,
	};

	const char* kCurveNames[] = {"Linear", "Smooth", "SlowStart", "QuickCut"};
	const char* kBeatNames[] = {"1/8", "1/4", "1/2", "1 Bar", "2 Bars"};

	inline float normToSeconds (float norm)
	{
		norm = std::clamp(norm, 0.f, 1.f);
		return 0.1f + norm * (2.0f - 0.1f);
	}

	inline float secondsToNorm (float sec)
	{
		sec = std::clamp(sec, 0.1f, 2.0f);
		return (sec - 0.1f) / (2.0f - 0.1f);
	}
}

/**
 * コンストラクタ
 * param [in] controller コントローラーのポインタ
 */
Kyun2StopGUIEditor::Kyun2StopGUIEditor (void* controllerPtr)
	: VSTGUIEditor (controllerPtr)
{
	ViewRect viewRect (0, 0, 460, 340);
	setRect (viewRect);
}

/**
 * GUIウィンドウを開いたときに呼び出される関数
 */
bool PLUGIN_API Kyun2StopGUIEditor::open (void* parent, const PlatformType& platformType)
{
	if (frame)
		return false;

	CRect size (0, 0, 460, 340);
	frame = new CFrame (size, this);
	if (!frame)
		return false;

	frame->setBackgroundColor (CColor (255, 240, 250, 255));
	frame->open (parent);

	backgroundView = new KyunKawaiiBackgroundView (size);
	frame->addView (backgroundView);

	if (auto* controller = getController ())
	{
		stopTimeSec = normToSeconds (static_cast<float> (controller->getParamNormalized (PARAM_STOP_TIME_TAG)));
		startTimeSec = normToSeconds (static_cast<float> (controller->getParamNormalized (PARAM_START_TIME_TAG)));
		curveIndex = std::clamp<int32> (static_cast<int32> (std::lround (controller->getParamNormalized (PARAM_CURVE_TAG) * 3.0)), 0, 3);
		syncBeatIndex = std::clamp<int32> (static_cast<int32> (std::lround (controller->getParamNormalized (PARAM_SYNC_BEAT_TAG) * 4.0)), 0, 4);
	}

	// Title
	auto* title = new CTextLabel (CRect (20, 12, 440, 40), "Kyun'Stop");
	title->setFontColor (CColor (180, 70, 120, 255));
	frame->addView (title);

	// Boolean parameters
	auto* trigger = new CCheckBox (CRect (20, 52, 190, 78), this, PARAM_TRIGGER_TAG, "Trigger");
	auto* sync = new CCheckBox (CRect (20, 82, 190, 108), this, PARAM_USE_SYNC_TAG, "BPM Sync");
	auto* filter = new CCheckBox (CRect (20, 112, 220, 138), this, PARAM_ENABLE_FILTER_TAG, "Low-pass Effect");
	if (auto* controller = getController ())
	{
		trigger->setValueNormalized (static_cast<float> (controller->getParamNormalized (PARAM_TRIGGER_TAG)));
		sync->setValueNormalized (static_cast<float> (controller->getParamNormalized (PARAM_USE_SYNC_TAG)));
		filter->setValueNormalized (static_cast<float> (controller->getParamNormalized (PARAM_ENABLE_FILTER_TAG)));
	}
	frame->addView (trigger);
	frame->addView (sync);
	frame->addView (filter);

	// Curve row
	frame->addView (new CTextLabel (CRect (20, 160, 130, 182), "Curve"));
	auto* curvePrev = new CTextButton (CRect (140, 156, 170, 184), this, kCurvePrevTag, "-");
	curvePrev->setRoundRadius (6.0);
	curvePrev->setFrameColor (CColor (250, 120, 170, 220));
	frame->addView (curvePrev);
	curveLabel = new CTextLabel (CRect (176, 160, 300, 182), "");
	curveLabel->setFontColor (CColor (90, 70, 90, 255));
	frame->addView (curveLabel);
	auto* curveNext = new CTextButton (CRect (306, 156, 336, 184), this, kCurveNextTag, "+");
	curveNext->setRoundRadius (6.0);
	curveNext->setFrameColor (CColor (250, 120, 170, 220));
	frame->addView (curveNext);

	// Sync beat row
	frame->addView (new CTextLabel (CRect (20, 194, 130, 216), "Sync Beat"));
	auto* beatPrev = new CTextButton (CRect (140, 190, 170, 218), this, kBeatPrevTag, "-");
	beatPrev->setRoundRadius (6.0);
	beatPrev->setFrameColor (CColor (250, 120, 170, 220));
	frame->addView (beatPrev);
	syncBeatLabel = new CTextLabel (CRect (176, 194, 300, 216), "");
	syncBeatLabel->setFontColor (CColor (90, 70, 90, 255));
	frame->addView (syncBeatLabel);
	auto* beatNext = new CTextButton (CRect (306, 190, 336, 218), this, kBeatNextTag, "+");
	beatNext->setRoundRadius (6.0);
	beatNext->setFrameColor (CColor (250, 120, 170, 220));
	frame->addView (beatNext);

	// Stop time row
	frame->addView (new CTextLabel (CRect (20, 228, 130, 250), "Stop Time"));
	auto* stopMinus = new CTextButton (CRect (140, 224, 170, 252), this, kStopMinusTag, "-");
	stopMinus->setRoundRadius (6.0);
	stopMinus->setFrameColor (CColor (250, 120, 170, 220));
	frame->addView (stopMinus);
	stopTimeLabel = new CTextLabel (CRect (176, 228, 300, 250), "");
	stopTimeLabel->setFontColor (CColor (90, 70, 90, 255));
	frame->addView (stopTimeLabel);
	auto* stopPlus = new CTextButton (CRect (306, 224, 336, 252), this, kStopPlusTag, "+");
	stopPlus->setRoundRadius (6.0);
	stopPlus->setFrameColor (CColor (250, 120, 170, 220));
	frame->addView (stopPlus);

	// Start time row
	frame->addView (new CTextLabel (CRect (20, 262, 130, 284), "Start Time"));
	auto* startMinus = new CTextButton (CRect (140, 258, 170, 286), this, kStartMinusTag, "-");
	startMinus->setRoundRadius (6.0);
	startMinus->setFrameColor (CColor (250, 120, 170, 220));
	frame->addView (startMinus);
	startTimeLabel = new CTextLabel (CRect (176, 262, 300, 284), "");
	startTimeLabel->setFontColor (CColor (90, 70, 90, 255));
	frame->addView (startTimeLabel);
	auto* startPlus = new CTextButton (CRect (306, 258, 336, 286), this, kStartPlusTag, "+");
	startPlus->setRoundRadius (6.0);
	startPlus->setFrameColor (CColor (250, 120, 170, 220));
	frame->addView (startPlus);

	uiTimer = new VSTGUI::CVSTGUITimer ([this] (VSTGUI::CVSTGUITimer*) {
		if (frame)
			frame->invalid ();
	}, 33, true);

	updateLabels ();
	return true;
}

/**
 * GUIウィンドウを閉じたときに呼び出される関数
 */
void PLUGIN_API Kyun2StopGUIEditor::close ()
{
	if (uiTimer) {
		uiTimer->stop ();
		uiTimer->forget ();
		uiTimer = nullptr;
	}

	backgroundView = nullptr;
	stopTimeLabel = nullptr;
	startTimeLabel = nullptr;
	curveLabel = nullptr;
	syncBeatLabel = nullptr;

	if (frame) {
		frame->forget ();
		frame = nullptr;
	}
}

void Kyun2StopGUIEditor::sendParamNormalized (ParamID tag, ParamValue value)
{
	if (auto* controller = getController ())
	{
		controller->beginEdit (tag);
		controller->performEdit (tag, value);
		controller->setParamNormalized (tag, value);
		controller->endEdit (tag);
	}
}

void Kyun2StopGUIEditor::updateLabels ()
{
	if (curveLabel)
		curveLabel->setText (kCurveNames[std::clamp<int32> (curveIndex, 0, 3)]);
	if (syncBeatLabel)
		syncBeatLabel->setText (kBeatNames[std::clamp<int32> (syncBeatIndex, 0, 4)]);

	char text[64];
	if (stopTimeLabel)
	{
		(void)std::snprintf (text, sizeof (text), "%.2f s", stopTimeSec);
		stopTimeLabel->setText (text);
	}
	if (startTimeLabel)
	{
		(void)std::snprintf (text, sizeof (text), "%.2f s", startTimeSec);
		startTimeLabel->setText (text);
	}
}

/**
 * コントロール操作時のコールバック
 */
void Kyun2StopGUIEditor::valueChanged (CControl* pControl)
{
	if (!pControl)
		return;

	const auto tag = static_cast<ParamID> (pControl->getTag ());
	switch (tag)
	{
	case PARAM_TRIGGER_TAG:
	case PARAM_USE_SYNC_TAG:
	case PARAM_ENABLE_FILTER_TAG:
		sendParamNormalized (tag, pControl->getValueNormalized ());
		break;
	case kCurvePrevTag:
		curveIndex = std::max<int32> (0, curveIndex - 1);
		sendParamNormalized (PARAM_CURVE_TAG, static_cast<double> (curveIndex) / 3.0);
		updateLabels ();
		break;
	case kCurveNextTag:
		curveIndex = std::min<int32> (3, curveIndex + 1);
		sendParamNormalized (PARAM_CURVE_TAG, static_cast<double> (curveIndex) / 3.0);
		updateLabels ();
		break;
	case kBeatPrevTag:
		syncBeatIndex = std::max<int32> (0, syncBeatIndex - 1);
		sendParamNormalized (PARAM_SYNC_BEAT_TAG, static_cast<double> (syncBeatIndex) / 4.0);
		updateLabels ();
		break;
	case kBeatNextTag:
		syncBeatIndex = std::min<int32> (4, syncBeatIndex + 1);
		sendParamNormalized (PARAM_SYNC_BEAT_TAG, static_cast<double> (syncBeatIndex) / 4.0);
		updateLabels ();
		break;
	case kStopMinusTag:
		stopTimeSec = std::max (0.1f, stopTimeSec - 0.05f);
		sendParamNormalized (PARAM_STOP_TIME_TAG, secondsToNorm (stopTimeSec));
		updateLabels ();
		break;
	case kStopPlusTag:
		stopTimeSec = std::min (2.0f, stopTimeSec + 0.05f);
		sendParamNormalized (PARAM_STOP_TIME_TAG, secondsToNorm (stopTimeSec));
		updateLabels ();
		break;
	case kStartMinusTag:
		startTimeSec = std::max (0.1f, startTimeSec - 0.05f);
		sendParamNormalized (PARAM_START_TIME_TAG, secondsToNorm (startTimeSec));
		updateLabels ();
		break;
	case kStartPlusTag:
		startTimeSec = std::min (2.0f, startTimeSec + 0.05f);
		sendParamNormalized (PARAM_START_TIME_TAG, secondsToNorm (startTimeSec));
		updateLabels ();
		break;
	default:
		break;
	}
}

} }