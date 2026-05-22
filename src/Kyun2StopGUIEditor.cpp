#include "Kyun2StopGUIEditor.h"
#include "Kyun2StopController.h"
#include "Kyun2StopDefine.h"

#include <algorithm>
#include <ctime>
#include <cstdio>
#include <cmath>
#include <cstring>

#include "vstgui/lib/controls/cbuttons.h"
#include "vstgui/lib/controls/ctextlabel.h"
#include "vstgui/lib/controls/coptionmenu.h"
#include "vstgui/lib/controls/cslider.h"
#include "vstgui/lib/cvstguitimer.h"
#include "vstgui/lib/cgradient.h"

namespace Steinberg {
namespace Vst {

namespace {
	CColor hsvaToCColor(float h, float s, float v, float a) {
		float r = 0, g = 0, b = 0;
		if (s == 0) {
			r = g = b = v;
		} else {
			float hTemp = h * 6.0f;
			if (hTemp >= 6.0f) hTemp = 0.0f;
			int i = static_cast<int>(hTemp);
			float f = hTemp - i;
			float p = v * (1.0f - s);
			float q = v * (1.0f - s * f);
			float t = v * (1.0f - s * (1.0f - f));
			switch (i) {
				case 0: r = v; g = t; b = p; break;
				case 1: r = q; g = v; b = p; break;
				case 2: r = p; g = v; b = t; break;
				case 3: r = p; g = q; b = v; break;
				case 4: r = t; g = p; b = v; break;
				default: r = v; g = p; b = q; break;
			}
		}
		return CColor(
			static_cast<uint8_t>(r * 255.0f),
			static_cast<uint8_t>(g * 255.0f),
			static_cast<uint8_t>(b * 255.0f),
			static_cast<uint8_t>(a * 255.0f)
		);
	}

	CColor getPastelColor(double t, double offset) {
		float h = static_cast<float>(std::fmod(t * 0.05 + offset, 1.0));
		if (h < 0.0f) h += 1.0f;
		return hsvaToCColor(h, 0.4f, 0.9f, 1.0f);
	}

	CColor getKawaiiColor(double t, double offset) {
		float h = static_cast<float>(std::fmod(t * 0.1 + offset, 1.0));
		if (h < 0.0f) h += 1.0f;
		return hsvaToCColor(h, 0.55f, 1.0f, 0.5f);
	}

	void paintAmoeba(CDrawContext* context, CPoint center, float radius, CColor color, double time, double seed) {
		constexpr int num_points = 64;
		PointList pointList;
		pointList.reserve(num_points + 1);
		
		for (int i = 0; i < num_points; ++i) {
			double angle = (static_cast<double>(i) / num_points) * 2.0 * 3.14159265358979323846;
			double noise = std::sin(angle * 2.0 + time * 0.5 + seed) * 0.15
						 + std::cos(angle * 3.0 - time * 0.3 + seed) * 0.1;
			float r = radius * (1.0f + static_cast<float>(noise));
			CPoint pt;
			pt.x = center.x + std::cos(angle) * r;
			pt.y = center.y + std::sin(angle) * r;
			pointList.push_back(pt);
		}
		pointList.push_back(pointList[0]);
		
		context->setFillColor(color);
		context->setFrameColor(color);
		context->drawPolygon(pointList, kDrawFilled);
	}

	inline void fillRoundRect(CDrawContext* context, const CRect& size, CCoord radius) {
		CGraphicsPath* path = context->createRoundRectGraphicsPath(size, radius);
		if (path) {
			context->drawGraphicsPath(path, CDrawContext::kPathFilled);
			path->forget();
		}
	}

	inline void fillAndStrokeRoundRect(CDrawContext* context, const CRect& size, CCoord radius) {
		CGraphicsPath* path = context->createRoundRectGraphicsPath(size, radius);
		if (path) {
			context->drawGraphicsPath(path, CDrawContext::kPathFilled);
			context->drawGraphicsPath(path, CDrawContext::kPathStroked);
			path->forget();
		}
	}

	class KyunKawaiiBackgroundView : public CView {
	public:
		explicit KyunKawaiiBackgroundView (const CRect& size) : CView (size) {}

