#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS_DESKTOP_DesktopChestScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS_DESKTOP_DesktopChestScreen_H__

#include "../../Screen.h"
#include "../../../../world/item/ItemInstance.h"
#include <vector>

class Container;
class Player;
class ChestTileEntity;
class ImageButton;
class NinePatchLayer;

class DesktopChestScreen : public Screen
{
    typedef Screen super;
public:
    DesktopChestScreen(Player* player, ChestTileEntity* chest);
    ~DesktopChestScreen();

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
    };

    Player* player;
    ChestTileEntity* chest;
    ItemInstance cursorItem;
    std::vector<Slot> slots;

    int chestSlotsBegin, chestSlotsEnd;
    int hotbarSlotsBegin, hotbarSlotsEnd;
    int invSlotsBegin, invSlotsEnd;

    int bgX, bgY, bgW, bgH;

    ImageButton*    btnClose;
    NinePatchLayer* guiBackground;
    NinePatchLayer* guiSlot;
    NinePatchLayer* guiSlotMarker;

    void rebuildSlots();
    int findSlotAt(int x, int y);
    void onSlotClick(int slotIdx, int buttonNum, bool shift);
    void shiftClickFrom(int slotIdx);
    bool tryDistributeStack(ItemInstance& stack, int rangeBegin, int rangeEnd);
};

#endif