#include "SimpleChooseLevelScreen.h"
#include "ProgressScreen.h"
#include "ScreenChooser.h"
#include "../components/Button.h"
#include "../components/ImageButton.h"
#include "../../Minecraft.h"
#include "../../../world/level/LevelSettings.h"
#include "../../../platform/time.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../platform/log.h"
#include "../../../locale/I18n.h"

SimpleChooseLevelScreen::SimpleChooseLevelScreen(const std::string& levelName)
:   bHeader(0),
    bGamemode(0),
    bWorldType(0),
    bCheats(0),
    bBack(0),
    bCreate(0),
    levelName(levelName),
    hasChosen(false),
    gamemode(GameType::Survival),
    worldType(WorldType::Old),
    cheatsEnabled(false),
    tLevelName(0, I18n::get("selectWorld.enterName")),
    tSeed(1, I18n::get("selectWorld.seedInfo"))
{
}

SimpleChooseLevelScreen::~SimpleChooseLevelScreen()
{
    if (bHeader) delete bHeader;
    delete bGamemode;
    delete bWorldType;
    delete bCheats;
    delete bBack;
    delete bCreate;
}

void SimpleChooseLevelScreen::init()
{
    ChooseLevelScreen::init();

    tLevelName.text = "New World";
    bHeader = new Touch::THeader(0, I18n::get("selectWorld.create"));
    bBack = new ImageButton(2, "");
    {
        ImageDef def;
        def.name = "gui/touchgui.png";
        def.width = 34;
        def.height = 26;
        def.setSrc(IntRectangle(150, 0, (int)def.width, (int)def.height));
        bBack->setImageDef(def, true);
    }
    if (/* minecraft->useTouchscreen() */ true) {
        bGamemode  = new Touch::TButton(1, I18n::get("gameMode.survival"));
        bCheats    = new Touch::TButton(4, "Cheats: Off");
        bWorldType = new Touch::TButton(5,
            I18n::get("selectWorld.mapType") + I18n::get("selectWorld.mapType.normal"));
        bCreate    = new Touch::TButton(3, I18n::get("selectWorld.create"));
    } else {
        bGamemode  = new Button(1, I18n::get("gameMode.survival"));
        bCheats    = new Button(4, "Cheats: Off");
        bWorldType = new Button(5,
            I18n::get("selectWorld.mapType") + I18n::get("selectWorld.mapType.normal"));
        bCreate    = new Button(3, I18n::get("selectWorld.create"));
    }

    buttons.push_back(bHeader);
    buttons.push_back(bBack);
    buttons.push_back(bGamemode);
    buttons.push_back(bCheats);
    buttons.push_back(bWorldType);
    buttons.push_back(bCreate);

    tabButtons.push_back(bGamemode);
    tabButtons.push_back(bCheats);
    tabButtons.push_back(bWorldType);
    tabButtons.push_back(bBack);
    tabButtons.push_back(bCreate);

    textBoxes.push_back(&tLevelName);
    textBoxes.push_back(&tSeed);
}

void SimpleChooseLevelScreen::setupPositions()
{
    int buttonHeight = bBack->height;

    bBack->x = width - bBack->width;
    bBack->y = 0;

    if (bHeader) {
        bHeader->x = 0;
        bHeader->y = 0;
        bHeader->width = width - bBack->width;
        bHeader->height = buttonHeight;
    }

    int centerX = width / 2;
    const int padding = 5;

    tLevelName.width = tSeed.width = 200;
    tLevelName.x = centerX - tLevelName.width / 2;
    tLevelName.y = buttonHeight + 20;

    tSeed.x = tLevelName.x;
    tSeed.y = tLevelName.y + 30;

    const int buttonWidth = 110;
    const int buttonSpacing = 8;
    const int totalButtonWidth = buttonWidth * 3 + buttonSpacing * 2;

    bGamemode->width  = buttonWidth;
    bCheats->width    = buttonWidth;
    bWorldType->width = buttonWidth;

    int rowLeft = centerX - totalButtonWidth / 2;
    bGamemode->x  = rowLeft;
    bCheats->x    = bGamemode->x + buttonWidth + buttonSpacing;
    bWorldType->x = bCheats->x   + buttonWidth + buttonSpacing;

    {
        int bottomPad = 20;
        int availTop = buttonHeight + 20 + 30 + 10;
        int availBottom = height - bottomPad - bCreate->height - 10;
        int availHeight = availBottom - availTop;
        if (availHeight < 0) availHeight = 0;

        int rowY = availTop + (availHeight - bGamemode->height) / 2;
        bGamemode->y  = rowY;
        bCheats->y    = rowY;
        bWorldType->y = rowY;
    }

    bCreate->width = 100;
    bCreate->x = centerX - bCreate->width / 2;
    int bottomPadding = 20;
    bCreate->y = height - bottomPadding - bCreate->height;
}

