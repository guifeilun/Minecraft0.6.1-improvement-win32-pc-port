#include "DesktopCreativeScreen.h"

#include "../../../Minecraft.h"
#include "../../../player/LocalPlayer.h"
#include "../../../renderer/entity/ItemRenderer.h"
#include "../../../sound/SoundEngine.h"
#include "../../Font.h"
#include "../../Gui.h"
#include "../../../../world/entity/player/Inventory.h"
#include "../../../../world/entity/player/Player.h"
#include "../../../../world/item/Item.h"
#include "../../../../world/item/ItemCategory.h"
#include "../../../../world/item/ItemInstance.h"
#include "../../../../world/level/tile/Tile.h"
#include "platform/input/Mouse.h"
#include "platform/input/Keyboard.h"
#include "../../../Options.h"

namespace {
const int SLOT_SIZE   = 18;
const int SLOT_PAD    = 1;
const int SLOT_INNER  = 16;
const int PALETTE_COLS = 9;
const int TAB_H       = 18;
const int TAB_PAD     = 6;
const int HOTBAR_GAP  = 10;
const int INNER_PAD   = 8;

// Prefixed CLR_* to dodge Win32's COLOR_* macros (winuser.h defines
// CLR_SCROLL_BAR, COLOR_BTNTEXT, etc. as integer system-colour ids).
const int CLR_DIM        = 0x60101020;
const int CLR_BG         = 0xc0202020;
const int CLR_FRAME      = 0xff8b8b8b;
const int CLR_SLOT       = 0xff5b5b5b;
const int CLR_HOVER      = 0x80ffffff;
const int CLR_TEXT       = 0xffe0e0e0;
const int CLR_TAB_BG     = 0xa0303030;
const int CLR_TAB_ACTIVE = 0xff404858;
const int CLR_TAB_HOVER  = 0xff505050;
const int CLR_TIP_BG     = 0xe0100010;
const int CLR_TIP_FR     = 0xff5000ff;
const int CLR_SCROLL_BAR = 0xff707070;
const int CLR_SCROLL_HND = 0xffb0b0b0;

const int CAT_ALL = -1;

// One row per cloth/dye colour so the palette shows all 16 of each.
struct AuxRange { int id; int auxLo; int auxHi; };
const AuxRange AUX_EXPAND[] = {
    {  35 /* cloth tile id */,    0, 15 },
    { 256 + 95 /* dye_powder */,  0, 15 },
};
}

DesktopCreativeScreen::DesktopCreativeScreen()
:   activeCategory(CAT_ALL),
    scrollOffsetRows(0),
    hoveredEntryIdx(-1),
    bgX(0), bgY(0), bgW(0), bgH(0),
    paletteX(0), paletteY(0), paletteW(0), paletteH(0),
    paletteCols(PALETTE_COLS), paletteRowsVisible(0),
    hotbarX(0), hotbarY(0),
    tabsY(0), tabW(0), tabH(TAB_H),
    hoveredTabIdx(-1)
{
    // Each tab is drawn as a 22x22 icon button. The icon is just a
    // representative item from that category (so e.g. the tools tab shows
    // the iron pickaxe). The full text label appears as a hover tooltip.
    // 0 = no icon (the All tab is drawn as text since no single item really
    // represents "everything").
    Tab t;
    t.label = "All";           t.category = CAT_ALL;                  t.iconItemId = 0;                                    t.x = 0; tabs.push_back(t);
    t.label = "Blocks";        t.category = ItemCategory::Structures; t.iconItemId = 1   /* Tile::rock id */;              t.x = 0; tabs.push_back(t);
    t.label = "Tools";         t.category = ItemCategory::Tools;      t.iconItemId = 256 + 1  /* Item::pickAxe_iron */;    t.x = 0; tabs.push_back(t);
    t.label = "Food & Armor";  t.category = ItemCategory::FoodArmor;  t.iconItemId = 256 + 4  /* Item::apple */;           t.x = 0; tabs.push_back(t);
    t.label = "Decorations";   t.category = ItemCategory::Decorations;t.iconItemId = 50  /* Tile::torch id */;             t.x = 0; tabs.push_back(t);
    t.label = "Mechanisms";    t.category = ItemCategory::Mechanisms; t.iconItemId = 256 + 75 /* Item::redStone */;        t.x = 0; tabs.push_back(t);
}

DesktopCreativeScreen::~DesktopCreativeScreen() {}

void DesktopCreativeScreen::init() {
    super::init();
    rebuildEntries();
    layout();
}

