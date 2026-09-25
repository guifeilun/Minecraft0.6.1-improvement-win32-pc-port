#include "DesktopCraftingScreen.h"

#include "../../../Minecraft.h"
#include "../../../player/LocalPlayer.h"
#include "../../../renderer/Tesselator.h"
#include "../../../renderer/entity/ItemRenderer.h"
#include "../../Font.h"
#include "../../Gui.h"
#include "../../components/ImageButton.h"
#include "../../../../world/inventory/CraftingContainer.h"
#include "../../../../world/entity/player/Inventory.h"
#include "../../../../world/entity/player/Player.h"
#include "../../../../world/item/Item.h"
#include "../../../../world/item/ItemInstance.h"
#include "../../../../world/item/ArmorItem.h"
#include "../../../../world/item/crafting/Recipe.h"
#include "../../../../world/item/crafting/Recipes.h"
#include "platform/input/Mouse.h"
#include "platform/input/Keyboard.h"
#include "platform/time.h"

namespace {
const int SLOT_SIZE      = 18;   // standard Java slot pitch
const int SLOT_INNER     = 16;   // item-rendering area inside a slot
const int SLOT_PAD       = 1;    // 1px padding inside the 18x18 slot

const int COLOR_BG       = 0xc0202020;
const int COLOR_FRAME    = 0xff8b8b8b;
const int COLOR_SLOT     = 0xff5b5b5b;
const int COLOR_HOVER    = 0x80ffffff;
const int COLOR_TEXT     = 0xffe0e0e0;
const int COLOR_TOOLTIP_BG = 0xe0100010;
const int COLOR_TOOLTIP_FR = 0xff5000ff;

int min_i(int a, int b) { return a < b ? a : b; }
int max_i(int a, int b) { return a > b ? a : b; }

// True iff two stacks can merge (same item, same aux, both non-null).
bool canMerge(const ItemInstance& a, const ItemInstance& b) {
    if (a.isNull() || b.isNull()) return false;
    if (a.id != b.id) return false;
    if (a.getAuxValue() != b.getAuxValue()) return false;
    if (!a.isStackable()) return false;
    return true;
}

// Container::setItem takes ItemInstance* (with slot ownership semantics that
// vary across subclasses), and CraftingContainer's setItem signature differs.
// Mutating the live ItemInstance returned by getItem() is the common-denominator
// write path and works correctly for both the player Inventory and the
// CraftingContainer used as our grid backing store.
//
// If getItem returns NULL (the slot is empty in PE's Inventory — linked slots
// can resolve to a NULL underlying pointer), fall back to setItem so that a
// fresh ItemInstance gets allocated rather than the write being silently lost.
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
    lastClickSlotIdx(-1),
    lastClickTime(-1.0f)
{
    craftingGrid = new CraftingContainer(gridSize, gridSize);
    cursorItem.setNull();
    resultItem.setNull();
}

DesktopCraftingScreen::~DesktopCraftingScreen() {
    delete craftingGrid;
    delete btnClose;
}

void DesktopCraftingScreen::init() {
    super::init();
    rebuildSlots();
    recomputeResult();
}

void DesktopCraftingScreen::rebuildSlots() {
    if (!minecraft || !minecraft->player || !minecraft->player->inventory)
        return;
    Inventory* inv = minecraft->player->inventory;

    // Layout: top section = grid + result; bottom section = inventory + hotbar.
    const int gridPx     = gridSize * SLOT_SIZE;
    const int invRowPx   = 9 * SLOT_SIZE;
    const int innerPad   = 7;
    const int sectionGap = 6;
    const int titleH     = 12;
    const int hotbarGap  = 4;

    // For the 2x2 (inventory) variant we add a column of 4 armor slots on the
    // far left, so the panel needs to be wider AND taller (4 armor slots are
    // taller than the 2x2 crafting grid).
    bool showArmor = (gridSize == 2);
    int armorW = showArmor ? (SLOT_SIZE + 6) : 0;
    int topSectionH = showArmor ? max_i(4 * SLOT_SIZE, gridPx) : gridPx;
    bgW = max_i(invRowPx, armorW + gridPx + 30 + SLOT_SIZE) + 2 * innerPad;
    bgH = titleH + topSectionH + sectionGap + 3 * SLOT_SIZE + hotbarGap + SLOT_SIZE + 2 * innerPad;

    bgX = (width  - bgW) / 2;
    bgY = (height - bgH) / 2;

    int contentX = bgX + innerPad;
    int contentY = bgY + innerPad + titleH;

    slots.clear();

    // Armor slots (only for the 2x2 inventory variant). container == NULL is
    // a sentinel telling render/click code to use Player::getArmor / setArmor
    // instead of a Container interface; slotIndex 0..3 = head/torso/legs/feet.
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

    // Crafting grid (NxN). Vertically centered within the top section so the
    // 2x2 grid lines up nicely next to the taller armor column.
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

    // Result slot to the right of the grid (Java places it after a small arrow gap)
    {
        Slot s;
        s.container = craftingGrid;  // unused (special-cased via isResult)
        s.slotIndex = -1;
        s.x = contentX + gridPx + 30;
        s.y = contentY + gridYOffset + (gridPx - SLOT_SIZE) / 2;
        s.isResult = true;
        resultSlot = (int)slots.size();
        slots.push_back(s);
    }

    // PE's Inventory uses a "linked slot" model: slots [0..numLinkedSlots-1] are
    // link entries that resolve to slots >= numLinkedSlots. The actual storage
    // for items currently in the hotbar lives in those higher slots. To avoid
    // showing the same item twice (once in the hotbar row, once in the main
    // grid), we skip slots that are currently linked from a hotbar position.
    const int numLinked = inv->getNumLinkedSlots();
    bool isLinkedTarget[256] = { false };
    for (int h = 0; h < numLinked && h < 256; ++h) {
        int target = inv->linkedSlots[h].inventorySlot;
        if (target >= 0 && target < 256) isLinkedTarget[target] = true;
    }

    // Main inventory: 3 rows x 9 cols. Iterate real storage slots
    // (>= numLinked), skipping any that are linked from the hotbar.
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
        s.x = bgX + (bgW - invRowPx) / 2 + col * SLOT_SIZE;
        s.y = invY + row * SLOT_SIZE;
        s.isResult = false;
        slots.push_back(s);
        ++placed;
    }
    invSlotsEnd = (int)slots.size();

