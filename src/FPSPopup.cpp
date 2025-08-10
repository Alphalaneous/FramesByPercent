#include "FPSPopup.hpp"

FPSCell* FPSCell::create(FPSPopup* popup, int percent, int fps) {
    auto ret = new FPSCell();
    if (ret->init(popup, percent, fps)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool FPSCell::init(FPSPopup* popup, int percent, int fps) {
    if (!CCLayer::init()) return false;
    m_popup = popup;
    m_percent = percent;
    m_fps = fps;
    
    setAnchorPoint({0.5f, 0.5f});
    ignoreAnchorPointForPosition(false);
    setContentSize({270, 30});

    auto background = CCScale9Sprite::create("square02b_001.png");
    background->setColor({0, 0, 0});
    background->setOpacity(127);
    background->setContentSize(getContentSize());
    background->setPosition(getContentSize()/2);

    addChild(background);

    CCLabelBMFont* percentLabel = CCLabelBMFont::create(fmt::format("Percent: {}", percent).c_str(), "bigFont.fnt");
    percentLabel->setScale(0.4f);
    percentLabel->setAnchorPoint({0.f, 0.5f});
    percentLabel->setPosition({10, getContentHeight()/2});

    addChild(percentLabel);

    auto fpsInput = geode::TextInput::create(50, "");
    fpsInput->setScale(0.6f);
    fpsInput->setString(std::to_string(m_fps));
    fpsInput->setCommonFilter(CommonFilter::Int);
    fpsInput->setPosition({getContentWidth() - 50, getContentHeight()/2});

    CCLabelBMFont* fpsLabel = CCLabelBMFont::create("FPS:", "bigFont.fnt");
    fpsLabel->setScale(0.4f);
    fpsLabel->setAnchorPoint({1.f, 0.5f});
    fpsLabel->setPosition({getContentWidth() - fpsInput->getScaledContentWidth() - 50, getContentHeight()/2});

    fpsInput->setCallback([this] (const std::string& str) {
        auto values = Mod::get()->getSavedValue<matjson::Value>(std::to_string(m_popup->m_id));
        auto res = numFromString<int>(str);
        if (res) {
            values[std::to_string(m_percent)] = res.unwrap();
            Mod::get()->setSavedValue(std::to_string(m_popup->m_id), values);
        }
    });

    addChild(fpsLabel);
    addChild(fpsInput);
    
    auto menu = CCMenu::create();
    menu->setContentSize({30, 30});
    menu->ignoreAnchorPointForPosition(false);
    menu->setAnchorPoint({1.f, 0.5f});
    menu->setPosition({getContentWidth(), getContentHeight()/2});

    auto delSpr = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
    delSpr->setScale(0.6f);
    auto btn = CCMenuItemSpriteExtra::create(delSpr, this, menu_selector(FPSCell::deleteSelf));
    btn->setPosition(menu->getContentSize()/2);

    menu->addChild(btn);

    addChild(menu);

    return true;
}

void FPSCell::deleteSelf(CCObject* sender) {
    createQuickPopup("Remove Percent?", "This will stop the FPS from being set for this percentage.", "Cancel", "Yes", [this] (FLAlertLayer*, bool selected) {
        if (selected) {
            auto values = Mod::get()->getSavedValue<matjson::Value>(std::to_string(m_popup->m_id));
            values.erase(std::to_string(m_percent));
            Mod::get()->setSavedValue(std::to_string(m_popup->m_id), values);
            m_popup->refreshCells();
        }
    }, true);
}

FPSPopup* FPSPopup::create(int id) {
    auto ret = new FPSPopup();
    if (ret->initAnchored(300.f, 280.f, id)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool FPSPopup::setup(int id) {

    setTitle("FPS by Percent");
    m_id = id;

    auto spr = CCSprite::createWithSpriteFrameName("GJ_newBtn_001.png");
    spr->setScale(0.6f);
    auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(FPSPopup::onCreateCell));

    btn->setPosition({m_mainLayer->getContentWidth() - 25, m_mainLayer->getContentHeight() - 25});

    m_buttonMenu->addChild(btn);

    auto background = CCScale9Sprite::create("square02b_001.png");
    background->setColor({0, 0, 0});
    background->setOpacity(127);
    background->setContentSize({280, 210});
    m_mainLayer->addChildAtPosition(background, Anchor::Center, {0, -20});

    m_myScollLayer = geode::ScrollLayer::create({270, 200});
    m_myScollLayer->m_contentLayer->setLayout(
        SimpleColumnLayout::create()
            ->setMainAxisDirection(AxisDirection::BottomToTop)
            ->setMainAxisAlignment(MainAxisAlignment::End)
            ->setMainAxisScaling(AxisScaling::Grow)
            ->setGap(2.5f)
    );

    m_myScollLayer->ignoreAnchorPointForPosition(false);
    m_myScollLayer->m_contentLayer->setContentHeight(220);
    m_mainLayer->addChildAtPosition(m_myScollLayer, Anchor::Center, {0, -20});

    refreshCells();
    return true;
}

void FPSPopup::refreshCells() {
    m_myScollLayer->m_contentLayer->removeAllChildren();
    auto values = Mod::get()->getSavedValue<matjson::Value>(std::to_string(m_id));
    for (const auto& [k, v] : values) {
        auto resPercent = numFromString<int>(k);
        if (resPercent) {
            auto resFPS = v.asInt();
            if (resFPS) {
                auto cell = FPSCell::create(this, resPercent.unwrap(), resFPS.unwrap());
                m_myScollLayer->m_contentLayer->addChild(cell);
            }
        }
    }
    m_myScollLayer->m_contentLayer->setContentHeight(220);
    m_myScollLayer->m_contentLayer->updateLayout();
    m_myScollLayer->scrollToTop();
}

void FPSPopup::onCreateCell(CCObject* sender) {
    auto manager = GameManager::get();
    auto values = Mod::get()->getSavedValue<matjson::Value>(std::to_string(m_id));

    int nextFreePercent = -1;
    for (int i = 0; i < 100; i++) {
        bool exists = false;
        for (const auto& [k, v] : values) {
            auto resPercent = numFromString<int>(k);
            if (resPercent) {
                if (i == resPercent.unwrap()) {
                    exists = true;
                }
            }
        }
        if (!exists) {
            nextFreePercent = i;
            break;
        }
    }

    if (nextFreePercent == -1) return;

    NumberPopup::create(this, nextFreePercent)->show();
}


NumberPopup* NumberPopup::create(FPSPopup* popup, int percent) {
    auto ret = new NumberPopup();
    if (ret->initAnchored(120.f, 110.f, popup, percent)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool NumberPopup::setup(FPSPopup* popup, int percent) {
    setTitle("Percent:");
    m_popup = popup;
    m_percent = percent;

    auto buttonSpr = ButtonSprite::create("Okay");
    buttonSpr->setCascadeColorEnabled(true);
    buttonSpr->setCascadeOpacityEnabled(true);
    auto btn = CCMenuItemSpriteExtra::create(buttonSpr, this, menu_selector(NumberPopup::onOkay));
    btn->setCascadeColorEnabled(true);
    btn->setCascadeOpacityEnabled(true);
    btn->setPosition({m_mainLayer->getContentWidth()/2, btn->getContentHeight()/2 + 10});

    auto percentInput = geode::TextInput::create(50, "0");
    percentInput->setScale(0.8f);
    percentInput->setCommonFilter(CommonFilter::Int);
    percentInput->setPosition({m_mainLayer->getContentWidth()/2, m_mainLayer->getContentHeight()/2 + 2.5f});
    percentInput->setCallback([this, btn] (const std::string& str) {
        auto res = numFromString<int>(str);
        if (res) {
            int percent = res.unwrap();
            bool isValidPercent = isValid(percent);
            if (isValidPercent) {
                btn->setColor({255, 255, 255});
                btn->setOpacity(255);
                m_percent = percent;
            }
            else {
                btn->setColor({200, 200, 200});
                btn->setOpacity(200);
            }
            m_isValid = isValidPercent;
            btn->setEnabled(isValidPercent);
        }
    });

    m_mainLayer->addChild(percentInput);
    m_buttonMenu->addChild(btn);
    return true;
}

void NumberPopup::onOkay(CCObject* sender) {
    if (!m_isValid) return;
    auto manager = GameManager::get();
    auto values = Mod::get()->getSavedValue<matjson::Value>(std::to_string(m_popup->m_id));
    values[std::to_string(m_percent)] = manager->m_customFPSTarget;
    Mod::get()->setSavedValue(std::to_string(m_popup->m_id), values);
    m_popup->refreshCells();
    keyBackClicked();
}

bool NumberPopup::isValid(int percent) {
    if (percent < 0 || percent >= 100) return false;

    auto values = Mod::get()->getSavedValue<matjson::Value>(std::to_string(m_popup->m_id));
    
    for (const auto& [k, v] : values) {
        auto resPercent = numFromString<int>(k);
        if (resPercent) {
            if (percent == resPercent.unwrap()) {
                return false;
            }
        }
    }
    
    return true;
}