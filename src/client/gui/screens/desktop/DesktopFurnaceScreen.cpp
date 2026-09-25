#include "DesktopFurnaceScreen.h"

#include "../../../Minecraft.h"
#include "../../../player/LocalPlayer.h"
#include "../../../renderer/entity/ItemRenderer.h"
#include "../../Font.h"
#include "../../Gui.h"
#include "../../../../world/entity/player/Player.h"
#include "../../../../world/entity/player/Inventory.h"
#include "../../../../world/inventory/FillingContainer.h"
#include "../../../../world/level/tile/entity/FurnaceTileEntity.h"
#include "../../../../world/item/Item.h"
#include "../../../../world/item/ItemInstance.h"
#include "platform/input/Mouse.h"
#include "platform/input/Keyboard.h"

namespace {
const int SLOT_SIZE      = 18;
const int SLOT_INNER     = 16;
const int SLOT_PAD       = 1;

const int COLOR_BG       = 0xc0202020;
const int COLOR_FRAME    = 0xff8b8b8b;
const int COLOR_SLOT     = 0xff5b5b5b;
const int COLOR_HOVER    = 0x80ffffff;
const int COLOR_TEXT     = 0xffe0e0e0;
const int COLOR_TOOLTIP_BG = 0xe0100010;
const int COLOR_TOOLTIP_FR = 0xff5000ff;
const int COLOR_ARROW_BG = 0xff404040;
const int COLOR_ARROW_FG = 0xffe0a040;  // amber, like Java's progress
const int COLOR_FLAME    = 0xffff8800;
const int COLOR_FLAME_BG = 0xff402010;

bool canMerge(const ItemInstance& a, const ItemInstance& b) {
    if (a.isNull() || b.isNull()) return false;
    if (a.id != b.id) return false;
    if (a.getAuxValue() != b.getAuxValue()) return false;
    if (!a.isStackable()) return false;
    return true;
}

void writeSlot(Container* c, int slotIndex, const ItemInstance& v) {
    ItemInstance* p = c->getItem(slotIndex);
    if (p) {
        *p = v;
    } else if (!v.isNull()) {
        ItemInstance copy = v;
        c->setItem(slotIndex, &copy);
    }
}
}

DesktopFurnaceScreen::DesktopFurnaceScreen(Player* player_, FurnaceTileEntity* furnace_)
:   player(player_),
    furnace(furnace_),
    furnaceSlotsBegin(0), furnaceSlotsEnd(0),
    hotbarSlotsBegin(0), hotbarSlotsEnd(0),
    invSlotsBegin(0), invSlotsEnd(0),
    bgX(0), bgY(0), bgW(0), bgH(0),
    arrowX(0), arrowY(0),
    flameX(0), flameY(0)
{
    cursorItem.setNull();
}

DesktopFurnaceScreen::~DesktopFurnaceScreen() {
}

void DesktopFurnaceScreen::init() {
    super::init();
    rebuildSlots();
}

void DesktopFurnaceScreen::setupPositions() { init(); }
void DesktopFurnaceScreen::tick() { rebuildSlots(); }

