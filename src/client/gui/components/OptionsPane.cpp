#include "OptionsPane.h"
#include "OptionsGroup.h"
#include "OptionsItem.h"
#include "ImageButton.h"
#include "Slider.h"
#include "../../Minecraft.h"
#include "client/Options.h"

OptionsPane::OptionsPane() {

}

void OptionsPane::setupPositions() {
	int currentHeight = y + 1;
	for(std::vector<GuiElement*>::iterator it = children.begin(); it != children.end(); ++it ) {
		(*it)->width = width;
		(*it)->y = currentHeight;
		(*it)->x = x;
		currentHeight += (*it)->height + 1;
	}
	height = currentHeight;
	super::setupPositions();
}

OptionsGroup& OptionsPane::createOptionsGroup( std::string label ) {
	OptionsGroup* newGroup = new OptionsGroup(label);
	children.push_back(newGroup);
	// create and return a new group index
	return *newGroup;
}

void OptionsPane::createToggle( unsigned int group, std::string label, OptionId option ) {
	if(group > children.size()) return;
	ImageDef def;
	def.setSrc(IntRectangle(160, 206, 39, 20));
	def.name = "gui/touchgui.png";
	def.width = 39 * 0.7f;
	def.height = 20 * 0.7f;
	OptionButton* element = new OptionButton(option);
	element->setImageDef(def, true);
	OptionsItem* item = new OptionsItem(option, label, element);
	((OptionsGroup*)children[group])->addChild(item);
	setupPositions();
}

void OptionsPane::createProgressSlider( Minecraft* minecraft, unsigned int group, std::string label, OptionId option, float progressMin/*=1.0f*/, float progressMax/*=1.0f */ ) {
	if(group > children.size()) return;
	Slider* element = new SliderFloat(minecraft, option);
	element->width = 100;
	element->height = 20;
	OptionsItem* item = new OptionsItem(option, label, element);
	((OptionsGroup*)children[group])->addChild(item);
	setupPositions();
}

void OptionsPane::createStepSlider( Minecraft* minecraft, unsigned int group, std::string label, OptionId option, const std::vector<int>& stepVec ) {
	if(group > children.size()) return;
	Slider* element = new SliderInt(minecraft, option);
	element->width = 100;
	element->height = 20;
	OptionsItem* item = new OptionsItem(option, label, element);
	((OptionsGroup*)children[group])->addChild(item);
	setupPositions();
}
