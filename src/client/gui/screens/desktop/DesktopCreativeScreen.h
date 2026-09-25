#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS_DESKTOP_DesktopCreativeScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS_DESKTOP_DesktopCreativeScreen_H__

#include "../../Screen.h"
#include "../../../../world/item/ItemInstance.h"
#include <vector>

class ImageButton;

// PC-style creative inventory. Enumerates every registered Item and every
// placeable Tile at startup, groups them by ItemCategory, and renders them
// in a scrollable 9-wide grid with tabs across the top. Mouse: click an
// entry to put it into your selected hotbar slot. Right-click an entry to
// instead pick a single-count stack. Wheel scrolls; tab buttons switch the
// active category; Esc/E closes.
//
// Unlike the touch IngameBlockSelectionScreen which scrolls through a fixed
// pre-populated inventory, this screen synthesises ItemInstances on demand
// from the global Item::items[] table, so every item the game knows about
// is reachable — including the placeholder items we wired up in Item.cpp.
class DesktopCreativeScreen : public Screen
{
    typedef Screen super;
public:
    DesktopCreativeScreen();
    ~DesktopCreativeScreen();

    void init();
    void render(int xm, int ym, float a);
    void mouseClicked(int x, int y, int buttonNum);
    void mouseWheel(int dx, int dy, int xm, int ym);
    void keyPressed(int eventKey);
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

    struct Entry {
        ItemInstance item;
        int category;
    };
    // All known items/tiles, deduped and category-tagged.
    std::vector<Entry> entries;
    // Indices into `entries` filtered by the currently-selected tab.
    std::vector<int>   filtered;

    int activeCategory;  // ItemCategory:: value, or -1 = "All"
    int scrollOffsetRows;
    int hoveredEntryIdx; // index into `filtered`, -1 if none

    // Layout positions (computed in layout()).
    int bgX, bgY, bgW, bgH;
    int paletteX, paletteY, paletteW, paletteH;
    int paletteCols, paletteRowsVisible;
    int hotbarX, hotbarY;
    int tabsY;
    int tabW, tabH;
    // Tab definitions: each tab has a short label (used as a hover tooltip)
    // and a representative item id whose icon is drawn into the tab — keeps
    // the row narrow enough to fit on small windows without truncation.
    struct Tab {
        const char* label;
        int category;
        int iconItemId;
        int x;
    };
    std::vector<Tab> tabs;
    int hoveredTabIdx;
};

#endif
