#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class FPSPopup : public geode::Popup<int> {
    geode::ScrollLayer* m_myScollLayer;
public:
    int m_id;
    static FPSPopup* create(int id);
    bool setup(int id) override;
    void refreshCells();
    void onCreateCell(CCObject* sender);
};

class NumberPopup : public geode::Popup<FPSPopup*, int> {
    FPSPopup* m_popup;
    bool m_isValid;
    int m_percent;
public:
    static NumberPopup* create(FPSPopup* popup, int percent);
    bool setup(FPSPopup* popup, int percent) override;
    bool isValid(int percent);
    void onOkay(CCObject* sender);
};

class FPSCell : public CCLayer {
    FPSPopup* m_popup;
    int m_percent;
    int m_fps;
public:
    static FPSCell* create(FPSPopup* popup, int percent, int fps);
    bool init(FPSPopup* popup, int percent, int fps);

    void deleteSelf(CCObject* sender);
};