    // Hotbar (1 row x 9 cols). Each hotbar position routes to its link target
    // in the underlying inventory; positions with no link are still drawn as
    // empty boxes (slotIndex = -1 sentinel; clicks are ignored).
    int hotbarY = invY + 3 * SLOT_SIZE + hotbarGap;
    hotbarSlotsBegin = (int)slots.size();
    for (int col = 0; col < 9 && col < numLinked; ++col) {
        int target = inv->linkedSlots[col].inventorySlot;
        Slot s;
        s.container = inv;
        s.slotIndex = (target >= 0) ? target : -1;  // -1 = empty hotbar position
        s.x = bgX + (bgW - invRowPx) / 2 + col * SLOT_SIZE;
        s.y = hotbarY;
        s.isResult = false;
        slots.push_back(s);
    }
    hotbarSlotsEnd = (int)slots.size();

    recomputeResult();
}

void DesktopCraftingScreen::setupPositions() {
    init();
}

void DesktopCraftingScreen::tick() {
    // Rebuild every tick because PE's hotbar links to inventory slots dynamically
    // (when items are picked up, dropped, etc., the link table mutates) and our
    // slot list needs to reflect the current state.
    rebuildSlots();
}

void DesktopCraftingScreen::recomputeResult() {
    resultItem = Recipes::getInstance()->getItemFor(craftingGrid);
}