void DesktopCreativeScreen::removed() {
    minecraft->gui.inventoryUpdated();
}

void DesktopCreativeScreen::rebuildEntries() {
    entries.clear();

    // Walk the global Item table (which covers both real items and the
    // auto-registered TileItems). Anything with a valid category is something
    // we want surfaced in the creative menu.
    for (int i = 0; i < Item::MAX_ITEMS; ++i) {
        Item* it = Item::items[i];
        if (!it) continue;
        if (it->category < 0) continue;

        // Skip aux variants we explicitly expand below to avoid duplicates.
        bool expanded = false;
        for (size_t a = 0; a < sizeof(AUX_EXPAND)/sizeof(AUX_EXPAND[0]); ++a) {
            if (AUX_EXPAND[a].id == i) { expanded = true; break; }
        }
        if (expanded) continue;

        Entry e;
        e.item = ItemInstance(it, 1, 0);
        e.category = it->category;
        entries.push_back(e);
    }

    // Now explicitly expand cloth and dye colours so every shade is visible.
    for (size_t a = 0; a < sizeof(AUX_EXPAND)/sizeof(AUX_EXPAND[0]); ++a) {
        int id = AUX_EXPAND[a].id;
        Item* it = Item::items[id];
        if (!it) continue;
        for (int aux = AUX_EXPAND[a].auxLo; aux <= AUX_EXPAND[a].auxHi; ++aux) {
            Entry e;
            e.item = ItemInstance(it, 1, aux);
            e.category = it->category;
            entries.push_back(e);
        }
    }

    // Build the filtered index list for the active category.
    filtered.clear();
    for (size_t i = 0; i < entries.size(); ++i) {
        if (activeCategory == CAT_ALL || entries[i].category == activeCategory)
            filtered.push_back((int)i);
    }
    scrollOffsetRows = 0;
}

void DesktopCreativeScreen::layout() {
    // Panel sized for 9 cols and ~7 visible rows. Centred on the screen.
    paletteCols = PALETTE_COLS;
    paletteRowsVisible = 7;

    int paletteContentW = paletteCols * SLOT_SIZE;
    int paletteContentH = paletteRowsVisible * SLOT_SIZE;

    int hotbarRowW = 9 * SLOT_SIZE;
    int innerW = paletteContentW + 12;   // +scrollbar
    bgW = innerW + 2 * INNER_PAD;
    bgH = TAB_H + 2 + paletteContentH + HOTBAR_GAP + SLOT_SIZE + 2 * INNER_PAD;

    bgX = (width  - bgW) / 2;
    bgY = (height - bgH) / 2;

    tabsY = bgY + INNER_PAD;
    // Each tab is a fixed-width icon button (slot-sized + a couple of pixels
    // of padding) so the row stays compact regardless of how long the label
    // text is. They're left-aligned and never overflow the panel.
    tabW = SLOT_SIZE + 4;
    int countTabs = (int)tabs.size();
    for (int i = 0; i < countTabs; ++i)
        tabs[i].x = bgX + INNER_PAD + i * (tabW + 2);

    paletteX = bgX + INNER_PAD;
    paletteY = tabsY + TAB_H + 4;
    paletteW = paletteContentW;
    paletteH = paletteContentH;

    hotbarX = bgX + (bgW - hotbarRowW) / 2;
    hotbarY = paletteY + paletteH + HOTBAR_GAP;
}

int DesktopCreativeScreen::visibleRows() const {
    return paletteRowsVisible;
}

int DesktopCreativeScreen::hitTestPaletteCell(int mx, int my) const {
    if (mx < paletteX || mx >= paletteX + paletteW) return -1;
    if (my < paletteY || my >= paletteY + paletteH) return -1;
    int col = (mx - paletteX) / SLOT_SIZE;
    int row = (my - paletteY) / SLOT_SIZE;
    int idx = (scrollOffsetRows + row) * paletteCols + col;
    if (idx < 0 || idx >= (int)filtered.size()) return -1;
    return idx;
}

int DesktopCreativeScreen::hitTestHotbar(int mx, int my) const {
    if (my < hotbarY || my >= hotbarY + SLOT_SIZE) return -1;
    int relX = mx - hotbarX;
    if (relX < 0 || relX >= 9 * SLOT_SIZE) return -1;
    return relX / SLOT_SIZE;
}

