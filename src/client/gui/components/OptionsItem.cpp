#include "OptionsItem.h"
#include "../../Minecraft.h"
#include "../../../locale/I18n.h"
#include "../../../util/Mth.h"
OptionsItem::OptionsItem( OptionId optionId, std::string label, GuiElement* element )
: GuiElementContainer(false, true, 0, 0, 24, 12),
  m_optionId(optionId),
  m_label(label) {
	  addChild(element);
}

void OptionsItem::setupPositions() {
	int currentHeight = 0;
	for(std::vector<GuiElement*>::iterator it = children.begin(); it != children.end(); ++it) {
		(*it)->x = x + width - (*it)->width - 15;
		(*it)->y = y + currentHeight;
		currentHeight += (*it)->height;
	}
	height = currentHeight;
}

void OptionsItem::render( Minecraft* minecraft, int xm, int ym ) {
	int yOffset = (height - 8) / 2;
	std::string text = m_label;
	if (m_optionId == OPTIONS_GUI_SCALE) {
		int value = minecraft->options.getIntValue(OPTIONS_GUI_SCALE);
		std::string scaleText;
		switch (value) {
		case 0: scaleText = I18n::get("options.guiScale.auto"); break;
		case 1: scaleText = I18n::get("options.guiScale.small"); break;
		case 2: scaleText = I18n::get("options.guiScale.medium"); break;
		case 3: scaleText = I18n::get("options.guiScale.large"); break;
		case 4: scaleText = I18n::get("options.guiScale.larger"); break;
		case 5: scaleText = I18n::get("options.guiScale.largest"); break;
		default: scaleText = I18n::get("options.guiScale.auto"); break;
		}
		text += ": " + scaleText;
	}
	if (m_optionId == OPTIONS_FOG_TYPE) {
		int value = minecraft->options.getIntValue(OPTIONS_FOG_TYPE);
		std::string scaleText;
		switch (value) {
		case 0: scaleText = I18n::get("options.fogType.vanilla"); break;
		case 1: scaleText = I18n::get("options.fogType.java"); break;
		case 2: scaleText = I18n::get("options.fogType.unused"); break;
		}
		text += ": " + scaleText;
	}
	if (m_optionId == OPTIONS_DEBUG_STYLE) {
		int value = minecraft->options.getIntValue(OPTIONS_DEBUG_STYLE);
		std::string scaleText;
		switch (value) {
		case 0: scaleText = I18n::get("options.debugStyle.javaBeta"); break;
		case 1: scaleText = I18n::get("options.debugStyle.custom"); break;
		case 2: scaleText = I18n::get("options.fogType.unused"); break;
		}
		text += ": " + scaleText;
	}
	if (m_optionId == OPTIONS_MENU_STYLE) {
		int value = minecraft->options.getIntValue(OPTIONS_MENU_STYLE);
		std::string scaleText;
		switch (value) {
		case 0: scaleText = I18n::get("options.menuStyle.pocket"); break;
		case 1: scaleText = I18n::get("options.menuStyle.xperia"); break;
		case 2: scaleText = I18n::get("options.menuStyle.java"); break;
		}
		text += ": " + scaleText;
	}
	if (m_optionId == OPTIONS_FOV) {
		int value = minecraft->options.getIntValue(OPTIONS_FOV);
		char buf[16];
		std::snprintf(buf, sizeof(buf), "%d", value);
		text += ": " + std::string(buf);
	}
	if (m_optionId == OPTIONS_VIEW_DISTANCE) {
		int value = minecraft->options.getIntValue(OPTIONS_VIEW_DISTANCE);
		std::string scaleText;
		switch (value) {
		case 0: scaleText = I18n::get("options.renderDistance.far"); break;
		case 1: scaleText = I18n::get("options.renderDistance.normal"); break;
		case 2: scaleText = I18n::get("options.renderDistance.short"); break;
		case 3: scaleText = I18n::get("options.renderDistance.tiny"); break;
		default: scaleText = "Debug"; break;
		}
		text += ": " + scaleText;
	}
	if (m_optionId == OPTIONS_DIFFICULTY) {
		int value = minecraft->options.getIntValue(OPTIONS_DIFFICULTY);
		std::string diffText;
		switch (value) {
		case 0: diffText = I18n::get("options.difficulty.peaceful"); break;
		case 1: diffText = I18n::get("options.difficulty.easy"); break;
		case 2: diffText = I18n::get("options.difficulty.normal"); break;
		case 3: diffText = I18n::get("options.difficulty.hard"); break;
		default: diffText = I18n::get("options.difficulty.normal"); break;
		}
		text += ": " + diffText;
	}
	if (m_optionId == OPTIONS_BRIGHTNESS) {
		// Java-style brightness labels. Four buckets matching Java's vanilla
		// stops: Moody (very dim, no boost), Dark, Normal (default), Bright!
		// (max boost).
		float v = minecraft->options.getProgressValue(OPTIONS_BRIGHTNESS);
		std::string gammaText;
		if      (v <= 0.25f) gammaText = I18n::get("options.gamma.moody");
		else if (v <= 0.50f) gammaText = I18n::get("options.gamma.dark");
		else if (v <= 0.75f) gammaText = I18n::get("options.gamma.normal");
		else                 gammaText = I18n::get("options.gamma.bright");
		text += ": " + gammaText;
	}

	minecraft->font->draw(text, (float)x, (float)y + yOffset, 0x909090, false);
	super::render(minecraft, xm, ym);
}