void DesktopFurnaceScreen::rebuildSlots() {
    if (!minecraft || !minecraft->player || !minecraft->player->inventory || !furnace)
        return;
    Inventory* inv = minecraft->player->inventory;

    const int invRowPx   = 9 * SLOT_SIZE;
    const int innerPad   = 7;
    const int sectionGap = 6;
    const int titleH     = 12;
    const int hotbarGap  = 4;
    const int furnaceTopH = 3 * SLOT_SIZE + 6;  // input/flame/fuel column height

    bgW = invRowPx + 2 * innerPad;
    bgH = titleH + furnaceTopH + sectionGap + 3 * SLOT_SIZE + hotbarGap + SLOT_SIZE + 2 * innerPad;

    bgX = (width  - bgW) / 2;
    bgY = (height - bgH) / 2;

    int contentX = bgX + innerPad;
    int contentY = bgY + innerPad + titleH;

    slots.clear();

    // Furnace column on the left
    int colX = contentX + 14;
    int inputY  = contentY;
    int fuelY   = contentY + 2 * SLOT_SIZE + 6;
    int outputX = colX + 4 * SLOT_SIZE;
    int outputY = contentY + SLOT_SIZE + 3;

    flameX = colX;
    flameY = contentY + SLOT_SIZE + 1;
    arrowX = colX + (int)(SLOT_SIZE * 1.5);
    arrowY = contentY + SLOT_SIZE + 5;

    furnaceSlotsBegin = (int)slots.size();
    {
        // Input
        Slot s; s.container = furnace; s.slotIndex = FurnaceTileEntity::SLOT_INGREDIENT;
        s.x = colX; s.y = inputY; s.kind = SLOT_FURNACE_INPUT; slots.push_back(s);
    }
    {
        // Fuel
        Slot s; s.container = furnace; s.slotIndex = FurnaceTileEntity::SLOT_FUEL;
        s.x = colX; s.y = fuelY; s.kind = SLOT_FURNACE_FUEL; slots.push_back(s);
    }
    {
        // Output
        Slot s; s.container = furnace; s.slotIndex = FurnaceTileEntity::SLOT_RESULT;
        s.x = outputX; s.y = outputY; s.kind = SLOT_FURNACE_OUTPUT; slots.push_back(s);
    }
    furnaceSlotsEnd = (int)slots.size();

    // Player inventory (link-aware)
    const int numLinked = inv->getNumLinkedSlots();
    bool isLinkedTarget[256] = { false };
    for (int h = 0; h < numLinked && h < 256; ++h) {
        int target = inv->linkedSlots[h].inventorySlot;
        if (target >= 0 && target < 256) isLinkedTarget[target] = true;
    }

    int invY = contentY + furnaceTopH + sectionGap;
    invSlotsBegin = (int)slots.size();
    int placed = 0;
    for (int playerSlot = numLinked; playerSlot < inv->getContainerSize() && placed < 27; ++playerSlot) {
        if (playerSlot < 256 && isLinkedTarget[playerSlot]) continue;
        int row = placed / 9;
        int col = placed % 9;
        Slot s;
        s.container = inv;
        s.slotIndex = playerSlot;
        s.x = contentX + col * SLOT_SIZE;
        s.y = invY + row * SLOT_SIZE;
        s.kind = SLOT_NORMAL;
        slots.push_back(s);
        ++placed;
    }
    invSlotsEnd = (int)slots.size();

    int hotbarY = invY + 3 * SLOT_SIZE + hotbarGap;
    hotbarSlotsBegin = (int)slots.size();
    for (int col = 0; col < 9 && col < numLinked; ++col) {
        int target = inv->linkedSlots[col].inventorySlot;
        Slot s;
        s.container = inv;
        s.slotIndex = (target >= 0) ? target : -1;
        s.x = contentX + col * SLOT_SIZE;
        s.y = hotbarY;
        s.kind = SLOT_NORMAL;
        slots.push_back(s);
    }
    hotbarSlotsEnd = (int)slots.size();
}

int DesktopFurnaceScreen::findSlotAt(int x, int y) {
    for (int i = 0; i < (int)slots.size(); ++i) {
        const Slot& s = slots[i];
        if (x >= s.x && x < s.x + SLOT_SIZE && y >= s.y && y < s.y + SLOT_SIZE)
            return i;
    }
    return -1;
}

void DesktopFurnaceScreen::renderSlotBg(int x, int y) {
    fill(x,             y,             x + SLOT_SIZE, y + SLOT_SIZE, COLOR_FRAME);
    fill(x + SLOT_PAD,  y + SLOT_PAD,  x + SLOT_SIZE - SLOT_PAD, y + SLOT_SIZE - SLOT_PAD, COLOR_SLOT);
}