		void draw (CDrawContext* context) override {
			const double time = static_cast<double> (std::clock ()) / static_cast<double> (CLOCKS_PER_SEC);
			const auto r = getViewSize ();

			float peak = gPeakMeter.load(std::memory_order_relaxed);
			double colorTime = time + (peak * 2.0);

			CColor c_tl = getPastelColor(colorTime, 0.0);
			CColor c_tr = getPastelColor(colorTime, 0.25);
			CColor c_br = getPastelColor(colorTime, 0.5);
			CColor c_bl = getPastelColor(colorTime, 0.75);

			CGraphicsPath* pathH = context->createGraphicsPath();
			if (pathH) {
				pathH->addRect(r);
				CGradient* gradH = CGradient::create(0.0, 1.0, c_tl, c_tr);
				if (gradH) {
					context->fillLinearGradient(pathH, *gradH, r.getTopLeft(), r.getTopRight());
					gradH->forget();
				}
				pathH->forget();
			}
			
			CColor c_bl_trans = c_bl; c_bl_trans.alpha = 96;
			CColor c_br_trans = c_br; c_br_trans.alpha = 96;
			CGraphicsPath* pathV = context->createGraphicsPath();
			if (pathV) {
				pathV->addRect(r);
				CGradient* gradV = CGradient::create(0.0, 1.0, c_bl_trans, c_br_trans);
				if (gradV) {
					context->fillLinearGradient(pathV, *gradV, r.getTopLeft(), r.getBottomLeft());
					gradV->forget();
				}
				pathV->forget();
			}

			if (peak > 0.001f) {
				float boost = peak * 40.0f;
				for (int i = 0; i < 6; ++i) {
					double seed = i * 123.45;
					float x = r.left + r.getWidth() * (0.5f + 0.4f * std::sin(time * 0.15 + seed));
					float y = r.top + r.getHeight() * (0.5f + 0.4f * std::cos(time * 0.2 + seed * 2.0));
					float size = 30.0f + 10.0f * std::sin(time * 0.5 + seed) + boost;
					CColor color = CColor(255, 255, 255, 180);
					paintAmoeba(context, CPoint(x, y), size, color, time, seed);
				}
			}

			CRect panelRect(30, 30, 430, 310);
			
			context->setFillColor(CColor(255, 255, 255, 50));
			CRect shadowRect = panelRect;
			shadowRect.extend(4, 4);
			fillRoundRect(context, shadowRect, 24);
			
			context->setFillColor(CColor(255, 255, 255, 40));
			context->setFrameColor(CColor(255, 255, 255, 190));
			context->setLineWidth(1.5f);
			fillAndStrokeRoundRect(context, panelRect, 20);

			setDirty (false);
		}
	};

	class KawaiiSlider : public CSlider {
	public:
		KawaiiSlider(const CRect& size, IControlListener* listener, int32 tag)
			: CSlider(size, listener, tag, 0, 100, nullptr, nullptr, CPoint(0, 0), kHorizontal) {}