int DesktopCraftingScreen::findSlotAt(int x, int y) {
    // Use the full slot rect (no inner padding) — Java fidelity, more forgiving.
    for (int i = 0; i < (int)slots.size(); ++i) {
        const Slot& s = slots[i];
        if (x >= s.x && x < s.x + SLOT_SIZE &&
            y >= s.y && y < s.y + SLOT_SIZE)
            return i;
    }
    return -1;
}

void DesktopCraftingScreen::renderSlotBg(int x, int y) {
    fill(x,             y,             x + SLOT_SIZE, y + SLOT_SIZE, COLOR_FRAME);
    fill(x + SLOT_PAD,  y + SLOT_PAD,  x + SLOT_SIZE - SLOT_PAD, y + SLOT_SIZE - SLOT_PAD, COLOR_SLOT);
}

void DesktopCraftingScreen::render(int xm, int ym, float a) {
    // Light dim over the live world (renderGameBehind() returns true so the
    // game render pass already drew the world; we tint it slightly darker).
    fill(0, 0, width, height, 0x60101020);

    // Panel background + frame
    fill(bgX - 1, bgY - 1, bgX + bgW + 1, bgY + bgH + 1, COLOR_FRAME);
    fill(bgX,     bgY,     bgX + bgW,     bgY + bgH,     COLOR_BG);

    // Title
    const char* title = (gridSize == 3) ? "Crafting" : "Inventory";
    minecraft->font->drawShadow(std::string(title), (float)(bgX + 8), (float)(bgY + 6), COLOR_TEXT);

    // Slot backgrounds
    for (int i = 0; i < (int)slots.size(); ++i)
        renderSlotBg(slots[i].x, slots[i].y);

    // Arrow between grid and result (drawn as text for simplicity)
    if (resultSlot >= 0) {
        const Slot& res = slots[resultSlot];
        int arrowX = res.x - 22;
        int arrowY = res.y + (SLOT_SIZE / 2) - 4;
        minecraft->font->drawShadow(std::string(">>"), (float)arrowX, (float)arrowY, COLOR_TEXT);
    }

    // Items inside slots
    for (int i = 0; i < (int)slots.size(); ++i) {
        const Slot& s = slots[i];
        ItemInstance* it = NULL;
        if (s.isResult) {
            it = resultItem.isNull() ? NULL : &resultItem;
        } else if (s.container == NULL) {
            // Armor slot
            if (minecraft->player) {
                ItemInstance* tmp = minecraft->player->getArmor(s.slotIndex);
                if (tmp && !tmp->isNull()) it = tmp;
            }
        } else if (s.slotIndex >= 0) {
            ItemInstance* tmp = s.container->getItem(s.slotIndex);
            if (tmp && !tmp->isNull()) it = tmp;
        }
        if (it) {
            ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, it,
                (float)(s.x + SLOT_PAD), (float)(s.y + SLOT_PAD + 1), true);
            minecraft->gui.renderSlotText(it, (float)(s.x + SLOT_PAD - 3), (float)(s.y + SLOT_PAD - 3), true, true);
        }
    }

    // Hover highlight
    int hov = findSlotAt(xm, ym);
    if (hov >= 0) {
        const Slot& s = slots[hov];
        fill(s.x + SLOT_PAD, s.y + SLOT_PAD, s.x + SLOT_SIZE - SLOT_PAD, s.y + SLOT_SIZE - SLOT_PAD, COLOR_HOVER);
    }

    // Cursor item
    if (!cursorItem.isNull()) {
        ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, &cursorItem,
            (float)(xm - SLOT_INNER / 2), (float)(ym - SLOT_INNER / 2 + 1), true);
        minecraft->gui.renderSlotText(&cursorItem,
            (float)(xm - SLOT_INNER / 2 - 3), (float)(ym - SLOT_INNER / 2 - 3), true, true);
    }

    // Tooltip for hovered slot (only if cursor has nothing)
    if (hov >= 0 && cursorItem.isNull()) {
        const Slot& s = slots[hov];
        ItemInstance* it = NULL;
        if (s.isResult) {
            it = resultItem.isNull() ? NULL : &resultItem;
        } else if (s.container == NULL) {
            if (minecraft->player) {
                ItemInstance* tmp = minecraft->player->getArmor(s.slotIndex);
                if (tmp && !tmp->isNull()) it = tmp;
            }
        } else if (s.slotIndex >= 0) {
            ItemInstance* tmp = s.container->getItem(s.slotIndex);
            if (tmp && !tmp->isNull()) it = tmp;
        }
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

    // Buttons (we have none, but call super for consistency / future use)
    super::render(xm, ym, a);
}