void DesktopFurnaceScreen::render(int xm, int ym, float a) {
    fill(0, 0, width, height, 0x60101020);
    fill(bgX - 1, bgY - 1, bgX + bgW + 1, bgY + bgH + 1, COLOR_FRAME);
    fill(bgX,     bgY,     bgX + bgW,     bgY + bgH,     COLOR_BG);

    minecraft->font->drawShadow(std::string("Furnace"), (float)(bgX + 8), (float)(bgY + 6), COLOR_TEXT);

    for (int i = 0; i < (int)slots.size(); ++i)
        renderSlotBg(slots[i].x, slots[i].y);

    // Flame indicator (between input and fuel slots) — fills upward as the
    // remaining fuel time burns down.
    {
        int max = SLOT_SIZE;
        int litLeft = furnace ? furnace->getLitProgress(max) : 0;
        fill(flameX, flameY, flameX + SLOT_SIZE, flameY + SLOT_SIZE, COLOR_FLAME_BG);
        if (litLeft > 0) {
            int top = flameY + (SLOT_SIZE - litLeft);
            fill(flameX + 4, top, flameX + SLOT_SIZE - 4, flameY + SLOT_SIZE, COLOR_FLAME);
        }
    }

    // Cooking arrow (between input and output) — fills left-to-right.
    {
        int max = 2 * SLOT_SIZE;
        int progress = furnace ? furnace->getBurnProgress(max) : 0;
        int arrowH = SLOT_SIZE - 6;
        fill(arrowX, arrowY, arrowX + max, arrowY + arrowH, COLOR_ARROW_BG);
        if (progress > 0) {
            fill(arrowX, arrowY + 2, arrowX + progress, arrowY + arrowH - 2, COLOR_ARROW_FG);
        }
        // Arrowhead
        for (int i = 0; i < 5; ++i) {
            fill(arrowX + max - 5 + i, arrowY - i, arrowX + max - 5 + i + 1, arrowY + arrowH + i, COLOR_ARROW_BG);
        }
    }

    // Items
    for (int i = 0; i < (int)slots.size(); ++i) {
        const Slot& s = slots[i];
        if (s.slotIndex < 0) continue;
        ItemInstance* live = s.container->getItem(s.slotIndex);
        if (!live || live->isNull()) continue;
        ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, live,
            (float)(s.x + SLOT_PAD), (float)(s.y + SLOT_PAD + 1), true);
        minecraft->gui.renderSlotText(live, (float)(s.x + SLOT_PAD - 3), (float)(s.y + SLOT_PAD - 3), true, true);
    }

    int hov = findSlotAt(xm, ym);
    if (hov >= 0) {
        const Slot& s = slots[hov];
        fill(s.x + SLOT_PAD, s.y + SLOT_PAD, s.x + SLOT_SIZE - SLOT_PAD, s.y + SLOT_SIZE - SLOT_PAD, COLOR_HOVER);
    }

    if (!cursorItem.isNull()) {
        ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, &cursorItem,
            (float)(xm - SLOT_INNER / 2), (float)(ym - SLOT_INNER / 2 + 1), true);
        minecraft->gui.renderSlotText(&cursorItem,
            (float)(xm - SLOT_INNER / 2 - 3), (float)(ym - SLOT_INNER / 2 - 3), true, true);
    }

    if (hov >= 0 && cursorItem.isNull()) {
        const Slot& s = slots[hov];
        if (s.slotIndex >= 0) {
            ItemInstance* live = s.container->getItem(s.slotIndex);
            if (live && !live->isNull()) {
                std::string name = live->getName();
                int tw = (int)minecraft->font->width(name) + 6;
                int th = 12;
                int tx = xm + 8;
                int ty = ym - 14;
                if (tx + tw > width) tx = width - tw;
                if (ty < 0) ty = 0;
                fill(tx - 1, ty - 1, tx + tw + 1, ty + th + 1, COLOR_TOOLTIP_FR);
                fill(tx,     ty,     tx + tw,     ty + th,     COLOR_TOOLTIP_BG);
                minecraft->font->drawShadow(name, (float)(tx + 3), (float)(ty + 2), COLOR_TEXT);
            }
        }
    }

    super::render(xm, ym, a);
}

void DesktopFurnaceScreen::mouseClicked(int x, int y, int buttonNum) {
    if (buttonNum != MouseAction::ACTION_LEFT && buttonNum != MouseAction::ACTION_RIGHT) {
        super::mouseClicked(x, y, buttonNum);
        return;
    }

    rebuildSlots();
    int idx = findSlotAt(x, y);
    if (idx < 0) {
        bool outsidePanel = (x < bgX || x >= bgX + bgW || y < bgY || y >= bgY + bgH);
        if (outsidePanel && !cursorItem.isNull() && minecraft->player) {
            if (buttonNum == MouseAction::ACTION_LEFT) {
                minecraft->player->drop(new ItemInstance(cursorItem), false);
                cursorItem.setNull();
            } else {
                ItemInstance one = cursorItem;
                one.count = 1;
                minecraft->player->drop(new ItemInstance(one), false);
                cursorItem.count -= 1;
                if (cursorItem.count <= 0) cursorItem.setNull();
            }
        }
        super::mouseClicked(x, y, buttonNum);
        return;
    }

    bool shift = Keyboard::isKeyDown(Keyboard::KEY_LSHIFT);
    onSlotClick(idx, buttonNum, shift);
    super::mouseClicked(x, y, buttonNum);
}