void SimpleChooseLevelScreen::tick()
{
    for (auto* tb : textBoxes)
        tb->tick(minecraft);
}

void SimpleChooseLevelScreen::render( int xm, int ym, float a )
{
    renderDirtBackground(0);
    glEnable2(GL_BLEND);

    const char* modeDesc = NULL;
    if (gamemode == GameType::Survival) {
        modeDesc = "Mobs, health and gather resources";
    } else if (gamemode == GameType::Creative) {
        modeDesc = "Unlimited resources and flying";
    }
    if (modeDesc) {
        drawCenteredString(minecraft->font, modeDesc, width / 2,
                           bGamemode->y + bGamemode->height + 4, 0xffcccccc);
    }

    {
        std::string nameLabel = I18n::get("selectWorld.enterName");
        std::string seedLabel = I18n::get("selectWorld.enterSeed");
        drawString(minecraft->font, nameLabel.c_str(), tLevelName.x,
                   tLevelName.y - Font::DefaultLineHeight - 2, 0xffcccccc);
        drawString(minecraft->font, seedLabel.c_str(), tSeed.x,
                   tSeed.y - Font::DefaultLineHeight - 2, 0xffcccccc);
    }

    Screen::render(xm, ym, a);
    glDisable2(GL_BLEND);
}

void SimpleChooseLevelScreen::mouseClicked(int x, int y, int buttonNum)
{
    if (buttonNum == MouseAction::ACTION_LEFT) {
        int lvlTop = tLevelName.y - (Font::DefaultLineHeight + 4);
        int lvlBottom = tLevelName.y + tLevelName.height;
        int lvlLeft = tLevelName.x;
        int lvlRight = tLevelName.x + tLevelName.width;
        bool clickedLevel = x >= lvlLeft && x < lvlRight && y >= lvlTop && y < lvlBottom;

        int seedTop = tSeed.y - (Font::DefaultLineHeight + 4);
        int seedBottom = tSeed.y + tSeed.height;
        int seedLeft = tSeed.x;
        int seedRight = tSeed.x + tSeed.width;
        bool clickedSeed  = x >= seedLeft && x < seedRight && y >= seedTop && y < seedBottom;

        if (clickedLevel) {
            LOGI("SimpleChooseLevelScreen: level textbox clicked (%d,%d)\n", x, y);
            tLevelName.setFocus(minecraft);
            tSeed.loseFocus(minecraft);
        } else if (clickedSeed) {
            LOGI("SimpleChooseLevelScreen: seed textbox clicked (%d,%d)\n", x, y);
            tSeed.setFocus(minecraft);
            tLevelName.loseFocus(minecraft);
        } else {
            tLevelName.loseFocus(minecraft);
            tSeed.loseFocus(minecraft);
        }
    }

    Screen::mouseClicked(x, y, buttonNum);
}

void SimpleChooseLevelScreen::buttonClicked( Button* button )
{
    if (hasChosen)
        return;

    if (button == bGamemode) {
        gamemode ^= 1;
        bGamemode->msg = (gamemode == GameType::Survival)
            ? I18n::get("gameMode.survival")
            : I18n::get("gameMode.creative");
        return;
    }

    if (button == bCheats) {
        cheatsEnabled = !cheatsEnabled;
        bCheats->msg = cheatsEnabled ? "Cheats: On" : "Cheats: Off";
        return;
    }

    if (button == bWorldType) {
        worldType = (worldType == WorldType::Old) ? WorldType::Infinite : WorldType::Old;
        bWorldType->msg = I18n::get("selectWorld.mapType") +
            ((worldType == WorldType::Infinite)
                ? I18n::get("enchantment.arrowInfinite")
                : I18n::get("selectWorld.mapType.normal"));
        return;
    }

    if (button == bCreate && !tLevelName.text.empty()) {
        int seed = getEpochTimeS();
        if (!tSeed.text.empty()) {
            std::string seedString = Util::stringTrim(tSeed.text);
            int tmpSeed;
            if (sscanf(seedString.c_str(), "%d", &tmpSeed) > 0) {
                seed = tmpSeed;
            } else {
                seed = Util::hashCode(seedString);
            }
        }
        std::string levelId = getUniqueLevelName(tLevelName.text);
        LevelSettings settings(seed, gamemode, worldType, cheatsEnabled);
        minecraft->selectLevel(levelId, levelId, settings);
        minecraft->hostMultiplayer();
        minecraft->setScreen(new ProgressScreen());
        hasChosen = true;
        return;
    }

    if (button == bBack) {
        minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
    }
}

void SimpleChooseLevelScreen::keyPressed(int eventKey)
{
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
        return;
    }
    Screen::keyPressed(eventKey);
}

bool SimpleChooseLevelScreen::handleBackEvent(bool isDown) {
        if (!isDown)
                minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
        return true;
}