void DesktopCraftingScreen::mouseClicked(int x, int y, int buttonNum) {
    if (buttonNum != MouseAction::ACTION_LEFT && buttonNum != MouseAction::ACTION_RIGHT) {
        super::mouseClicked(x, y, buttonNum);
        return;
    }

    rebuildSlots();
    int idx = findSlotAt(x, y);
    if (idx < 0) {
        // Click that missed every slot. Java behaviour: only drop the cursor
        // item if the click is OUTSIDE the panel rect; clicks in the dead space
        // between/around slots inside the panel are no-ops so users don't
        // accidentally throw items away.
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

    // Double-click on the result slot acts like a shift-click — bulk-craft
    // until the grid runs out of ingredients or the inventory fills up.
    float now = getTimeS();
    bool isDoubleClick =
        (idx == lastClickSlotIdx) &&
        (now - lastClickTime < 0.4f) &&
        (buttonNum == MouseAction::ACTION_LEFT);
    lastClickSlotIdx = idx;
    lastClickTime = now;
    if (isDoubleClick && idx >= 0 && idx < (int)slots.size() && slots[idx].isResult) {
        shift = true;
    }

    onSlotClick(idx, buttonNum, shift);
    super::mouseClicked(x, y, buttonNum);
}

void DesktopCraftingScreen::onSlotClick(int idx, int buttonNum, bool shift) {
    Slot& s = slots[idx];

    // Empty-hotbar sentinel: clicking with a cursor item allocates a free
    // inventory slot, writes the item there, and links the hotbar position to
    // the new slot.
    if (!s.isResult && s.slotIndex < 0) {
        if (cursorItem.isNull()) return;
        if (idx < hotbarSlotsBegin || idx >= hotbarSlotsEnd) return;
        int col = idx - hotbarSlotsBegin;

        Inventory* inv = minecraft->player->inventory;
        const int numLinked = inv->getNumLinkedSlots();

        // Find an inventory slot that's both empty AND not already linked from
        // another hotbar position.
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
        if (freeSlot < 0) return;  // no room

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

    // Armor slot (container == NULL sentinel). Left/right click both behave
    // like Java: cursor empty -> take equipped armor; cursor has matching
    // armor -> equip it (swap if already equipped). Wrong-type items are
    // rejected silently.
    if (!s.isResult && s.container == NULL) {
        Player* p = minecraft->player;
        if (!p) return;
        ItemInstance* live = p->getArmor(s.slotIndex);
        ItemInstance equipped = (live && !live->isNull()) ? *live : ItemInstance();

        if (cursorItem.isNull() && !equipped.isNull()) {
            // Take equipped armor onto cursor.
            cursorItem = equipped;
            p->setArmor(s.slotIndex, NULL);
            return;
        }
        if (!cursorItem.isNull()) {
            // Validate that cursor item is armor of the right slot type.
            if (!ItemInstance::isArmorItem(&cursorItem)) return;
            ArmorItem* ai = dynamic_cast<ArmorItem*>(cursorItem.getItem());
            if (!ai || ai->slot != s.slotIndex) return;

            // Place — armor doesn't stack so we always place exactly one and
            // swap with whatever's there.
            ItemInstance toEquip = cursorItem;
            toEquip.count = 1;
            p->setArmor(s.slotIndex, &toEquip);
            cursorItem = equipped;  // either null (unequipping) or the old armor
        }
        return;
    }

    // Result slot: special — never accepts items, click takes one craft worth.
    if (s.isResult) {
        if (resultItem.isNull()) return;
        if (shift) {
            // Take repeatedly, place each result into inventory, until grid changes break recipe.
            while (!resultItem.isNull()) {
                ItemInstance toPlace = resultItem;
                if (!minecraft->player->inventory->add(&toPlace)) break;
                // Consume one of each ingredient
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
        // Single take: result must merge onto cursor or fill empty cursor.
        if (cursorItem.isNull()) {
            cursorItem = resultItem;
        } else if (canMerge(cursorItem, resultItem) &&
                   cursorItem.count + resultItem.count <= cursorItem.getMaxStackSize()) {
            cursorItem.count += resultItem.count;
        } else {
            return;  // can't take
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
            // Pick up entire stack
            cursorItem = slotItem;
            writeSlot(s.container, s.slotIndex,ItemInstance());
        } else if (!cursorItem.isNull() && slotItem.isNull()) {
            // Drop entire cursor into slot
            writeSlot(s.container, s.slotIndex,cursorItem);
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
                    writeSlot(s.container, s.slotIndex,slotItem);
                } else {
                    // No room — swap
                    writeSlot(s.container, s.slotIndex,cursorItem);
                    cursorItem = slotItem;
                }
            } else {
                // Different items — swap
                writeSlot(s.container, s.slotIndex,cursorItem);
                cursorItem = slotItem;
            }
        }
    } else {  // ACTION_RIGHT
        if (cursorItem.isNull() && !slotItem.isNull()) {
            // Pick up half (rounded up)
            int take = (slotItem.count + 1) / 2;
            cursorItem = slotItem;
            cursorItem.count = take;
            slotItem.count -= take;
            if (slotItem.count <= 0) slotItem.setNull();
            writeSlot(s.container, s.slotIndex,slotItem);
        } else if (!cursorItem.isNull() && slotItem.isNull()) {
            // Place one
            ItemInstance one = cursorItem;
            one.count = 1;
            writeSlot(s.container, s.slotIndex,one);
            cursorItem.count -= 1;
            if (cursorItem.count <= 0) cursorItem.setNull();
        } else if (!cursorItem.isNull() && !slotItem.isNull()) {
            if (canMerge(cursorItem, slotItem) && slotItem.count < slotItem.getMaxStackSize()) {
                slotItem.count += 1;
                cursorItem.count -= 1;
                if (cursorItem.count <= 0) cursorItem.setNull();
                writeSlot(s.container, s.slotIndex,slotItem);
            } else if (!canMerge(cursorItem, slotItem)) {
                // Swap
                writeSlot(s.container, s.slotIndex,cursorItem);
                cursorItem = slotItem;
            }
        }
    }
}

bool DesktopCraftingScreen::tryDistributeStack(ItemInstance& stack, int rangeBegin, int rangeEnd) {
    bool moved = false;
    // First pass: merge into existing partial stacks.
    for (int i = rangeBegin; i < rangeEnd && !stack.isNull(); ++i) {
        Slot& s = slots[i];
        if (s.isResult) continue;
        if (s.container == NULL) continue;  // armor slot — never auto-distribute
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
        writeSlot(s.container, s.slotIndex,*live);
        moved = true;
    }
    // Second pass: drop into empty slots.
    for (int i = rangeBegin; i < rangeEnd && !stack.isNull(); ++i) {
        Slot& s = slots[i];
        if (s.isResult) continue;
        if (s.container == NULL) continue;  // armor slot — never auto-distribute
        ItemInstance* live = s.container->getItem(s.slotIndex);
        if (live && !live->isNull()) continue;
        writeSlot(s.container, s.slotIndex,stack);
        stack.setNull();
        moved = true;
    }
    return moved;
}

void DesktopCraftingScreen::shiftClickFrom(int idx) {
    Slot& s = slots[idx];

    // Pick up the stack (or use the result slot's virtual item).
    ItemInstance moving;
    bool fromResult = s.isResult;
    if (fromResult) {
        if (resultItem.isNull()) return;
        moving = resultItem;
    } else {
        ItemInstance* live = s.container->getItem(s.slotIndex);
        if (!live || live->isNull()) return;
        moving = *live;
    }

    // Pick destination range. Java rules:
    //  - From hotbar -> main inventory
    //  - From main inventory -> hotbar
    //  - From crafting grid or result -> hotbar first, then main inventory
    int destBegin, destEnd;
    if (idx >= hotbarSlotsBegin && idx < hotbarSlotsEnd) {
        destBegin = invSlotsBegin; destEnd = invSlotsEnd;
    } else if (idx >= invSlotsBegin && idx < invSlotsEnd) {
        destBegin = hotbarSlotsBegin; destEnd = hotbarSlotsEnd;
    } else {
        // From grid or result: try hotbar then inventory
        destBegin = hotbarSlotsBegin; destEnd = hotbarSlotsEnd;
        ItemInstance moving1 = moving;
        bool placed = tryDistributeStack(moving1, destBegin, destEnd);
        if (!moving1.isNull()) {
            tryDistributeStack(moving1, invSlotsBegin, invSlotsEnd);
        }
        // Apply consumption back to source
        int placedCount = moving.count - moving1.count;
        if (placedCount <= 0) return;
        if (fromResult) {
            // Consume one of each ingredient
            for (int i = 0; i < craftingGrid->getContainerSize(); ++i) {
                ItemInstance* g = craftingGrid->getItem(i);
                if (g && !g->isNull()) {
                    g->count -= 1;
                    if (g->count <= 0) g->setNull();
                }
            }
            recomputeResult();
            // Java continues taking until result becomes null or inventory is full.
            while (!resultItem.isNull()) {
                ItemInstance again = resultItem;
                ItemInstance again1 = again;
                bool p1 = tryDistributeStack(again1, hotbarSlotsBegin, hotbarSlotsEnd);
                bool p2 = !again1.isNull() ? tryDistributeStack(again1, invSlotsBegin, invSlotsEnd) : false;
                (void)p1; (void)p2;
                if (again1.count == again.count) break;  // could not place any more
                for (int i = 0; i < craftingGrid->getContainerSize(); ++i) {
                    ItemInstance* g = craftingGrid->getItem(i);
                    if (g && !g->isNull()) {
                        g->count -= 1;
                        if (g->count <= 0) g->setNull();
                    }
                }
                recomputeResult();
                if (!again1.isNull()) break;  // partial — stop
            }
        } else {
            // Source is the crafting grid input
            ItemInstance* live = s.container->getItem(s.slotIndex);
            if (live) {
                live->count -= placedCount;
                if (live->count <= 0) live->setNull();
                writeSlot(s.container, s.slotIndex,*live);
            }
            recomputeResult();
        }
        (void)placed;
        return;
    }

    // From hotbar or main inv: try to distribute, then write back the remainder.
    ItemInstance remainder = moving;
    tryDistributeStack(remainder, destBegin, destEnd);
    writeSlot(s.container, s.slotIndex,remainder);
}

void DesktopCraftingScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE || eventKey == Keyboard::KEY_E) {
        minecraft->setScreen(NULL);
        return;
    }
    super::keyPressed(eventKey);
}

void DesktopCraftingScreen::returnGridItems() {
    if (!minecraft->player || !minecraft->player->inventory) return;
    for (int i = 0; i < craftingGrid->getContainerSize(); ++i) {
        ItemInstance* g = craftingGrid->getItem(i);
        if (g && !g->isNull()) {
            ItemInstance copy = *g;
            if (!minecraft->player->inventory->add(&copy)) {
                // Inventory full — drop remainder in the world.
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
