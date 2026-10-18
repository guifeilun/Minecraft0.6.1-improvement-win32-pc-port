#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS_DESKTOP_DesktopFurnaceScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS_DESKTOP_DesktopFurnaceScreen_H__

#include "../../Screen.h"
#include "../../../../world/item/ItemInstance.h"
#include <vector>

class Container;
class Player;
class FurnaceTileEntity;
class ImageButton;
class NinePatchLayer;

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
    void buttonClicked(Button* button);
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

    int furnaceSlotsBegin, furnaceSlotsEnd;
    int hotbarSlotsBegin, hotbarSlotsEnd;
    int invSlotsBegin, invSlotsEnd;

    int bgX, bgY, bgW, bgH;

    ImageButton*    btnClose;
    NinePatchLayer* guiBackground;
    NinePatchLayer* guiSlot;
    NinePatchLayer* guiSlotMarker;

    int arrowX, arrowY;
    int flameX, flameY;

    int contentY_arrow_y();

    void rebuildSlots();
    int findSlotAt(int x, int y);
    void onSlotClick(int slotIdx, int buttonNum, bool shift);
    void shiftClickFrom(int slotIdx);
    bool tryDistributeStack(ItemInstance& stack, int rangeBegin, int rangeEnd);
};

#endif