#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__LanguageScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__LanguageScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include <string>
#include <vector>

class LanguageScreen: public Screen
{
    typedef Screen super;
public:
    LanguageScreen();
    virtual ~LanguageScreen();

    virtual void init();
    virtual void setupPositions();
    virtual void render(int xm, int ym, float a);
    virtual void buttonClicked(Button* button);

    void selectLanguage(int index);

    Touch::THeader* bHeader;
    Touch::TButton* bBack;
    std::vector<Touch::TButton*> langButtons;
    std::vector<std::string> langCodes;
    std::vector<std::string> langNames;
};

#endif