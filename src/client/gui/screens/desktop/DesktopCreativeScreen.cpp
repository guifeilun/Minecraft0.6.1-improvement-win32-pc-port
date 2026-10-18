#include "DesktopCreativeScreen.h"

#include "../../../Minecraft.h"
#include "../../../player/LocalPlayer.h"
#include "../../../renderer/entity/ItemRenderer.h"
#include "../../../renderer/Tesselator.h"
#include "../../../sound/SoundEngine.h"
#include "../../Font.h"
#include "../../Gui.h"
#include "../../components/ImageButton.h"
#include "../../components/NinePatch.h"
#include "../../../../world/entity/player/Inventory.h"
#include "../../../../world/entity/player/Player.h"
#include "../../../../world/item/Item.h"
#include "../../../../world/item/ItemCategory.h"
#include "../../../../world/item/ItemInstance.h"
#include "../../../../locale/I18n.h"
#include "../../../../world/level/tile/Tile.h"
#include "platform/input/Mouse.h"
#include "platform/input/Keyboard.h"
#include "../../../Options.h"
#include <cmath>

namespace {
const int SLOT_SIZE   = 18;
const int SLOT_PAD    = 1;
const int SLOT_INNER  = 16;
const int PALETTE_COLS = 9;
const int TAB_H       = 18;
const int HOTBAR_GAP  = 10;
const int INNER_PAD   = 8;
const int ScrollStartThreshold = 10;

const int CLR_DIM        = 0x60101020;
const int CLR_TEXT       = 0xffe0e0e0;
const int CLR_TIP_BG     = 0xe0100010;
const int CLR_TIP_FR     = 0xff5000ff;

const int CAT_ALL = -1;

struct AuxRange { int id; int auxLo; int auxHi; };
const AuxRange AUX_EXPAND[] = {
    {  35, 0, 15 },
    { 256 + 95, 0, 15 },
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
    hoveredTabIdx(-1),
    btnClose(NULL),
    guiBackground(NULL), guiSlot(NULL), guiSlotMarker(NULL),
    guiTabNormal(NULL), guiTabSelected(NULL),
    trackingScrollGesture(false), scrollingGesture(false),
    dragStartX(0), dragStartY(0), lastDragY(0),
    touchStartX(0), touchStartY(0)
{
    Tab t;
    t.label = "All";           t.category = CAT_ALL;                  t.iconItemId = Item::compass ? Item::compass->id : 256 + 89; t.x = 0; tabs.push_back(t);
    t.label = "Blocks";        t.category = ItemCategory::Structures; t.iconItemId = 1;        t.x = 0; tabs.push_back(t);
    t.label = "Tools";         t.category = ItemCategory::Tools;      t.iconItemId = 256 + 1;  t.x = 0; tabs.push_back(t);
    t.label = "Food & Armor";  t.category = ItemCategory::FoodArmor;  t.iconItemId = 256 + 4;  t.x = 0; tabs.push_back(t);
    t.label = "Decorations";   t.category = ItemCategory::Decorations;t.iconItemId = 50;       t.x = 0; tabs.push_back(t);
    t.label = "Mechanisms";    t.category = ItemCategory::Mechanisms; t.iconItemId = 256 + 75; t.x = 0; tabs.push_back(t);
}

DesktopCreativeScreen::~DesktopCreativeScreen() {
    delete btnClose;
    delete guiBackground;
    delete guiSlot;
    delete guiSlotMarker;
    delete guiTabSelected;
}

void DesktopCreativeScreen::init() {
    super::init();
    rebuildEntries();
    layout();

    NinePatchFactory builder(minecraft->textures, "gui/spritesheet.png");
    guiBackground  = builder.createSymmetrical(IntRectangle(0, 0, 16, 16), 4, 4);
    guiSlot        = builder.createSymmetrical(IntRectangle(0, 32, 8, 8), 3, 3);
    guiSlotMarker  = builder.createSymmetrical(IntRectangle(10, 42, 16, 16), 5, 5);
    guiTabNormal   = guiSlot;
    guiTabSelected = builder.createSymmetrical(IntRectangle(0, 44, 8, 8), 3, 3);

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

    trackingScrollGesture = false;
    scrollingGesture = false;
}

void DesktopCreativeScreen::setupPositions() {
    layout();

    if (btnClose) {
        btnClose->width  = 19;
        btnClose->height = 19;
        btnClose->x = bgX + bgW - btnClose->width;
        btnClose->y = bgY;
    }
    if (guiBackground) guiBackground->setSize((float)bgW, (float)bgH);
}

void DesktopCreativeScreen::removed() {
    minecraft->gui.inventoryUpdated();
}

void DesktopCreativeScreen::rebuildEntries() {
    entries.clear();

    for (int i = 0; i < Item::MAX_ITEMS; ++i) {
        Item* it = Item::items[i];
        if (!it) continue;
        if (it->category < 0) continue;

        const AuxRange* expand = NULL;
        for (size_t a = 0; a < sizeof(AUX_EXPAND)/sizeof(AUX_EXPAND[0]); ++a) {
            if (AUX_EXPAND[a].id == i) { expand = &AUX_EXPAND[a]; break; }
        }

        int lo = expand ? expand->auxLo : 0;
        int hi = expand ? expand->auxHi : 0;
        for (int aux = lo; aux <= hi; ++aux) {
            Entry e;
            e.item = ItemInstance(it, 1, aux);
            e.category = it->category;
            entries.push_back(e);
        }
    }

    filtered.clear();
    for (size_t i = 0; i < entries.size(); ++i) {
        if (activeCategory == CAT_ALL || entries[i].category == activeCategory)
            filtered.push_back((int)i);
    }
    scrollOffsetRows = 0;
}

void DesktopCreativeScreen::layout() {
    paletteCols = PALETTE_COLS;
    paletteRowsVisible = 7;

    int paletteContentW = paletteCols * SLOT_SIZE;
    int paletteContentH = paletteRowsVisible * SLOT_SIZE;

    int hotbarRowW = 9 * SLOT_SIZE;
    int innerW = paletteContentW + 12;
    bgW = innerW + 2 * INNER_PAD;
    bgH = TAB_H + 2 + paletteContentH + HOTBAR_GAP + SLOT_SIZE + 2 * INNER_PAD;

    bgX = (width  - bgW) / 2;
    bgY = (height - bgH) / 2;
    if (bgY < 22) bgY = 22;

    tabsY = bgY + INNER_PAD;
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

void DesktopCreativeScreen::giveToHotbar(const ItemInstance& item) {
    if (item.isNull()) return;
    if (!minecraft || !minecraft->player || !minecraft->player->inventory) return;
    Inventory* inv = minecraft->player->inventory;
    int sel = inv->selected;
    if (sel < 0 || sel >= inv->getNumLinkedSlots()) return;

    int targetSlot = inv->linkedSlots[sel].inventorySlot;
    if (targetSlot < 0) {
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

void DesktopCreativeScreen::tick() {
    super::tick();

    int xm = Mouse::getX();
    int ym = Mouse::getY();
    if (minecraft->screen)
        minecraft->screen->toGUICoordinate(xm, ym);

    bool leftDown = Mouse::isButtonDown(MouseAction::ACTION_LEFT);

    if (trackingScrollGesture && leftDown) {
        int dy = ym - lastDragY;
        if (!scrollingGesture) {
            int totalDx = xm - dragStartX;
            int totalDy = ym - dragStartY;
            if (std::abs(totalDx) >= ScrollStartThreshold ||
                std::abs(totalDy) >= ScrollStartThreshold) {
                if (std::abs(totalDy) >= std::abs(totalDx)) {
                    scrollingGesture = true;
                }
            }
        }
        if (scrollingGesture && dy != 0) {
            int totalRows = ((int)filtered.size() + paletteCols - 1) / paletteCols;
            int maxScroll = totalRows - paletteRowsVisible;
            if (maxScroll < 0) maxScroll = 0;
            scrollOffsetRows -= (dy > 0 ? 1 : -1);
            if (scrollOffsetRows < 0) scrollOffsetRows = 0;
            if (scrollOffsetRows > maxScroll) scrollOffsetRows = maxScroll;
        }
        lastDragY = ym;
    }
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
    super::keyPressed(eventKey);
}

void DesktopCreativeScreen::buttonClicked(Button* button) {
    if (button == btnClose) { minecraft->setScreen(NULL); return; }
    super::buttonClicked(button);
}

void DesktopCreativeScreen::mouseClicked(int x, int y, int buttonNum) {
    trackingScrollGesture = false;
    scrollingGesture = false;

    if (buttonNum == MouseAction::ACTION_LEFT) {
        int paletteIdx = hitTestPaletteCell(x, y);
        if (paletteIdx >= 0) {
            trackingScrollGesture = true;
            dragStartX = x;
            dragStartY = y;
            lastDragY = y;
            touchStartX = x;
            touchStartY = y;
            return;
        }
    }

    if (buttonNum != MouseAction::ACTION_LEFT && buttonNum != MouseAction::ACTION_RIGHT) {
        super::mouseClicked(x, y, buttonNum);
        return;
    }

    int tabIdx = hitTestTab(x, y);
    if (tabIdx >= 0) {
        activeCategory = tabs[tabIdx].category;
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

void DesktopCreativeScreen::mouseReleased(int x, int y, int buttonNum) {
    if (trackingScrollGesture && !scrollingGesture) {
        int paletteIdx = hitTestPaletteCell(touchStartX, touchStartY);
        if (paletteIdx >= 0) {
            const Entry& e = entries[filtered[paletteIdx]];
            ItemInstance give = e.item;
            if (buttonNum == MouseAction::ACTION_LEFT)
                give.count = (give.id < Item::MAX_ITEMS && Item::items[give.id])
                             ? Item::items[give.id]->getMaxStackSize() : 64;
            else
                give.count = 1;
            giveToHotbar(give);
            if (minecraft->soundEngine)
                minecraft->soundEngine->playUI("random.click", 1, 1);
        }
    }
    trackingScrollGesture = false;
    scrollingGesture = false;
    super::mouseReleased(x, y, buttonNum);
}

void DesktopCreativeScreen::drawTooltip(int xm, int ym, const std::string& text) {
    int tw = (int)minecraft->font->width(text) + 6;
    int th = 12;
    int tx = xm + 8;
    int ty = ym - 14;
    if (tx + tw > width) tx = width - tw;
    if (ty < 0) ty = 0;
    fill(tx - 1, ty - 1, tx + tw + 1, ty + th + 1, CLR_TIP_FR);
    fill(tx,     ty,     tx + tw,     ty + th,     CLR_TIP_BG);
    minecraft->font->drawShadow(text, (float)(tx + 3), (float)(ty + 2), CLR_TEXT);
}

void DesktopCreativeScreen::render(int xm, int ym, float a) {
    Tesselator& t = Tesselator::instance;

    fill(0, 0, width, height, CLR_DIM);

    guiBackground->setSize((float)bgW, (float)bgH);
    guiBackground->draw(t, (float)bgX, (float)bgY);

    hoveredTabIdx = hitTestTab(xm, ym);
    for (int i = 0; i < (int)tabs.size(); ++i) {
        int tx = tabs[i].x;
        bool active = (tabs[i].category == activeCategory);
        NinePatchLayer* patch = active ? guiTabSelected : guiTabNormal;
        patch->setSize((float)tabW, (float)TAB_H);
        patch->draw(t, (float)tx, (float)tabsY);

        if (tabs[i].iconItemId > 0 && tabs[i].iconItemId < Item::MAX_ITEMS
            && Item::items[tabs[i].iconItemId])
        {
            ItemInstance icon(Item::items[tabs[i].iconItemId], 1, 0);
            int ix = tx + (tabW - SLOT_INNER) / 2;
            int iy = tabsY + (TAB_H - SLOT_INNER) / 2;
            ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, &icon,
                (float)ix, (float)iy, true);
        }
    }

    hoveredEntryIdx = hitTestPaletteCell(xm, ym);

    guiSlot->setSize((float)SLOT_SIZE, (float)SLOT_SIZE);
    for (int row = 0; row < paletteRowsVisible; ++row) {
        for (int col = 0; col < paletteCols; ++col) {
            int sx = paletteX + col * SLOT_SIZE;
            int sy = paletteY + row * SLOT_SIZE;
            guiSlot->draw(t, (float)sx, (float)sy);
            int idx = (scrollOffsetRows + row) * paletteCols + col;
            if (idx >= 0 && idx < (int)filtered.size()) {
                const Entry& e = entries[filtered[idx]];
                ItemInstance copy = e.item;
                ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, &copy,
                    (float)(sx + SLOT_PAD), (float)(sy + SLOT_PAD), true);
            }
        }
    }

    if (hoveredEntryIdx >= 0) {
        int row = hoveredEntryIdx / paletteCols - scrollOffsetRows;
        int col = hoveredEntryIdx % paletteCols;
        int sx = paletteX + col * SLOT_SIZE;
        int sy = paletteY + row * SLOT_SIZE;
        guiSlotMarker->setSize((float)SLOT_SIZE + 4, (float)SLOT_SIZE + 4);
        guiSlotMarker->draw(t, (float)(sx - 2), (float)(sy - 2));
    }

    {
        int totalRows = ((int)filtered.size() + paletteCols - 1) / paletteCols;
        int sbX = paletteX + paletteW + 2;
        int sbY = paletteY;
        int sbW = 6;
        int sbH = paletteH;

        guiSlot->setSize((float)sbW, (float)sbH);
        guiSlot->draw(t, (float)sbX, (float)sbY);

        if (totalRows > paletteRowsVisible) {
            float tt = (float)scrollOffsetRows / (float)(totalRows - paletteRowsVisible);
            int knobH = sbH * paletteRowsVisible / totalRows;
            if (knobH < 6) knobH = 6;
            int knobY = sbY + (int)((sbH - knobH) * tt);
            guiSlotMarker->setSize((float)sbW, (float)knobH);
            guiSlotMarker->draw(t, (float)sbX, (float)knobY);
        }
    }

    Inventory* inv = minecraft->player ? minecraft->player->inventory : NULL;
    int hoveredHotbar = hitTestHotbar(xm, ym);

    guiSlot->setSize((float)SLOT_SIZE, (float)SLOT_SIZE);
    for (int col = 0; col < 9; ++col) {
        int sx = hotbarX + col * SLOT_SIZE;
        int sy = hotbarY;
        guiSlot->draw(t, (float)sx, (float)sy);
        if (inv) {
            int targetSlot = inv->linkedSlots[col].inventorySlot;
            ItemInstance* it = (targetSlot >= 0) ? inv->getItem(targetSlot) : NULL;
            if (it && !it->isNull()) {
                ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, it,
                    (float)(sx + SLOT_PAD), (float)(sy + SLOT_PAD), true);
            }
        }
    }

    if (inv) {
        int sx = hotbarX + inv->selected * SLOT_SIZE;
        guiSlotMarker->setSize((float)SLOT_SIZE + 4, (float)SLOT_SIZE + 4);
        guiSlotMarker->draw(t, (float)(sx - 2), (float)(hotbarY - 2));
    }
    if (hoveredHotbar >= 0) {
        int sx = hotbarX + hoveredHotbar * SLOT_SIZE;
        guiSlotMarker->setSize((float)SLOT_SIZE + 4, (float)SLOT_SIZE + 4);
        guiSlotMarker->draw(t, (float)(sx - 2), (float)(hotbarY - 2));
    }

    for (int col = 0; col < 9; ++col) {
        int sx = hotbarX + col * SLOT_SIZE;
        int sy = hotbarY;
        if (inv) {
            int targetSlot = inv->linkedSlots[col].inventorySlot;
            ItemInstance* it = (targetSlot >= 0) ? inv->getItem(targetSlot) : NULL;
            if (it && !it->isNull()) {
                minecraft->gui.renderSlotText(it,
                    (float)(sx + SLOT_PAD - 3), (float)(sy + SLOT_PAD - 3), true, true);
            }
        }
    }

    {
        std::string label = I18n::get("container.creative");
        int lw = (int)minecraft->font->width(label);
        minecraft->font->drawShadow(label,
            (float)(bgX + (bgW - lw) / 2),
            (float)(bgY - 12), CLR_TEXT);
    }

    Screen::render(xm, ym, a);

    if (hoveredTabIdx >= 0 && hoveredEntryIdx < 0) {
        drawTooltip(xm, ym, tabs[hoveredTabIdx].label);
    }
    if (hoveredEntryIdx >= 0) {
        ItemInstance copy = entries[filtered[hoveredEntryIdx]].item;
        std::string name = copy.getName();
        if (!name.empty())
            drawTooltip(xm, ym, name);
    }
}