void DesktopFurnaceScreen::onSlotClick(int idx, int buttonNum, bool shift) {
    Slot& s = slots[idx];

    // Empty hotbar sentinel
    if (s.kind == SLOT_NORMAL && s.slotIndex < 0) {
        if (cursorItem.isNull()) return;
        if (idx < hotbarSlotsBegin || idx >= hotbarSlotsEnd) return;
        int col = idx - hotbarSlotsBegin;

        Inventory* inv = minecraft->player->inventory;
        const int numLinked = inv->getNumLinkedSlots();
        bool isLinked[256] = { false };
        for (int h = 0; h < numLinked && h < 256; ++h) {
            int t = inv->linkedSlots[h].inventorySlot;
            if (t >= 0 && t < 256) isLinked[t] = true;
        }
        int freeSlot = -1;
        for (int i = numLinked; i < inv->getContainerSize(); ++i) {
            if (i < 256 && isLinked[i]) continue;
            ItemInstance* p = inv->getItem(i);
            if (!p || p->isNull()) { freeSlot = i; break; }
        }
        if (freeSlot < 0) return;

        ItemInstance toPlace = cursorItem;
        if (buttonNum == MouseAction::ACTION_RIGHT) {
            toPlace.count = 1;
            cursorItem.count -= 1;
            if (cursorItem.count <= 0) cursorItem.setNull();
        } else {
            cursorItem.setNull();
        }
        writeSlot(inv, freeSlot, toPlace);
        inv->linkSlot(col, freeSlot, false);
        return;
    }

    // Output slot: take-only. Cursor must be empty or hold the same item with room.
    if (s.kind == SLOT_FURNACE_OUTPUT) {
        ItemInstance* live = s.container->getItem(s.slotIndex);
        if (!live || live->isNull()) return;
        if (shift) {
            ItemInstance moving = *live;
            ItemInstance moving1 = moving;
            tryDistributeStack(moving1, hotbarSlotsBegin, hotbarSlotsEnd);
            if (!moving1.isNull())
                tryDistributeStack(moving1, invSlotsBegin, invSlotsEnd);
            int placed = moving.count - moving1.count;
            if (placed > 0) {
                live->count -= placed;
                if (live->count <= 0) live->setNull();
                writeSlot(s.container, s.slotIndex, *live);
            }
            return;
        }
        if (cursorItem.isNull()) {
            cursorItem = *live;
            live->setNull();
            writeSlot(s.container, s.slotIndex, *live);
        } else if (canMerge(cursorItem, *live) &&
                   cursorItem.count + live->count <= cursorItem.getMaxStackSize()) {
            cursorItem.count += live->count;
            live->setNull();
            writeSlot(s.container, s.slotIndex, *live);
        }
        return;
    }

    if (shift) { shiftClickFrom(idx); return; }

    // Normal slot click logic (input/fuel/inventory/hotbar).
    ItemInstance* slotItemPtr = s.container->getItem(s.slotIndex);
    ItemInstance slotItem = (slotItemPtr && !slotItemPtr->isNull()) ? *slotItemPtr : ItemInstance();

    if (buttonNum == MouseAction::ACTION_LEFT) {
        if (cursorItem.isNull() && !slotItem.isNull()) {
            cursorItem = slotItem;
            writeSlot(s.container, s.slotIndex, ItemInstance());
        } else if (!cursorItem.isNull() && slotItem.isNull()) {
            writeSlot(s.container, s.slotIndex, cursorItem);
            cursorItem.setNull();
        } else if (!cursorItem.isNull() && !slotItem.isNull()) {
            if (canMerge(cursorItem, slotItem)) {
                int max = slotItem.getMaxStackSize();
                int room = max - slotItem.count;
                int give = (cursorItem.count < room) ? cursorItem.count : room;
                if (give > 0) {
                    slotItem.count += give;
                    cursorItem.count -= give;
                    if (cursorItem.count <= 0) cursorItem.setNull();
                    writeSlot(s.container, s.slotIndex, slotItem);
                } else {
                    writeSlot(s.container, s.slotIndex, cursorItem);
                    cursorItem = slotItem;
                }
            } else {
                writeSlot(s.container, s.slotIndex, cursorItem);
                cursorItem = slotItem;
            }
        }
    } else {  // RIGHT
        if (cursorItem.isNull() && !slotItem.isNull()) {
            int take = (slotItem.count + 1) / 2;
            cursorItem = slotItem;
            cursorItem.count = take;
            slotItem.count -= take;
            if (slotItem.count <= 0) slotItem.setNull();
            writeSlot(s.container, s.slotIndex, slotItem);
        } else if (!cursorItem.isNull() && slotItem.isNull()) {
            ItemInstance one = cursorItem;
            one.count = 1;
            writeSlot(s.container, s.slotIndex, one);
            cursorItem.count -= 1;
            if (cursorItem.count <= 0) cursorItem.setNull();
        } else if (!cursorItem.isNull() && !slotItem.isNull()) {
            if (canMerge(cursorItem, slotItem) && slotItem.count < slotItem.getMaxStackSize()) {
                slotItem.count += 1;
                cursorItem.count -= 1;
                if (cursorItem.count <= 0) cursorItem.setNull();
                writeSlot(s.container, s.slotIndex, slotItem);
            } else if (!canMerge(cursorItem, slotItem)) {
                writeSlot(s.container, s.slotIndex, cursorItem);
                cursorItem = slotItem;
            }
        }
    }
}