int DesktopCreativeScreen::hitTestTab(int mx, int my) const {
    if (my < tabsY || my >= tabsY + TAB_H) return -1;
    for (int i = 0; i < (int)tabs.size(); ++i) {
        if (mx >= tabs[i].x && mx < tabs[i].x + tabW)
            return i;
    }
    return -1;
}

// Place `item` into whatever slot the player has selected on the hotbar.
// PE's hotbar is a "linked slot" — a redirection into a real inventory slot,
// so we either overwrite the linked target or, if there isn't one, allocate
// a fresh inventory slot and link to it.
void DesktopCreativeScreen::giveToHotbar(const ItemInstance& item) {
    if (item.isNull()) return;
    if (!minecraft || !minecraft->player || !minecraft->player->inventory) return;
    Inventory* inv = minecraft->player->inventory;
    int sel = inv->selected;
    if (sel < 0 || sel >= inv->getNumLinkedSlots()) return;

    int targetSlot = inv->linkedSlots[sel].inventorySlot;
    if (targetSlot < 0) {
        // No backing inventory slot yet — try to find a free one.
        int numLinked = inv->getNumLinkedSlots();
        for (int s = numLinked; s < inv->getContainerSize(); ++s) {
            if (inv->getItem(s) == NULL) {
                ItemInstance copy = item;
                inv->setItem(s, &copy);
                inv->linkSlot(sel, s, true);
                return;
            }
        }
        return;
    }
    // Overwrite the existing slot. ItemInstance is value-typed in PE so writing
    // through getItem() updates the live store.
    ItemInstance* existing = inv->getItem(targetSlot);
    if (existing) {
        *existing = item;
    } else {
        ItemInstance copy = item;
        inv->setItem(targetSlot, &copy);
    }
}

void DesktopCreativeScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    if (dy == 0) return;
    int totalRows = ((int)filtered.size() + paletteCols - 1) / paletteCols;
    int maxScroll = totalRows - paletteRowsVisible;
    if (maxScroll < 0) maxScroll = 0;
    scrollOffsetRows -= (dy > 0 ? 1 : -1);
    if (scrollOffsetRows < 0) scrollOffsetRows = 0;
    if (scrollOffsetRows > maxScroll) scrollOffsetRows = maxScroll;
}

void DesktopCreativeScreen::keyPressed(int eventKey) {
    Options& o = minecraft->options;
    if (eventKey == o.getIntValue(OPTIONS_KEY_MENU_CANCEL) || eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->setScreen(NULL);
        return;
    }
    if (eventKey == Keyboard::KEY_E || eventKey == Keyboard::KEY_I) {
        minecraft->setScreen(NULL);
        return;
    }
}

void DesktopCreativeScreen::mouseClicked(int x, int y, int buttonNum) {
    if (buttonNum != MouseAction::ACTION_LEFT && buttonNum != MouseAction::ACTION_RIGHT) {
        super::mouseClicked(x, y, buttonNum);
        return;
    }

    // Tab click — switch category.
    int tabIdx = hitTestTab(x, y);
    if (tabIdx >= 0) {
        activeCategory = tabs[tabIdx].category;
        // Rebuild filter only (entries themselves are stable).
        filtered.clear();
        for (size_t i = 0; i < entries.size(); ++i) {
            if (activeCategory == CAT_ALL || entries[i].category == activeCategory)
                filtered.push_back((int)i);
        }
        scrollOffsetRows = 0;
        if (minecraft->soundEngine)
            minecraft->soundEngine->playUI("random.click", 1, 1);
        return;
    }

    // Palette click — give the clicked item to the selected hotbar slot.
    int paletteIdx = hitTestPaletteCell(x, y);
    if (paletteIdx >= 0) {
        const Entry& e = entries[filtered[paletteIdx]];
        ItemInstance give = e.item;
        // Left = full stack, right = single. Tools cap at 1 so the stack-size
        // dance reduces to a no-op for them.
        if (buttonNum == MouseAction::ACTION_LEFT)
            give.count = (give.id < Item::MAX_ITEMS && Item::items[give.id])
                         ? Item::items[give.id]->getMaxStackSize() : 64;
        else
            give.count = 1;
        giveToHotbar(give);
        if (minecraft->soundEngine)
            minecraft->soundEngine->playUI("random.click", 1, 1);
        return;
    }

    // Hotbar click — clear the slot (helpful when you've messed it up).
    int hotIdx = hitTestHotbar(x, y);
    if (hotIdx >= 0 && minecraft->player && minecraft->player->inventory) {
        Inventory* inv = minecraft->player->inventory;
        int targetSlot = inv->linkedSlots[hotIdx].inventorySlot;
        if (targetSlot >= 0) {
            ItemInstance* existing = inv->getItem(targetSlot);
            if (existing) existing->setNull();
        }
        if (minecraft->soundEngine)
            minecraft->soundEngine->playUI("random.click", 1, 1);
        return;
    }

    super::mouseClicked(x, y, buttonNum);
}

