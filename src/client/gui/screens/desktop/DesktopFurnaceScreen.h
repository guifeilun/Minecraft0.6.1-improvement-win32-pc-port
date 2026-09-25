#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS_DESKTOP_DesktopFurnaceScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS_DESKTOP_DesktopFurnaceScreen_H__

#include "../../Screen.h"
#include "../../../../world/item/ItemInstance.h"
#include <vector>

class Container;
class Player;
class FurnaceTileEntity;

// Java-style furnace UI: input + fuel slots on the left, an arrow + flame
// progress display in the middle, output slot on the right, then the player's
// main inventory and hotbar below.
class DesktopFurnaceScreen : public Screen
{
    typedef Screen super;
public:
    DesktopFurnaceScreen(Player* player, FurnaceTileEntity* furnace);
    ~DesktopFurnaceScreen();

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
    enum SlotKind {
        SLOT_NORMAL = 0,
        SLOT_FURNACE_INPUT,
        SLOT_FURNACE_FUEL,
        SLOT_FURNACE_OUTPUT,
    };

    struct Slot {
        Container* container;
        int slotIndex;
        int x, y;
        int kind;
    };

    Player* player;
    FurnaceTileEntity* furnace;
    ItemInstance cursorItem;
    std::vector<Slot> slots;

    // Index ranges (in `slots`)
    int furnaceSlotsBegin, furnaceSlotsEnd;
    int hotbarSlotsBegin, hotbarSlotsEnd;
    int invSlotsBegin, invSlotsEnd;

    int bgX, bgY, bgW, bgH;

    // Coords of the cook-arrow and flame indicators (computed in rebuildSlots).
    int arrowX, arrowY;
    int flameX, flameY;

    void rebuildSlots();
    int findSlotAt(int x, int y);
    void onSlotClick(int slotIdx, int buttonNum, bool shift);
    void shiftClickFrom(int slotIdx);
    bool tryDistributeStack(ItemInstance& stack, int rangeBegin, int rangeEnd);
    void renderSlotBg(int x, int y);
};

#endif
