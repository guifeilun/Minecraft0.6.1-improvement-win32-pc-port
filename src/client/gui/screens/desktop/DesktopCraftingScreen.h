#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS_DESKTOP_DesktopCraftingScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS_DESKTOP_DesktopCraftingScreen_H__

#include "../../Screen.h"
#include "../../../../world/item/ItemInstance.h"
#include <vector>

class CraftingContainer;
class Container;
class ImageButton;
class NinePatchLayer;

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
    void buttonClicked(Button* button);
    void removed();

    bool renderGameBehind() { return true; }
    bool closeOnPlayerHurt() { return true; }

private:
    struct Slot {
        Container* container;
        int slotIndex;
        int x, y;
        bool isResult;
    };

    int gridSize;
    CraftingContainer* craftingGrid;
    ItemInstance cursorItem;
    ItemInstance resultItem;
    std::vector<Slot> slots;
    int gridSlotsBegin, gridSlotsEnd;
    int resultSlot;
    int armorSlotsBegin, armorSlotsEnd;
    int hotbarSlotsBegin, hotbarSlotsEnd;
    int invSlotsBegin, invSlotsEnd;

    int bgX, bgY, bgW, bgH;

    ImageButton*    btnClose;
    NinePatchLayer* guiBackground;
    NinePatchLayer* guiSlot;
    NinePatchLayer* guiSlotMarker;

    void rebuildSlots();
    void recomputeResult();
    int findSlotAt(int x, int y);
    ItemInstance* getSlotItem(const Slot& s);
    void onSlotClick(int slotIdx, int buttonNum, bool shift);
    void shiftClickFrom(int slotIdx);
    bool tryDistributeStack(ItemInstance& stack, int rangeBegin, int rangeEnd);
    void returnGridItems();
};

#endif