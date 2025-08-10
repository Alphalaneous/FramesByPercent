#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "FPSPopup.hpp"

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {

    struct Fields {
        std::unordered_map<int, int> m_percentPoints;
        int m_originalFPS;
        bool m_fpsSet = false;
    };

    void setFPS(int fps) {
        auto fields = m_fields.self();
        if (!fields->m_fpsSet) return;
        auto manager = GameManager::get();
        manager->m_customFPSTarget = fps;
        manager->updateCustomFPS();
    }

    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        auto fields = m_fields.self();
        auto manager = GameManager::get();
        fields->m_originalFPS = manager->m_customFPSTarget;
        fields->m_fpsSet = true;
        refreshPercentPoints();
        schedule(schedule_selector(MyPlayLayer::checkPercent));
    }

    void onQuit() {
        auto fields = m_fields.self();
        setFPS(fields->m_originalFPS);
        PlayLayer::onQuit();
    }

    void resetLevel() {
        auto fields = m_fields.self();
        setFPS(fields->m_originalFPS);
        PlayLayer::resetLevel();
    }

    void levelComplete() {
        auto fields = m_fields.self();
        setFPS(fields->m_originalFPS);
        PlayLayer::levelComplete();
    }

    void resume() {
        refreshPercentPoints();
        PlayLayer::resume();
    }

    void resumeAndRestart(bool p0) {
        refreshPercentPoints();
        PlayLayer::resumeAndRestart(p0);
    }

    void checkPercent(float dt) {
        auto self = reinterpret_cast<PlayLayer*>(this);
        auto fields = m_fields.self();
        int percent = self->getCurrentPercentInt();

        for (const auto& [k, v] : fields->m_percentPoints) {
            if (percent == k) {
                setFPS(v);
                return;
            }
        }
    }

    void refreshPercentPoints() {
        auto self = reinterpret_cast<PlayLayer*>(this);
        auto fields = m_fields.self();
        fields->m_percentPoints.clear();

        auto values = Mod::get()->getSavedValue<matjson::Value>(std::to_string(self->m_level->m_levelID));
        for (const auto& [k, v] : values) {
            auto resPercent = numFromString<int>(k);
            if (resPercent) {
                auto resFPS = v.asInt();
                if (resFPS) {
                    fields->m_percentPoints[resPercent.unwrap()] = resFPS.unwrap();
                }
            }
        }
    }
};

class $modify(MyPauseLayer, PauseLayer) {

    void customSetup() {
        PauseLayer::customSetup();
        auto label = CCLabelBMFont::create("\%FPS", "bigFont.fnt");
        auto spr = CircleButtonSprite::create(label, CircleBaseColor::Green, CircleBaseSize::MediumAlt);
        spr->setScale(0.6f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(MyPauseLayer::onFpsPercent));
        if (auto menu = getChildByID("right-button-menu")) {
            menu->addChild(btn);
        }
    }

    void onFpsPercent(CCObject* sender) {
        FPSPopup::create(PlayLayer::get()->m_level->m_levelID)->show();
    }
};