		void draw(CDrawContext* context) override {
			CRect r = getViewSize();
			
			CRect groove = r;
			groove.top = r.top + r.getHeight() * 0.4f;
			groove.bottom = r.bottom - r.getHeight() * 0.4f;
			
			context->setFillColor(CColor(255, 235, 245));
			context->setFrameColor(CColor(255, 120, 170));
			context->setLineWidth(1.0f);
			fillAndStrokeRoundRect(context, groove, groove.getHeight() / 2);
			
			CRect filledGroove = groove;
			float val = getValueNormalized();
			filledGroove.right = r.left + r.getWidth() * val;
			context->setFillColor(CColor(255, 160, 200));
			fillRoundRect(context, filledGroove, groove.getHeight() / 2);

			float handleSize = r.getHeight() * 0.7f;
			CPoint handleCenter;
			handleCenter.x = r.left + handleSize / 2 + (r.getWidth() - handleSize) * val;
			handleCenter.y = r.top + r.getHeight() / 2;
			
			CRect handleRect(handleCenter.x - handleSize/2, handleCenter.y - handleSize/2, handleCenter.x + handleSize/2, handleCenter.y + handleSize/2);
			
			context->setFillColor(CColor(255, 245, 250));
			context->setFrameColor(CColor(255, 120, 170));
			context->setLineWidth(1.5f);
			context->drawEllipse(handleRect, kDrawFilledAndStroked);
			
			setDirty(false);
		}
	};

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

Kyun2StopGUIEditor::Kyun2StopGUIEditor (void* controllerPtr)
	: VSTGUIEditor (controllerPtr)
{
	ViewRect viewRect (0, 0, 460, 340);
	setRect (viewRect);
}

bool PLUGIN_API Kyun2StopGUIEditor::open (void* parent, const PlatformType& platformType)
{
	if (frame)
		return false;

	CRect size (0, 0, 460, 340);
	frame = new CFrame (size, this);
	if (!frame)
		return false;

	frame->open (parent);

	backgroundView = new KyunKawaiiBackgroundView (size);
	frame->addView (backgroundView);

	CFontDesc* titleFont = new CFontDesc(*kNormalFontVeryBig);
	titleFont->setSize(28);
	
	CFontDesc* normFont = new CFontDesc(*kNormalFont);
	normFont->setSize(14);

	auto* title = new CTextLabel (CRect (30, 45, 430, 80), "Kyun'Stop");
	title->setFontColor (CColor (200, 80, 120, 255));
	title->setFont(titleFont);
	title->setBackColor(CColor(0,0,0,0));
	title->setFrameColor(CColor(0,0,0,0));
	title->setHoriAlign(kCenterText);
	frame->addView (title);
	titleFont->forget();

	// 1. CURVE
	auto* curveLabel = new CTextLabel(CRect(60, 95, 170, 120), "CURVE");
	curveLabel->setFont(normFont);
	curveLabel->setFontColor(CColor(110, 80, 100));
	curveLabel->setBackColor(CColor(0,0,0,0));
	curveLabel->setFrameColor(CColor(0,0,0,0));
	frame->addView(curveLabel);

	curveMenu = new COptionMenu(CRect(180, 92, 380, 122), this, PARAM_CURVE_TAG);
	curveMenu->addEntry("Linear");
	curveMenu->addEntry("Smooth");
	curveMenu->addEntry("SlowStart");
	curveMenu->addEntry("QuickCut");
	curveMenu->setBackColor(CColor(255, 235, 245));
	curveMenu->setFontColor(CColor(110, 80, 100));
	curveMenu->setFrameColor(CColor(255, 120, 170));
	curveMenu->setFont(normFont);
	frame->addView(curveMenu);

	// 2. SYNC
	auto* syncLabel = new CTextLabel(CRect(60, 135, 170, 160), "SYNC");
	syncLabel->setFont(normFont);
	syncLabel->setFontColor(CColor(110, 80, 100));
	syncLabel->setBackColor(CColor(0,0,0,0));
	syncLabel->setFrameColor(CColor(0,0,0,0));
	frame->addView(syncLabel);

	syncBox = new CCheckBox(CRect(180, 132, 280, 162), this, PARAM_USE_SYNC_TAG, "ON ♪");
	syncBox->setFont(normFont);
	syncBox->setFontColor(CColor(110, 80, 100));
	frame->addView(syncBox);

	// 3. STOP TIME
	auto* stopLabel = new CTextLabel(CRect(60, 175, 170, 200), "STOP TIME");
	stopLabel->setFont(normFont);
	stopLabel->setFontColor(CColor(110, 80, 100));
	stopLabel->setBackColor(CColor(0,0,0,0));
	stopLabel->setFrameColor(CColor(0,0,0,0));
	frame->addView(stopLabel);

	stopSlider = new KawaiiSlider(CRect(180, 175, 330, 195), this, PARAM_STOP_TIME_TAG);
	frame->addView(stopSlider);

	stopTimeLabel = new CTextLabel(CRect(340, 175, 400, 200), "0.50 s");
	stopTimeLabel->setFont(normFont);
	stopTimeLabel->setFontColor(CColor(110, 80, 100));
	stopTimeLabel->setBackColor(CColor(0,0,0,0));
	stopTimeLabel->setFrameColor(CColor(0,0,0,0));
	frame->addView(stopTimeLabel);

	// 4. START TIME
	auto* startLabel = new CTextLabel(CRect(60, 215, 170, 240), "START TIME");
	startLabel->setFont(normFont);
	startLabel->setFontColor(CColor(110, 80, 100));
	startLabel->setBackColor(CColor(0,0,0,0));
	startLabel->setFrameColor(CColor(0,0,0,0));
	frame->addView(startLabel);

	startSlider = new KawaiiSlider(CRect(180, 215, 330, 235), this, PARAM_START_TIME_TAG);
	frame->addView(startSlider);

	startTimeLabel = new CTextLabel(CRect(340, 215, 400, 240), "0.50 s");
	startTimeLabel->setFont(normFont);
	startTimeLabel->setFontColor(CColor(110, 80, 100));
	startTimeLabel->setBackColor(CColor(0,0,0,0));
	startTimeLabel->setFrameColor(CColor(0,0,0,0));
	frame->addView(startTimeLabel);

	// 5. TRIGGER
	CFontDesc* btnFont = new CFontDesc(*kNormalFont);
	btnFont->setSize(16);
	btnFont->setStyle(kBoldFace);

	triggerButton = new CTextButton(CRect(130, 255, 330, 295), this, PARAM_TRIGGER_TAG, "TAP TO STOP", CTextButton::kOnOffStyle);
	triggerButton->setRoundRadius(10.0f);
	triggerButton->setFrameWidth(1.5f);

	// 通常時 (OFF: TAP TO STOP) - 青
	triggerButton->setFrameColor(CColor(120, 180, 255));
	triggerButton->setTextColor(CColor(255, 255, 255));
	CGradient* gradNormal = CGradient::create(0.0, 1.0, CColor(120, 180, 255), CColor(120, 180, 255));
	if (gradNormal) {
		triggerButton->setGradient(gradNormal);
		gradNormal->forget();
	}

	// ハイライト時 (ON: Stopping...) - 赤ピンク
	triggerButton->setFrameColorHighlighted(CColor(255, 120, 140));
	triggerButton->setTextColorHighlighted(CColor(255, 255, 255));
	CGradient* gradHigh = CGradient::create(0.0, 1.0, CColor(255, 120, 140), CColor(255, 120, 140));
	if (gradHigh) {
		triggerButton->setGradientHighlighted(gradHigh);
		gradHigh->forget();
	}

	triggerButton->setFont(btnFont);
	frame->addView(triggerButton);

	btnFont->forget();
	normFont->forget();

	if (auto* controller = dynamic_cast<Kyun2StopController*> (getController ()))
	{
		controller->editor = this;
		float curveVal = static_cast<float> (controller->getParamNormalized (PARAM_CURVE_TAG));
		if (curveMenu) curveMenu->setValueNormalized (curveVal);

		float syncVal = static_cast<float> (controller->getParamNormalized (PARAM_USE_SYNC_TAG));
		if (syncBox) syncBox->setValueNormalized (syncVal);

		float stopVal = static_cast<float> (controller->getParamNormalized (PARAM_STOP_TIME_TAG));
		if (stopSlider) stopSlider->setValueNormalized (stopVal);
		stopTimeSec = normToSeconds (stopVal);
		char text[64];
		(void)std::snprintf (text, sizeof (text), "%.2f s", stopTimeSec);
		if (stopTimeLabel) stopTimeLabel->setText (text);

		float startVal = static_cast<float> (controller->getParamNormalized (PARAM_START_TIME_TAG));
		if (startSlider) startSlider->setValueNormalized (startVal);
		startTimeSec = normToSeconds (startVal);
		(void)std::snprintf (text, sizeof (text), "%.2f s", startTimeSec);
		if (startTimeLabel) startTimeLabel->setText (text);

		float triggerVal = static_cast<float> (controller->getParamNormalized (PARAM_TRIGGER_TAG));
		if (triggerButton) {
			triggerButton->setValueNormalized (triggerVal);
			bool stopping = (triggerVal > 0.5f);
			triggerButton->setTitle (stopping ? "Stopping..." : "TAP TO STOP");
		}
	}

	uiTimer = new VSTGUI::CVSTGUITimer ([this] (VSTGUI::CVSTGUITimer*) {
		updateControlStates ();
		if (frame)
			frame->invalid ();
	}, 33, true);

	return true;
}

void PLUGIN_API Kyun2StopGUIEditor::close ()
{
	if (auto* controller = dynamic_cast<Kyun2StopController*> (getController ()))
	{
		if (controller->editor == this)
			controller->editor = nullptr;
	}

	if (uiTimer) {
		uiTimer->stop ();
		uiTimer->forget ();
		uiTimer = nullptr;
	}

	backgroundView = nullptr;
	stopTimeLabel = nullptr;
	startTimeLabel = nullptr;
	curveMenu = nullptr;
	syncBox = nullptr;
	stopSlider = nullptr;
	startSlider = nullptr;
	triggerButton = nullptr;

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

void Kyun2StopGUIEditor::updateControlStates ()
{
	if (stopSlider && stopTimeLabel) {
		float stopVal = stopSlider->getValueNormalized();
		stopTimeSec = normToSeconds (stopVal);
		char text[64];
		(void)std::snprintf (text, sizeof (text), "%.2f s", stopTimeSec);
		if (strcmp (stopTimeLabel->getText(), text) != 0) {
			stopTimeLabel->setText (text);
			stopTimeLabel->invalid ();
		}
	}

	if (startSlider && startTimeLabel) {
		float startVal = startSlider->getValueNormalized();
		startTimeSec = normToSeconds (startVal);
		char text[64];
		(void)std::snprintf (text, sizeof (text), "%.2f s", startTimeSec);
		if (strcmp (startTimeLabel->getText(), text) != 0) {
			startTimeLabel->setText (text);
			startTimeLabel->invalid ();
		}
	}

	if (triggerButton) {
		bool stopping = (triggerButton->getValueNormalized() > 0.5f);
		const char* expectedTitle = stopping ? "Stopping..." : "TAP TO STOP";
		if (strcmp (triggerButton->getTitle(), expectedTitle) != 0) {
			triggerButton->setTitle (expectedTitle);
			triggerButton->invalid ();
		}
	}
}

void Kyun2StopGUIEditor::valueChanged (CControl* pControl)
{
	if (!pControl)
		return;

	const auto tag = static_cast<ParamID> (pControl->getTag ());
	float value = pControl->getValueNormalized ();
	sendParamNormalized (tag, value);

	char text[64];
	switch (tag)
	{
	case PARAM_STOP_TIME_TAG:
		stopTimeSec = normToSeconds (value);
		if (stopTimeLabel) {
			(void)std::snprintf (text, sizeof (text), "%.2f s", stopTimeSec);
			stopTimeLabel->setText (text);
			stopTimeLabel->invalid ();
		}
		break;

	case PARAM_START_TIME_TAG:
		startTimeSec = normToSeconds (value);
		if (startTimeLabel) {
			(void)std::snprintf (text, sizeof (text), "%.2f s", startTimeSec);
			startTimeLabel->setText (text);
			startTimeLabel->invalid ();
		}
		break;

	case PARAM_TRIGGER_TAG:
		if (triggerButton) {
			bool stopping = (value > 0.5f);
			triggerButton->setTitle (stopping ? "Stopping..." : "TAP TO STOP");
			triggerButton->invalid ();
		}
		break;

	default:
		break;
	}
}

void Kyun2StopGUIEditor::updateControl (ParamID tag, ParamValue value)
{
	switch (tag)
	{
	case PARAM_CURVE_TAG:
		if (curveMenu) {
			curveMenu->setValueNormalized (static_cast<float> (value));
			curveMenu->invalid ();
		}
		break;

	case PARAM_USE_SYNC_TAG:
		if (syncBox) {
			syncBox->setValueNormalized (static_cast<float> (value));
			syncBox->invalid ();
		}
		break;

	case PARAM_STOP_TIME_TAG:
		if (stopSlider) {
			stopSlider->setValueNormalized (static_cast<float> (value));
			stopSlider->invalid ();
		}
		break;

	case PARAM_START_TIME_TAG:
		if (startSlider) {
			startSlider->setValueNormalized (static_cast<float> (value));
			startSlider->invalid ();
		}
		break;

	case PARAM_TRIGGER_TAG:
		if (triggerButton) {
			triggerButton->setValueNormalized (static_cast<float> (value));
			bool stopping = (value > 0.5f);
			triggerButton->setTitle (stopping ? "Stopping..." : "TAP TO STOP");
			triggerButton->invalid ();
		}
		break;

	default:
		break;
	}
}

} }