#include "DesktopChestScreen.h"

#include "../../../Minecraft.h"
#include "../../../player/LocalPlayer.h"
#include "../../../renderer/entity/ItemRenderer.h"
#include "../../../renderer/Tesselator.h"
#include "../../Font.h"
#include "../../Gui.h"
#include "../../components/ImageButton.h"
#include "../../components/NinePatch.h"
#include "../../../../world/entity/player/Player.h"
#include "../../../../world/entity/player/Inventory.h"
#include "../../../../world/inventory/FillingContainer.h"
#include "../../../../world/level/tile/entity/ChestTileEntity.h"
#include "../../../../world/item/Item.h"
#include "../../../../world/item/ItemInstance.h"
#include "../../../../locale/I18n.h"
#include "platform/input/Mouse.h"
#include "platform/input/Keyboard.h"

namespace {
const int SLOT_SIZE      = 18;
const int SLOT_INNER     = 16;
const int SLOT_PAD       = 1;

const int COLOR_TEXT     = 0xffe0e0e0;
const int COLOR_TOOLTIP_BG = 0xe0100010;
const int COLOR_TOOLTIP_FR = 0xff5000ff;

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

DesktopChestScreen::DesktopChestScreen(Player* player_, ChestTileEntity* chest_)
:   player(player_),
    chest(chest_),
    chestSlotsBegin(0), chestSlotsEnd(0),
    hotbarSlotsBegin(0), hotbarSlotsEnd(0),
    invSlotsBegin(0), invSlotsEnd(0),
    bgX(0), bgY(0), bgW(0), bgH(0),
    btnClose(NULL),
    guiBackground(NULL), guiSlot(NULL), guiSlotMarker(NULL)
{
    cursorItem.setNull();
}

DesktopChestScreen::~DesktopChestScreen() {
    delete btnClose;
    delete guiBackground;
    delete guiSlot;
    delete guiSlotMarker;
    if (chest && chest->clientSideOnly)
        delete chest;
}

void DesktopChestScreen::init() {
    super::init();
    rebuildSlots();

    NinePatchFactory builder(minecraft->textures, "gui/spritesheet.png");
    guiBackground = builder.createSymmetrical(IntRectangle(0, 0, 16, 16), 4, 4);
    guiSlot       = builder.createSymmetrical(IntRectangle(0, 32, 8, 8), 3, 3);
    guiSlotMarker = builder.createSymmetrical(IntRectangle(10, 42, 16, 16), 5, 5);

    if (!btnClose) {
        btnClose = new ImageButton(0, "");
        ImageDef def;
        def.name = "gui/spritesheet.png";
        def.x = 0; def.y = 1;
        def.width = def.height = 18;
        def.setSrc(IntRectangle(60, 0, 18, 18));
        btnClose->setImageDef(def, true);
        btnClose->scaleWhenPressed = false;
        buttons.push_back(btnClose);
    }
}

void DesktopChestScreen::setupPositions() {
    rebuildSlots();

    if (btnClose) {
        btnClose->width  = 19;
        btnClose->height = 19;
        btnClose->x = bgX + bgW - btnClose->width;
        btnClose->y = bgY;
    }
    if (guiBackground) guiBackground->setSize((float)bgW, (float)bgH);
}

void DesktopChestScreen::tick() {
    rebuildSlots();
}

void DesktopChestScreen::rebuildSlots() {
    if (!minecraft || !minecraft->player || !minecraft->player->inventory || !chest)
        return;
    Inventory* inv = minecraft->player->inventory;

    const int chestRows  = (chest->getContainerSize() + 8) / 9;
    const int chestCols  = 9;
    const int chestPx    = chestCols * SLOT_SIZE;
    const int innerPad   = 7;
    const int sectionGap = 6;
    const int titleH     = 12;
    const int hotbarGap  = 4;

    bgW = chestPx + 2 * innerPad;
    bgH = titleH + chestRows * SLOT_SIZE + sectionGap + 3 * SLOT_SIZE + hotbarGap + SLOT_SIZE + 2 * innerPad;

    bgX = (width  - bgW) / 2;
    bgY = (height - bgH) / 2;
    if (bgY < 22) bgY = 22;

    int contentX = bgX + innerPad;
    int contentY = bgY + innerPad + titleH;

    slots.clear();

    chestSlotsBegin = (int)slots.size();
    for (int row = 0; row < chestRows; ++row) {
        for (int col = 0; col < chestCols; ++col) {
            int slotIdx = row * 9 + col;
            if (slotIdx >= chest->getContainerSize()) break;
            Slot s;
            s.container = chest;
            s.slotIndex = slotIdx;
            s.x = contentX + col * SLOT_SIZE;
            s.y = contentY + row * SLOT_SIZE;
            slots.push_back(s);
        }
    }
    chestSlotsEnd = (int)slots.size();

    const int numLinked = inv->getNumLinkedSlots();
    bool isLinkedTarget[256] = { false };
    for (int h = 0; h < numLinked && h < 256; ++h) {
        int target = inv->linkedSlots[h].inventorySlot;
        if (target >= 0 && target < 256) isLinkedTarget[target] = true;
    }

    int invY = contentY + chestRows * SLOT_SIZE + sectionGap;
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
        slots.push_back(s);
    }
    hotbarSlotsEnd = (int)slots.size();
}

