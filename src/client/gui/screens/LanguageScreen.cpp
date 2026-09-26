#include "LanguageScreen.h"
#include "../../Minecraft.h"
#include "../../../locale/I18n.h"

LanguageScreen::LanguageScreen() {
    bHeader = 0;
    bBack = 0;
}

LanguageScreen::~LanguageScreen() {
    if (bHeader) { delete bHeader; bHeader = 0; }
    if (bBack) { delete bBack; bBack = 0; }
    for (size_t i = 0; i < langButtons.size(); i++)
        delete langButtons[i];
    langButtons.clear();
}

void LanguageScreen::init() {
    langCodes.clear();
    langNames.clear();
    std::vector<std::string> codes = I18n::availableLanguages(minecraft->platform());
    for (size_t i = 0; i < codes.size(); i++) {
        langCodes.push_back(codes[i]);
        langNames.push_back(I18n::languageDisplayName(minecraft->platform(), codes[i]));
    }

    bHeader = new Touch::THeader(0, I18n::get("options.language"));
    bBack = new Touch::TButton(1, I18n::get("gui.done"));
    buttons.push_back(bHeader);
    buttons.push_back(bBack);

    for (int i = 0; i < (int)langCodes.size(); i++) {
        Touch::TButton* btn = new Touch::TButton(10 + i, langNames[i]);
        buttons.push_back(btn);
        langButtons.push_back(btn);
    }
}

void LanguageScreen::setupPositions() {
    bHeader->x = 0;
    bHeader->y = 0;
    bHeader->width = width;
    bHeader->height = 20;
    bBack->x = 4;
    bBack->y = 4;
    bBack->width = 40;
    bBack->height = 18;

    int y = 30;
    for (size_t i = 0; i < langButtons.size(); i++) {
        langButtons[i]->x = width / 2 - 100;
        langButtons[i]->y = y;
        langButtons[i]->width = 200;
        langButtons[i]->height = 20;
        y += 24;
    }
}

void LanguageScreen::render(int xm, int ym, float a) {
    renderBackground(0);
    Screen::render(xm, ym, a);
}

void LanguageScreen::buttonClicked(Button* button) {
    if (button == bBack) {
        minecraft->setScreen(NULL);
        return;
    }
    for (size_t i = 0; i < langButtons.size(); i++) {
        if (button == langButtons[i]) {
            selectLanguage(i);
            return;
        }
    }
}

void LanguageScreen::selectLanguage(int index) {
    if (index < 0 || index >= (int)langCodes.size()) return;
    if (minecraft->options.language == langCodes[index]) return;
    minecraft->options.language = langCodes[index];
    minecraft->options.set(OPTIONS_LANGUAGE, langCodes[index]);
    I18n::loadLanguage(minecraft->platform(), langCodes[index]);
    minecraft->options.save();
    minecraft->setScreen(new LanguageScreen());
}