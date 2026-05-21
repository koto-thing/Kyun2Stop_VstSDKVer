#pragma once

enum class TapeCurve {
    Linear = 0,
    Smooth = 1,
    SlowStart = 2,
    QuickCut = 3,
};

enum class SyncBeat {
    Eight = 0,
    Quarter = 1,
    Half = 2,
    OneBar = 3,
    TwoBars = 4,
};