int DesktopChestScreen::findSlotAt(int x, int y) {
    for (int i = 0; i < (int)slots.size(); ++i) {
        const Slot& s = slots[i];
        if (x >= s.x && x < s.x + SLOT_SIZE && y >= s.y && y < s.y + SLOT_SIZE)
            return i;
    }
    return -1;
}

void DesktopChestScreen::render(int xm, int ym, float a) {
    Tesselator& t = Tesselator::instance;

    fill(0, 0, width, height, 0x60101020);

   guiBackground->setSize((float)bgW, (float)bgH);
   guiBackground->draw(t, (float)bgX, (float)bgY);

    minecraft->font->drawShadow(I18n::get("container.chest"), (float)(bgX + 8), (float)(bgY + 6), COLOR_TEXT);

    guiSlot->setSize((float)SLOT_SIZE, (float)SLOT_SIZE);
    for (int i = 0; i < (int)slots.size(); ++i)
        guiSlot->draw(t, (float)slots[i].x, (float)slots[i].y);

    for (int i = 0; i < (int)slots.size(); ++i) {
        const Slot& s = slots[i];
        if (s.slotIndex < 0) continue;
        ItemInstance* live = s.container->getItem(s.slotIndex);
        if (!live || live->isNull()) continue;
        ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, live,
            (float)(s.x + SLOT_PAD), (float)(s.y + SLOT_PAD), true);
    }

    int hov = findSlotAt(xm, ym);
    if (hov >= 0) {
        const Slot& s = slots[hov];
        guiSlotMarker->setSize((float)SLOT_SIZE + 4, (float)SLOT_SIZE + 4);
        guiSlotMarker->draw(t, (float)(s.x - 2), (float)(s.y - 2));
    }

    if (!cursorItem.isNull()) {
        ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, &cursorItem,
            (float)(xm - SLOT_INNER / 2), (float)(ym - SLOT_INNER / 2 + 1), true);
    }

    t.beginOverride();
    for (int i = 0; i < (int)slots.size(); ++i) {
        const Slot& s = slots[i];
        if (s.slotIndex < 0) continue;
        ItemInstance* live = s.container->getItem(s.slotIndex);
        if (!live || live->isNull()) continue;
        if (live->isDamaged()) {
            ItemRenderer::renderGuiItemDecorations(live, (float)(s.x + 1), (float)(s.y + 1));
        }
    }
    if (!cursorItem.isNull() && cursorItem.isDamaged()) {
        ItemRenderer::renderGuiItemDecorations(&cursorItem,
            (float)(xm - SLOT_INNER / 2 + 1), (float)(ym - SLOT_INNER / 2 + 1));
    }
    glDisable2(GL_TEXTURE_2D);
    t.endOverrideAndDraw();
    glEnable2(GL_TEXTURE_2D);

    for (int i = 0; i < (int)slots.size(); ++i) {
        const Slot& s = slots[i];
        if (s.slotIndex < 0) continue;
        ItemInstance* live = s.container->getItem(s.slotIndex);
        if (!live || live->isNull()) continue;
        minecraft->gui.renderSlotText(live, (float)(s.x + SLOT_PAD - 3), (float)(s.y + SLOT_PAD - 3), true, true);
    }
    if (!cursorItem.isNull()) {
        minecraft->gui.renderSlotText(&cursorItem,
            (float)(xm - SLOT_INNER / 2 - 3), (float)(ym - SLOT_INNER / 2 - 3), true, true);
    }

    super::render(xm, ym, a);

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
}

void DesktopChestScreen::mouseClicked(int x, int y, int buttonNum) {
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

void DesktopChestScreen::onSlotClick(int idx, int buttonNum, bool shift) {
    Slot& s = slots[idx];

    if (s.slotIndex < 0) {
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

    if (shift) {
        shiftClickFrom(idx);
        return;
    }

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
    } else {
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

bool DesktopChestScreen::tryDistributeStack(ItemInstance& stack, int rangeBegin, int rangeEnd) {
    bool moved = false;
    for (int i = rangeBegin; i < rangeEnd && !stack.isNull(); ++i) {
        Slot& s = slots[i];
        if (s.slotIndex < 0) continue;
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
        ItemInstance* live = s.container->getItem(s.slotIndex);
        if (live && !live->isNull()) continue;
        writeSlot(s.container, s.slotIndex, stack);
        stack.setNull();
        moved = true;
    }
    return moved;
}

void DesktopChestScreen::shiftClickFrom(int idx) {
    Slot& s = slots[idx];
    if (s.slotIndex < 0) return;

    ItemInstance* live = s.container->getItem(s.slotIndex);
    if (!live || live->isNull()) return;
    ItemInstance moving = *live;

    bool fromChest = (idx >= chestSlotsBegin && idx < chestSlotsEnd);
    if (fromChest) {
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

    ItemInstance remainder = moving;
    tryDistributeStack(remainder, chestSlotsBegin, chestSlotsEnd);
    writeSlot(s.container, s.slotIndex, remainder);
}

void DesktopChestScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE || eventKey == Keyboard::KEY_E) {
        if (minecraft->player) minecraft->player->closeContainer();
        return;
    }
    super::keyPressed(eventKey);
}

void DesktopChestScreen::buttonClicked(Button* button) {
    if (button == btnClose) {
        if (minecraft->player) minecraft->player->closeContainer();
        return;
    }
    super::buttonClicked(button);
}

void DesktopChestScreen::removed() {
    if (!cursorItem.isNull() && minecraft->player) {
        minecraft->player->drop(new ItemInstance(cursorItem), false);
        cursorItem.setNull();
    }
}