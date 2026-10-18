#include "DesktopCraftingScreen.h"

#include "../../../Minecraft.h"
#include "../../../player/LocalPlayer.h"
#include "../../../renderer/Tesselator.h"
#include "../../../renderer/entity/ItemRenderer.h"
#include "../../Font.h"
#include "../../Gui.h"
#include "../../components/ImageButton.h"
#include "../../components/NinePatch.h"
#include "../../../../world/inventory/CraftingContainer.h"
#include "../../../../world/entity/player/Inventory.h"
#include "../../../../world/entity/player/Player.h"
#include "../../../../world/item/Item.h"
#include "../../../../world/item/ItemInstance.h"
#include "../../../../locale/I18n.h"
#include "../../../../world/item/ArmorItem.h"
#include "../../../../world/item/crafting/Recipe.h"
#include "../../../../world/item/crafting/Recipes.h"
#include "platform/input/Mouse.h"
#include "platform/input/Keyboard.h"

namespace {
const int SLOT_SIZE      = 18;
const int SLOT_INNER     = 16;
const int SLOT_PAD       = 1;
const int RESULT_GAP     = 30;

const int COLOR_TEXT     = 0xffe0e0e0;
const int COLOR_TOOLTIP_BG = 0xe0100010;
const int COLOR_TOOLTIP_FR = 0xff5000ff;

int min_i(int a, int b) { return a < b ? a : b; }
int max_i(int a, int b) { return a > b ? a : b; }

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

DesktopCraftingScreen::DesktopCraftingScreen(int gridSize_)
:   gridSize(gridSize_),
    craftingGrid(NULL),
    gridSlotsBegin(0),
    gridSlotsEnd(0),
    resultSlot(-1),
    armorSlotsBegin(0),
    armorSlotsEnd(0),
    hotbarSlotsBegin(0),
    hotbarSlotsEnd(0),
    invSlotsBegin(0),
    invSlotsEnd(0),
    bgX(0), bgY(0), bgW(0), bgH(0),
    btnClose(NULL),
    guiBackground(NULL), guiSlot(NULL), guiSlotMarker(NULL)
{
    craftingGrid = new CraftingContainer(gridSize, gridSize);
    cursorItem.setNull();
    resultItem.setNull();
}

DesktopCraftingScreen::~DesktopCraftingScreen() {
    delete craftingGrid;
    delete btnClose;
    delete guiBackground;
    delete guiSlot;
    delete guiSlotMarker;
}

void DesktopCraftingScreen::init() {
    super::init();
    rebuildSlots();
    recomputeResult();

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

void DesktopCraftingScreen::setupPositions() {
    rebuildSlots();
    recomputeResult();

    if (btnClose) {
        btnClose->width  = 19;
        btnClose->height = 19;
        btnClose->x = bgX + bgW - btnClose->width;
        btnClose->y = bgY;
    }
    if (guiBackground) guiBackground->setSize((float)bgW, (float)bgH);
}

void DesktopCraftingScreen::rebuildSlots() {
    if (!minecraft || !minecraft->player || !minecraft->player->inventory)
        return;
    Inventory* inv = minecraft->player->inventory;

    const int gridPx     = gridSize * SLOT_SIZE;
    const int invRowPx   = 9 * SLOT_SIZE;
    const int innerPad   = 7;
    const int sectionGap = 6;
    const int titleH     = 12;
    const int hotbarGap  = 4;

    bool showArmor = (gridSize == 2);

    const int armorW = showArmor ? (SLOT_SIZE + 6) : 0;
    const int topSectionH = showArmor ? max_i(4 * SLOT_SIZE, gridPx) : gridPx;

    bgW = max_i(invRowPx, armorW + gridPx + RESULT_GAP + SLOT_SIZE) + 2 * innerPad;
    bgH = titleH + topSectionH + sectionGap + 3 * SLOT_SIZE + hotbarGap + SLOT_SIZE + 2 * innerPad;

    bgX = (width  - bgW) / 2;
    bgY = (height - bgH) / 2;
    if (bgY < 22) bgY = 22;

    int contentX = bgX + innerPad;
    int contentY = bgY + innerPad + titleH;

    slots.clear();

    armorSlotsBegin = (int)slots.size();
    if (showArmor) {
        for (int i = 0; i < 4; ++i) {
            Slot s;
            s.container = NULL;
            s.slotIndex = i;
            s.x = contentX;
            s.y = contentY + i * SLOT_SIZE;
            s.isResult = false;
            slots.push_back(s);
        }
        contentX += armorW;
    }
    armorSlotsEnd = (int)slots.size();

    int gridYOffset = (topSectionH - gridPx) / 2;

    gridSlotsBegin = (int)slots.size();
    for (int row = 0; row < gridSize; ++row) {
        for (int col = 0; col < gridSize; ++col) {
            Slot s;
            s.container = craftingGrid;
            s.slotIndex = row * gridSize + col;
            s.x = contentX + col * SLOT_SIZE;
            s.y = contentY + gridYOffset + row * SLOT_SIZE;
            s.isResult = false;
            slots.push_back(s);
        }
    }
    gridSlotsEnd = (int)slots.size();

    {
        Slot s;
        s.container = craftingGrid;
        s.slotIndex = -1;
        s.x = contentX + gridPx + RESULT_GAP;
        s.y = contentY + gridYOffset + (gridPx - SLOT_SIZE) / 2;
        s.isResult = true;
        resultSlot = (int)slots.size();
        slots.push_back(s);
    }

    const int numLinked = inv->getNumLinkedSlots();
    bool isLinkedTarget[256] = { false };
    for (int h = 0; h < numLinked && h < 256; ++h) {
        int target = inv->linkedSlots[h].inventorySlot;
        if (target >= 0 && target < 256) isLinkedTarget[target] = true;
    }

    int invX = bgX + (bgW - invRowPx) / 2;
    int invY = contentY + topSectionH + sectionGap;

    invSlotsBegin = (int)slots.size();
    int placed = 0;
    for (int playerSlot = numLinked; playerSlot < inv->getContainerSize() && placed < 27; ++playerSlot) {
        if (playerSlot < 256 && isLinkedTarget[playerSlot]) continue;
        int row = placed / 9;
        int col = placed % 9;
        Slot s;
        s.container = inv;
        s.slotIndex = playerSlot;
        s.x = invX + col * SLOT_SIZE;
        s.y = invY + row * SLOT_SIZE;
        s.isResult = false;
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
        s.x = invX + col * SLOT_SIZE;
        s.y = hotbarY;
        s.isResult = false;
        slots.push_back(s);
    }
    hotbarSlotsEnd = (int)slots.size();

    recomputeResult();
}

void DesktopCraftingScreen::tick() {
    rebuildSlots();
}

void DesktopCraftingScreen::recomputeResult() {
    resultItem = Recipes::getInstance()->getItemFor(craftingGrid);
}

int DesktopCraftingScreen::findSlotAt(int x, int y) {
    for (int i = 0; i < (int)slots.size(); ++i) {
        const Slot& s = slots[i];
        if (x >= s.x && x < s.x + SLOT_SIZE &&
            y >= s.y && y < s.y + SLOT_SIZE)
            return i;
    }
    return -1;
}

ItemInstance* DesktopCraftingScreen::getSlotItem(const Slot& s) {
    if (s.isResult)
        return resultItem.isNull() ? NULL : &resultItem;
    if (s.container == NULL) {
        if (!minecraft->player) return NULL;
        ItemInstance* tmp = minecraft->player->getArmor(s.slotIndex);
        if (tmp && !tmp->isNull()) return tmp;
        return NULL;
    }
    if (s.slotIndex < 0) return NULL;
    ItemInstance* tmp = s.container->getItem(s.slotIndex);
    if (tmp && !tmp->isNull()) return tmp;
    return NULL;
}

void DesktopCraftingScreen::render(int xm, int ym, float a) {
    Tesselator& t = Tesselator::instance;

    fill(0, 0, width, height, 0x60101020);

    guiBackground->setSize((float)bgW, (float)bgH);
    guiBackground->draw(t, (float)bgX, (float)bgY);

    std::string title = (gridSize == 3) ? I18n::get("container.crafting") : I18n::get("container.inventory");
    minecraft->font->drawShadow(title, (float)(bgX + 8), (float)(bgY + 6), COLOR_TEXT);

    guiSlot->setSize((float)SLOT_SIZE, (float)SLOT_SIZE);
    for (int i = 0; i < (int)slots.size(); ++i)
        guiSlot->draw(t, (float)slots[i].x, (float)slots[i].y);

    if (gridSize == 2) {
        for (int i = armorSlotsBegin; i < armorSlotsEnd; ++i) {
            const Slot& s = slots[i];
            ItemInstance* armor = minecraft->player ? minecraft->player->getArmor(s.slotIndex) : NULL;
            if (!armor || armor->isNull()) {
                minecraft->textures->loadAndBindTexture("gui/items.png");
                blit(s.x + 1, s.y, 15 * 16, s.slotIndex * 16, 16, 16, 16, 16);
            }
        }
    }

    if (resultSlot >= 0) {
        const Slot& res = slots[resultSlot];
        int arrowX = res.x - RESULT_GAP + (RESULT_GAP - 24) / 2;
        int arrowY = res.y + (SLOT_SIZE - 16) / 2;
        minecraft->textures->loadAndBindTexture("gui/spritesheet.png");
        blit(arrowX, arrowY, 144, 40, 24, 16, 48, 32);
    }

    for (int i = 0; i < (int)slots.size(); ++i) {
        const Slot& s = slots[i];
        ItemInstance* it = getSlotItem(s);
        if (it) {
            ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, it,
                (float)(s.x + SLOT_PAD), (float)(s.y + SLOT_PAD), true);
        }
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
        ItemInstance* it = getSlotItem(slots[i]);
        if (it && it->isDamaged())
            ItemRenderer::renderGuiItemDecorations(it, (float)(slots[i].x + 1), (float)(slots[i].y + 1));
    }
    if (!cursorItem.isNull() && cursorItem.isDamaged()) {
        ItemRenderer::renderGuiItemDecorations(&cursorItem,
            (float)(xm - SLOT_INNER / 2 + 1), (float)(ym - SLOT_INNER / 2 + 1));
    }
    glDisable2(GL_TEXTURE_2D);
    t.endOverrideAndDraw();
    glEnable2(GL_TEXTURE_2D);

    for (int i = 0; i < (int)slots.size(); ++i) {
        ItemInstance* it = getSlotItem(slots[i]);
        if (it) minecraft->gui.renderSlotText(it,
            (float)(slots[i].x + SLOT_PAD - 3), (float)(slots[i].y + SLOT_PAD - 3), true, true);
    }
    if (!cursorItem.isNull()) {
        minecraft->gui.renderSlotText(&cursorItem,
            (float)(xm - SLOT_INNER / 2 - 3), (float)(ym - SLOT_INNER / 2 - 3), true, true);
    }

    super::render(xm, ym, a);

    if (hov >= 0 && cursorItem.isNull()) {
        ItemInstance* it = getSlotItem(slots[hov]);
        if (it) {
            std::string name = it->getName();
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

void DesktopCraftingScreen::mouseClicked(int x, int y, int buttonNum) {
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

void DesktopCraftingScreen::onSlotClick(int idx, int buttonNum, bool shift) {
    Slot& s = slots[idx];

    if (!s.isResult && s.slotIndex < 0) {
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

    if (!s.isResult && s.container == NULL) {
        Player* p = minecraft->player;
        if (!p) return;
        ItemInstance* live = p->getArmor(s.slotIndex);
        ItemInstance equipped = (live && !live->isNull()) ? *live : ItemInstance();

        if (cursorItem.isNull() && !equipped.isNull()) {
            cursorItem = equipped;
            p->setArmor(s.slotIndex, NULL);
            return;
        }
        if (!cursorItem.isNull()) {
            if (!ItemInstance::isArmorItem(&cursorItem)) return;
            ArmorItem* ai = dynamic_cast<ArmorItem*>(cursorItem.getItem());
            if (!ai || ai->slot != s.slotIndex) return;

            ItemInstance toEquip = cursorItem;
            toEquip.count = 1;
            p->setArmor(s.slotIndex, &toEquip);
            cursorItem = equipped;
        }
        return;
    }

    if (s.isResult) {
        if (resultItem.isNull()) return;
        if (shift) {
            while (!resultItem.isNull()) {
                ItemInstance toPlace = resultItem;
                if (!minecraft->player->inventory->add(&toPlace)) break;
                for (int i = 0; i < craftingGrid->getContainerSize(); ++i) {
                    ItemInstance* g = craftingGrid->getItem(i);
                    if (g && !g->isNull()) {
                        g->count -= 1;
                        if (g->count <= 0) g->setNull();
                    }
                }
                recomputeResult();
            }
            return;
        }
        if (cursorItem.isNull()) {
            cursorItem = resultItem;
        } else if (canMerge(cursorItem, resultItem) &&
                   cursorItem.count + resultItem.count <= cursorItem.getMaxStackSize()) {
            cursorItem.count += resultItem.count;
        } else {
            return;
        }
        for (int i = 0; i < craftingGrid->getContainerSize(); ++i) {
            ItemInstance* g = craftingGrid->getItem(i);
            if (g && !g->isNull()) {
                g->count -= 1;
                if (g->count <= 0) g->setNull();
            }
        }
        recomputeResult();
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

bool DesktopCraftingScreen::tryDistributeStack(ItemInstance& stack, int rangeBegin, int rangeEnd) {
    bool moved = false;
    for (int i = rangeBegin; i < rangeEnd && !stack.isNull(); ++i) {
        Slot& s = slots[i];
        if (s.isResult) continue;
        if (s.container == NULL) continue;
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
        if (s.isResult) continue;
        if (s.container == NULL) continue;
        if (s.slotIndex < 0) continue;
        ItemInstance* live = s.container->getItem(s.slotIndex);
        if (live && !live->isNull()) continue;
        writeSlot(s.container, s.slotIndex, stack);
        stack.setNull();
        moved = true;
    }
    return moved;
}

void DesktopCraftingScreen::shiftClickFrom(int idx) {
    Slot& s = slots[idx];

    ItemInstance moving;
    bool fromResult = s.isResult;
    if (fromResult) {
        if (resultItem.isNull()) return;
        moving = resultItem;
    } else {
        if (s.container == NULL) return;
        if (s.slotIndex < 0) return;
        ItemInstance* live = s.container->getItem(s.slotIndex);
        if (!live || live->isNull()) return;
        moving = *live;
    }

    int destBegin, destEnd;
    if (idx >= hotbarSlotsBegin && idx < hotbarSlotsEnd) {
        destBegin = invSlotsBegin; destEnd = invSlotsEnd;
    } else if (idx >= invSlotsBegin && idx < invSlotsEnd) {
        destBegin = hotbarSlotsBegin; destEnd = hotbarSlotsEnd;
    } else {
        destBegin = hotbarSlotsBegin; destEnd = hotbarSlotsEnd;
        ItemInstance moving1 = moving;
        bool placed = tryDistributeStack(moving1, destBegin, destEnd);
        if (!moving1.isNull()) {
            tryDistributeStack(moving1, invSlotsBegin, invSlotsEnd);
        }
        int placedCount = moving.count - moving1.count;
        if (placedCount <= 0) return;
        if (fromResult) {
            for (int i = 0; i < craftingGrid->getContainerSize(); ++i) {
                ItemInstance* g = craftingGrid->getItem(i);
                if (g && !g->isNull()) {
                    g->count -= 1;
                    if (g->count <= 0) g->setNull();
                }
            }
            recomputeResult();
            while (!resultItem.isNull()) {
                ItemInstance again = resultItem;
                ItemInstance again1 = again;
                bool p1 = tryDistributeStack(again1, hotbarSlotsBegin, hotbarSlotsEnd);
                bool p2 = !again1.isNull() ? tryDistributeStack(again1, invSlotsBegin, invSlotsEnd) : false;
                (void)p1; (void)p2;
                if (again1.count == again.count) break;
                for (int i = 0; i < craftingGrid->getContainerSize(); ++i) {
                    ItemInstance* g = craftingGrid->getItem(i);
                    if (g && !g->isNull()) {
                        g->count -= 1;
                        if (g->count <= 0) g->setNull();
                    }
                }
                recomputeResult();
                if (!again1.isNull()) break;
            }
        } else {
            ItemInstance* live = s.container->getItem(s.slotIndex);
            if (live) {
                live->count -= placedCount;
                if (live->count <= 0) live->setNull();
                writeSlot(s.container, s.slotIndex, *live);
            }
            recomputeResult();
        }
        (void)placed;
        return;
    }

    ItemInstance remainder = moving;
    tryDistributeStack(remainder, destBegin, destEnd);
    writeSlot(s.container, s.slotIndex, remainder);
}

void DesktopCraftingScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE || eventKey == Keyboard::KEY_E) {
        minecraft->setScreen(NULL);
        return;
    }
    super::keyPressed(eventKey);
}

void DesktopCraftingScreen::buttonClicked(Button* button) {
    if (button == btnClose) { minecraft->setScreen(NULL); return; }
    super::buttonClicked(button);
}

void DesktopCraftingScreen::returnGridItems() {
    if (!minecraft->player || !minecraft->player->inventory) return;
    for (int i = 0; i < craftingGrid->getContainerSize(); ++i) {
        ItemInstance* g = craftingGrid->getItem(i);
        if (g && !g->isNull()) {
            ItemInstance copy = *g;
            if (!minecraft->player->inventory->add(&copy)) {
                if (copy.count > 0) {
                    minecraft->player->drop(new ItemInstance(copy), false);
                }
            }
            g->setNull();
        }
    }
}

void DesktopCraftingScreen::removed() {
    returnGridItems();
    if (!cursorItem.isNull() && minecraft->player) {
        minecraft->player->drop(new ItemInstance(cursorItem), false);
        cursorItem.setNull();
    }
}