void DesktopCreativeScreen::render(int xm, int ym, float a) {
    // Dim world behind, then draw the panel.
    fill(0, 0, width, height, CLR_DIM);
    fill(bgX - 1, bgY - 1, bgX + bgW + 1, bgY + bgH + 1, CLR_FRAME);
    fill(bgX,     bgY,     bgX + bgW,     bgY + bgH,     CLR_BG);

    // Tabs.
    hoveredTabIdx = hitTestTab(xm, ym);
    for (int i = 0; i < (int)tabs.size(); ++i) {
        int tx = tabs[i].x;
        bool active = (tabs[i].category == activeCategory);
        int color = active ? CLR_TAB_ACTIVE
                  : (hoveredTabIdx == i ? CLR_TAB_HOVER : CLR_TAB_BG);
        fill(tx, tabsY, tx + tabW, tabsY + TAB_H, color);

        if (tabs[i].iconItemId > 0 && tabs[i].iconItemId < Item::MAX_ITEMS
            && Item::items[tabs[i].iconItemId])
        {
            // Centre the item icon in the tab.
            ItemInstance icon(Item::items[tabs[i].iconItemId], 1, 0);
            int ix = tx + (tabW - SLOT_INNER) / 2;
            int iy = tabsY + (TAB_H - SLOT_INNER) / 2;
            ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, &icon,
                (float)ix, (float)iy + 1, true);
        } else {
            // No icon (the All tab) — fall back to a centred text label.
            std::string label = tabs[i].label;
            int tw = (int)minecraft->font->width(label);
            int lx = tx + (tabW - tw) / 2;
            int ly = tabsY + (TAB_H - 8) / 2;
            minecraft->font->drawShadow(label, (float)lx, (float)ly, CLR_TEXT);
        }

        // Underline the active tab so it pops even when its icon doesn't.
        if (active) {
            int underY = tabsY + TAB_H - 2;
            fill(tx, underY, tx + tabW, underY + 2, 0xff5cc0ff);
        }
    }

    // Palette grid background.
    fill(paletteX - 1, paletteY - 1, paletteX + paletteW + 1, paletteY + paletteH + 1, CLR_FRAME);
    fill(paletteX,     paletteY,     paletteX + paletteW,     paletteY + paletteH,     CLR_SLOT);

    // Palette slot grid + items.
    hoveredEntryIdx = hitTestPaletteCell(xm, ym);
    for (int row = 0; row < paletteRowsVisible; ++row) {
        for (int col = 0; col < paletteCols; ++col) {
            int sx = paletteX + col * SLOT_SIZE;
            int sy = paletteY + row * SLOT_SIZE;
            fill(sx + SLOT_PAD, sy + SLOT_PAD, sx + SLOT_SIZE - SLOT_PAD, sy + SLOT_SIZE - SLOT_PAD, 0x20000000);

            int idx = (scrollOffsetRows + row) * paletteCols + col;
            if (idx >= 0 && idx < (int)filtered.size()) {
                const Entry& e = entries[filtered[idx]];
                ItemInstance copy = e.item;
                ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, &copy,
                    (float)(sx + SLOT_PAD), (float)(sy + SLOT_PAD + 1), true);
            }
        }
    }
    // Hover highlight.
    if (hoveredEntryIdx >= 0) {
        int row = hoveredEntryIdx / paletteCols - scrollOffsetRows;
        int col = hoveredEntryIdx % paletteCols;
        int sx = paletteX + col * SLOT_SIZE;
        int sy = paletteY + row * SLOT_SIZE;
        fill(sx + SLOT_PAD, sy + SLOT_PAD, sx + SLOT_SIZE - SLOT_PAD, sy + SLOT_SIZE - SLOT_PAD, CLR_HOVER);
    }

    // Scrollbar.
    {
        int totalRows = ((int)filtered.size() + paletteCols - 1) / paletteCols;
        int sbX = paletteX + paletteW + 2;
        int sbY = paletteY;
        int sbW = 6;
        int sbH = paletteH;
        fill(sbX, sbY, sbX + sbW, sbY + sbH, CLR_SCROLL_BAR);
        if (totalRows > paletteRowsVisible) {
            float t = (float)scrollOffsetRows / (float)(totalRows - paletteRowsVisible);
            int knobH = sbH * paletteRowsVisible / totalRows;
            if (knobH < 6) knobH = 6;
            int knobY = sbY + (int)((sbH - knobH) * t);
            fill(sbX + 1, knobY, sbX + sbW - 1, knobY + knobH, CLR_SCROLL_HND);
        }
    }

    // Hotbar (read-only display of the player's current hotbar).
    Inventory* inv = minecraft->player ? minecraft->player->inventory : NULL;
    int hoveredHotbar = hitTestHotbar(xm, ym);
    for (int col = 0; col < 9; ++col) {
        int sx = hotbarX + col * SLOT_SIZE;
        int sy = hotbarY;
        fill(sx,            sy,            sx + SLOT_SIZE, sy + SLOT_SIZE, CLR_FRAME);
        fill(sx + SLOT_PAD, sy + SLOT_PAD, sx + SLOT_SIZE - SLOT_PAD, sy + SLOT_SIZE - SLOT_PAD, CLR_SLOT);

        if (inv) {
            int targetSlot = inv->linkedSlots[col].inventorySlot;
            ItemInstance* it = (targetSlot >= 0) ? inv->getItem(targetSlot) : NULL;
            if (it && !it->isNull()) {
                ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, it,
                    (float)(sx + SLOT_PAD), (float)(sy + SLOT_PAD + 1), true);
                minecraft->gui.renderSlotText(it, (float)(sx + SLOT_PAD - 3), (float)(sy + SLOT_PAD - 3), true, true);
            }
            // Mark the currently-selected hotbar slot.
            if (col == inv->selected) {
                int c = 0xff00ff00;
                fill(sx,            sy,            sx + SLOT_SIZE, sy + 1,       c);
                fill(sx,            sy + SLOT_SIZE - 1, sx + SLOT_SIZE, sy + SLOT_SIZE, c);
                fill(sx,            sy,            sx + 1,         sy + SLOT_SIZE, c);
                fill(sx + SLOT_SIZE - 1, sy,            sx + SLOT_SIZE, sy + SLOT_SIZE, c);
            }
        }
        if (hoveredHotbar == col) {
            fill(sx + SLOT_PAD, sy + SLOT_PAD, sx + SLOT_SIZE - SLOT_PAD, sy + SLOT_SIZE - SLOT_PAD, CLR_HOVER);
        }
    }

    // Tooltip for hovered tab (full category name).
    if (hoveredTabIdx >= 0 && hoveredEntryIdx < 0) {
        std::string label = tabs[hoveredTabIdx].label;
        int tw = (int)minecraft->font->width(label) + 6;
        int th = 12;
        int tx = xm + 8;
        int ty = ym - 14;
        if (tx + tw > width) tx = width - tw;
        if (ty < 0) ty = 0;
        fill(tx - 1, ty - 1, tx + tw + 1, ty + th + 1, CLR_TIP_FR);
        fill(tx,     ty,     tx + tw,     ty + th,     CLR_TIP_BG);
        minecraft->font->drawShadow(label, (float)(tx + 3), (float)(ty + 2), CLR_TEXT);
    }

    // Tooltip for hovered palette entry.
    if (hoveredEntryIdx >= 0) {
        const Entry& e = entries[filtered[hoveredEntryIdx]];
        ItemInstance copy = e.item;
        std::string name = copy.getName();
        if (!name.empty()) {
            int tw = (int)minecraft->font->width(name) + 6;
            int th = 12;
            int tx = xm + 8;
            int ty = ym - 14;
            if (tx + tw > width) tx = width - tw;
            if (ty < 0) ty = 0;
            fill(tx - 1, ty - 1, tx + tw + 1, ty + th + 1, CLR_TIP_FR);
            fill(tx,     ty,     tx + tw,     ty + th,     CLR_TIP_BG);
            minecraft->font->drawShadow(name, (float)(tx + 3), (float)(ty + 2), CLR_TEXT);
        }
    }

    // Title.
    {
        const char* label = "Creative Inventory";
        int lw = (int)minecraft->font->width(std::string(label));
        minecraft->font->drawShadow(std::string(label),
            (float)(bgX + (bgW - lw) / 2),
            (float)(bgY - 12), CLR_TEXT);
    }

    Screen::render(xm, ym, a);
}
