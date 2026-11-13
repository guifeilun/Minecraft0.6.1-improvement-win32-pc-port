#include "TouchStartMenuScreen.h"
#include "../ProgressScreen.h"
#include "../OptionsScreen.h"
#include "../PauseScreen.h"

#include "../../Font.h"
#include "../../components/ScrolledSelectionList.h"
#include "../../components/GuiElement.h"

#include "../../../Minecraft.h"
#include "../../../renderer/Tesselator.h"
#include "../../../renderer/Textures.h"
#include "../../../renderer/TextureData.h"
#include "../../../../SharedConstants.h"
#include "../../../../AppPlatform.h"
#include "../../../../LicenseCodes.h"
#include "../../../../util/Mth.h"
#include "../../../../locale/I18n.h"

#include "../DialogDefinitions.h"
#include "../SimpleChooseLevelScreen.h"

namespace Touch {

// 
// Start menu screen implementation
//

// Some kind of default settings, might be overridden in ::init
StartMenuScreen::StartMenuScreen()
:	bHost(    2, I18n::get("menu.singleplayer")),   // ← 改
	bJoin(    3, I18n::get("menu.multiplayer")),    // ← 改
	bOptions( 4, I18n::get("menu.options")),        // ← 改
	bQuit(    5, "")
{
	ImageDef def;
	bJoin.width = 75;
	def.width = def.height = (float) bJoin.width;

	def.setSrc(IntRectangle(0, 26, (int)def.width, (int)def.width));
	def.name = "gui/touchgui.png";
	IntRectangle& defSrc = *def.getSrc();

	bOptions.setImageDef(def, true);

	defSrc.y += defSrc.h;
	bHost.setImageDef(def, true);

	defSrc.y += defSrc.h;
	bJoin.setImageDef(def, true);
}

StartMenuScreen::~StartMenuScreen()
{
}

void StartMenuScreen::init()
{
	buttons.push_back(&bHost);
	buttons.push_back(&bJoin);
	buttons.push_back(&bOptions);

	// add quit icon (same look as options header)
	{
		ImageDef def;
		def.name = "gui/touchgui.png";
		def.width = 34;
		def.height = 26;
		def.setSrc(IntRectangle(150, 0, (int)def.width, (int)def.height));
		bQuit.setImageDef(def, true);
		bQuit.scaleWhenPressed = false;
		buttons.push_back(&bQuit);
	}

	tabButtons.push_back(&bHost);
	tabButtons.push_back(&bJoin);
	tabButtons.push_back(&bOptions);

	#ifdef DEMO_MODE
		buttons.push_back(&bBuy);
		tabButtons.push_back(&bBuy);
	#endif

	copyright = "\xffMojang AB";//. Do not distribute!";

	// always show base version string
	std::string versionString = Common::getGameVersionString();

	std::string _username = minecraft->options.getStringValue(OPTIONS_USERNAME);
	if (_username.empty()) _username = "unknown";

	username = "Username: " + _username;

	#ifdef DEMO_MODE
		version = versionString + " (Demo)";
	#else
		version = versionString;
	#endif

	bJoin.active = bHost.active = bOptions.active = true;
}

void StartMenuScreen::setupPositions() {
	int yBase = 2 + height / 3;
	int buttonWidth = bHost.width;
	float spacing = (width - (3.0f * buttonWidth)) / 4;

	bHost.y =	 yBase;
	bJoin.y =	 yBase;
	bOptions.y = yBase;

	// Center buttons
	bJoin.x		= 0*buttonWidth + (int)(1*spacing);
	bHost.x		= 1*buttonWidth + (int)(2*spacing);
	bOptions.x	= 2*buttonWidth + (int)(3*spacing);
    
	// quit icon top-right (use size assigned in init)
	bQuit.x = width - bQuit.width;
	bQuit.y = 0;

	copyrightPosX = width - minecraft->font->width(copyright) - 1;
	versionPosX = (width - minecraft->font->width(version)) / 2;
}

void StartMenuScreen::buttonClicked(::Button* button) {

	if (button->id == bHost.id)
	{
		#if defined(DEMO_MODE)
			minecraft->setScreen( new SimpleChooseLevelScreen("_DemoLevel") );
		#else
			minecraft->screenChooser.setScreen(SCREEN_SELECTWORLD);
		#endif
	}
	if (button->id == bJoin.id)
	{
		minecraft->locateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
	}
	if (button->id == bOptions.id)
	{
		minecraft->setScreen(new OptionsScreen());
	}
	if (button == &bQuit)
	{
		minecraft->quit();
	}
}

bool StartMenuScreen::isInGameScreen() { return false; }

void StartMenuScreen::render( int xm, int ym, float a )
{
	renderBackground();

	// Show current username in the top-left corner
	drawString(font, username, 2, 2, 0xffffffff);
    
    glEnable2(GL_BLEND);

	TextureId id = minecraft->textures->loadTexture("gui/title.png");
	const TextureData* data = minecraft->textures->getTemporaryTextureData(id);

	if (data) {
		minecraft->textures->bind(id);

		const float x = (float)width / 2;
		const float y = 4;
		const float wh = 0.5f * Mth::Min((float)width/2.0f, (float)data->w / 2);
		const float scale = 2.0f * wh / (float)data->w;
		const float h = scale * (float)data->h;

		// Render title text
		Tesselator& t = Tesselator::instance;
		glColor4f2(1, 1, 1, 1);
		t.begin();
			t.vertexUV(x-wh, y+h, blitOffset, 0, 1);
			t.vertexUV(x+wh, y+h, blitOffset, 1, 1);
			t.vertexUV(x+wh, y+0, blitOffset, 1, 0);
			t.vertexUV(x-wh, y+0, blitOffset, 0, 0);
		t.draw();

		drawString(font, version, versionPosX, (int)(y+h)+2, 0xffcccccc);
		drawString(font, copyright, copyrightPosX, height - 10, 0xffffff);
	}
	Screen::render(xm, ym, a);
    glDisable2(GL_BLEND);
}


void StartMenuScreen::mouseClicked(int x, int y, int buttonNum) {
	Screen::mouseClicked(x, y, buttonNum);
}

bool StartMenuScreen::handleBackEvent( bool isDown ) {
	minecraft->quit();
	return true;
}

} // namespace Touch