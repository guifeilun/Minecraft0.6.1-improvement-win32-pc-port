#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS_DESKTOP_DesktopCraftingScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS_DESKTOP_DesktopCraftingScreen_H__

#include "../../Screen.h"
#include "../../../../world/item/ItemInstance.h"
#include <vector>

class CraftingContainer;
class Container;
class ImageButton;

// Java-style crafting screen: NxN grid + result slot + 27-slot main inventory + 9-slot hotbar.
// Holds a transient CraftingContainer and a cursor item; recipe matching reuses
// Recipes::getItemFor() so behaviour matches the existing in-engine crafting rules.
//
// gridSize = 2 -> player inventory crafting (E key)
// gridSize = 3 -> workbench crafting
class DesktopCraftingScreen : public Screen
{
    typedef Screen super;
public:
    explicit DesktopCraftingScreen(int gridSize);
    ~DesktopCraftingScreen();

    void init();
    void setupPositions();
    void render(int xm, int ym, float a);
    void tick();
    void mouseClicked(int x, int y, int buttonNum);
    void keyPressed(int eventKey);
    void removed();

    bool renderGameBehind() { return true; }
    bool closeOnPlayerHurt() { return true; }

private:
    struct Slot {
        Container* container;   // backing store
        int slotIndex;          // index in the container
        int x, y;               // top-left pixel position in screen coords
        bool isResult;          // true for the crafting output slot
    };

    int gridSize;
    CraftingContainer* craftingGrid;
    ItemInstance cursorItem;
    ItemInstance resultItem;
    std::vector<Slot> slots;
    int gridSlotsBegin;  // index in `slots` of the first crafting-grid slot
    int gridSlotsEnd;    // exclusive end index
    int resultSlot;      // index in `slots` of the result slot
    int armorSlotsBegin; // armor slots only present for the 2x2 (inventory) variant
    int armorSlotsEnd;
    int hotbarSlotsBegin;
    int hotbarSlotsEnd;
    int invSlotsBegin;
    int invSlotsEnd;

    int bgX, bgY, bgW, bgH;
    ImageButton* btnClose;

    // For double-click detection on the result slot.
    int lastClickSlotIdx;
    float lastClickTime;

    void rebuildSlots();
    void recomputeResult();
    int findSlotAt(int x, int y);
    void onSlotClick(int slotIdx, int buttonNum, bool shift);
    void giveResultOnce(bool toCursor);
    void shiftClickFrom(int slotIdx);
    bool tryDistributeStack(ItemInstance& stack, int rangeBegin, int rangeEnd);
    void returnGridItems();
    void renderSlotBg(int x, int y);
};

#endif
