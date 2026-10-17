#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS_DESKTOP_DesktopCreativeScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS_DESKTOP_DesktopCreativeScreen_H__

#include "../../Screen.h"
#include "../../../../world/item/ItemInstance.h"
#include <vector>

class ImageButton;
class NinePatchLayer;

class DesktopCreativeScreen : public Screen
{
    typedef Screen super;
public:
    DesktopCreativeScreen();
    ~DesktopCreativeScreen();

    void init();
    void setupPositions();
    void render(int xm, int ym, float a);
    void tick();
    void mouseClicked(int x, int y, int buttonNum);
    void mouseReleased(int x, int y, int buttonNum);
    void mouseWheel(int dx, int dy, int xm, int ym);
    void keyPressed(int eventKey);
    void buttonClicked(Button* button);
    void removed();

    bool renderGameBehind() { return true; }
    bool closeOnPlayerHurt() { return true; }

private:
    void rebuildEntries();
    void layout();
    int  visibleRows() const;
    int  hitTestPaletteCell(int mx, int my) const;
    int  hitTestHotbar(int mx, int my) const;
    int  hitTestTab(int mx, int my) const;
    void giveToHotbar(const ItemInstance& item);
    void drawTooltip(int xm, int ym, const std::string& text);

    struct Entry {
        ItemInstance item;
        int category;
    };
    std::vector<Entry> entries;
    std::vector<int>   filtered;

    int activeCategory;
    int scrollOffsetRows;
    int hoveredEntryIdx;

    int bgX, bgY, bgW, bgH;
    int paletteX, paletteY, paletteW, paletteH;
    int paletteCols, paletteRowsVisible;
    int hotbarX, hotbarY;
    int tabsY;
    int tabW, tabH;
    struct Tab {
        std::string label;
        int category;
        int iconItemId;
        int x;
    };
    std::vector<Tab> tabs;
    int hoveredTabIdx;

    ImageButton*    btnClose;
    NinePatchLayer* guiBackground;
    NinePatchLayer* guiSlot;
    NinePatchLayer* guiSlotMarker;
    NinePatchLayer* guiTabNormal;
    NinePatchLayer* guiTabSelected;

    bool  trackingScrollGesture;
    bool  scrollingGesture;
    int   dragStartX, dragStartY, lastDragY;
    int   touchStartX, touchStartY;
};

#endif