bool DesktopFurnaceScreen::tryDistributeStack(ItemInstance& stack, int rangeBegin, int rangeEnd) {
    bool moved = false;
    for (int i = rangeBegin; i < rangeEnd && !stack.isNull(); ++i) {
        Slot& s = slots[i];
        if (s.slotIndex < 0) continue;
        if (s.kind == SLOT_FURNACE_OUTPUT) continue;  // never auto-fill the output slot
        ItemInstance* live = s.container->getItem(s.slotIndex);
        if (!live || live->isNull()) continue;
        if (!canMerge(stack, *live)) continue;
        int max  = live->getMaxStackSize();
        int room = max - live->count;
        if (room <= 0) continue;
        int give = (stack.count < room) ? stack.count : room;
        live->count += give;
        stack.count -= give;
        if (stack.count <= 0) stack.setNull();
        writeSlot(s.container, s.slotIndex, *live);
        moved = true;
    }
    for (int i = rangeBegin; i < rangeEnd && !stack.isNull(); ++i) {
        Slot& s = slots[i];
        if (s.slotIndex < 0) continue;
        if (s.kind == SLOT_FURNACE_OUTPUT) continue;
        ItemInstance* live = s.container->getItem(s.slotIndex);
        if (live && !live->isNull()) continue;
        writeSlot(s.container, s.slotIndex, stack);
        stack.setNull();
        moved = true;
    }
    return moved;
}

void DesktopFurnaceScreen::shiftClickFrom(int idx) {
    Slot& s = slots[idx];
    if (s.slotIndex < 0) return;

    ItemInstance* live = s.container->getItem(s.slotIndex);
    if (!live || live->isNull()) return;
    ItemInstance moving = *live;

    // From any furnace slot -> player inventory (hotbar first, then main).
    bool fromFurnace = (idx >= furnaceSlotsBegin && idx < furnaceSlotsEnd);
    if (fromFurnace) {
        ItemInstance moving1 = moving;
        tryDistributeStack(moving1, hotbarSlotsBegin, hotbarSlotsEnd);
        if (!moving1.isNull())
            tryDistributeStack(moving1, invSlotsBegin, invSlotsEnd);
        int placed = moving.count - moving1.count;
        if (placed <= 0) return;
        live->count -= placed;
        if (live->count <= 0) live->setNull();
        writeSlot(s.container, s.slotIndex, *live);
        return;
    }

    // From player side: prefer fuel slot if the item is fuel, otherwise input.
    bool isFuel = FurnaceTileEntity::isFuel(moving);
    int targetSlot = isFuel ? FurnaceTileEntity::SLOT_FUEL : FurnaceTileEntity::SLOT_INGREDIENT;

    // Find that furnace slot in our list.
    int destIdx = -1;
    for (int i = furnaceSlotsBegin; i < furnaceSlotsEnd; ++i) {
        if (slots[i].slotIndex == targetSlot) { destIdx = i; break; }
    }
    if (destIdx < 0) return;

    ItemInstance remainder = moving;
    tryDistributeStack(remainder, destIdx, destIdx + 1);
    writeSlot(s.container, s.slotIndex, remainder);
}

void DesktopFurnaceScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE || eventKey == Keyboard::KEY_E) {
        if (minecraft->player) minecraft->player->closeContainer();
        return;
    }
    super::keyPressed(eventKey);
}

void DesktopFurnaceScreen::removed() {
    if (!cursorItem.isNull() && minecraft->player) {
        minecraft->player->drop(new ItemInstance(cursorItem), false);
        cursorItem.setNull();
    }
}
