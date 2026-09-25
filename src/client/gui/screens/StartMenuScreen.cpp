#include "StartMenuScreen.h"
#include "UsernameScreen.h"
#include "SelectWorldScreen.h"
#include "ProgressScreen.h"
#include "JoinGameScreen.h"
#include "OptionsScreen.h"
#include "PauseScreen.h"
#include "PrerenderTilesScreen.h" // test button
#include "../components/ImageButton.h"

#include "../../../util/Mth.h"

#include "../Font.h"
#include "../components/ScrolledSelectionList.h"

#include "../../Minecraft.h"
#include "../../renderer/Tesselator.h"
#include "../../../AppPlatform.h"
#include "../../../LicenseCodes.h"
#include "SimpleChooseLevelScreen.h"
#include "../../renderer/Textures.h"
#include "../../../SharedConstants.h"

// Some kind of default settings, might be overridden in ::init
StartMenuScreen::StartMenuScreen()
{
}

StartMenuScreen::~StartMenuScreen()
{
	delete bHost;
	delete bJoin;
	delete bOptions;
	delete bQuit;
}

void StartMenuScreen::init()
{
	if (minecraft->options.getIntValue(OPTIONS_MENU_STYLE) == 2){
		bHost = new Button(    2, 0, 0, 200, 20, "Singleplayer");
		bJoin = new Button(    3, 0, 0, 200, 20, "Multiplayer");
		bOptions = new Button( 4, 0, 0, 200, 20, "Options...");
		bQuit = new Button( 5, 0, 0, 200, 20, "Ouit Game");
	} else {
		bHost = new Button(    2, 0, 0, 160, 24, "Start Game");
		bJoin = new Button(    3, 0, 0, 160, 24, "Join Game");
		bOptions = new Button( 4, 0, 0, 160, 24, "Options");
		bQuit = new Button( 5, 0, 0, 160, 24, "Ouit Game");
	}
	bJoin->active = bHost->active = bOptions->active = true;

	if (minecraft->options.getStringValue(OPTIONS_USERNAME).empty()) {
		return; // tick() will redirect to UsernameScreen
	}

	buttons.push_back(bHost);
	buttons.push_back(bJoin);
	//buttons.push_back(&bTest);
	buttons.push_back(bQuit);

	tabButtons.push_back(bHost);
	tabButtons.push_back(bJoin);
	tabButtons.push_back(bQuit);

	#ifndef RPI
		buttons.push_back(bOptions);
		tabButtons.push_back(bOptions);
	#endif

    //// add quit button (top right X icon) – match OptionsScreen style
    //{
    //    ImageDef def;
    //    def.name = "gui/touchgui.png";
    //    def.width = 34;
    //    def.height = 26;
    //    def.setSrc(IntRectangle(150, 0, (int)def.width, (int)def.height));
    //    bQuit.setImageDef(def, true);
    //    bQuit.scaleWhenPressed = false;
    //    buttons.push_back(&bQuit);
    //    // don't include in tab navigation
    //}

	copyright = "\xffMojang AB";//. Do not distribute!";

	// always show base version string, suffix was previously added for Android builds
	std::string versionString = Common::getGameVersionString();

	std::string _username = minecraft->options.getStringValue(OPTIONS_USERNAME);
	if (_username.empty()) _username = "unknown";

	username = "Username: " + _username;

	#ifdef DEMO_MODE
	#ifdef __APPLE__
		version = versionString + " (Lite)";
	#else
		version = versionString + " (Demo)";
	#endif
	#else
		#ifdef RPI
			version = "v0.1.1 alpha";//(MCPE " + versionString + " compatible)";
		#else
			version = versionString;
		#endif
	#endif
}

void StartMenuScreen::setupPositions() {
	if (minecraft->options.getIntValue(OPTIONS_MENU_STYLE) == 2){
		int yBase = (height / 2) - 20;

		bHost->y =	 yBase;
		bJoin->y =	 bHost->y + 24;
		bOptions->y = bJoin->y + 24;
		bQuit->y = bOptions->y + 24;
	} else {
		int yBase = height / 2;
		bHost->y =	 yBase;
		bJoin->y =	 bHost->y + 24 + 4;
		bOptions->y = bJoin->y + 24 + 4;
		bQuit->y = bOptions->y + 24 + 4;
	}

	// Center buttons
	bHost->x = (width - bHost->width) / 2;
	bJoin->x = (width - bJoin->width) / 2;
	bOptions->x = (width - bOptions->width) / 2;
	bQuit->x = (width - bQuit->width) / 2;

    //// position quit icon at top-right (use image-defined size)
    //bQuit.x = width - bQuit.width;
    //bQuit.y = 0;
}

void StartMenuScreen::tick() {
}

void StartMenuScreen::buttonClicked(Button* button) {

	if (button->id == bHost->id)
	{
        #if defined(DEMO_MODE) || defined(APPLE_DEMO_PROMOTION)
			minecraft->setScreen( new SimpleChooseLevelScreen("_DemoLevel") );
		#else
			minecraft->screenChooser.setScreen(SCREEN_SELECTWORLD);
		#endif
	}
	if (button->id == bJoin->id)
	{
		minecraft->locateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
	}
	if (button->id == bOptions->id)
	{
		minecraft->setScreen(new OptionsScreen());
	}
	if (button->id == bQuit->id)
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

#if defined(RPI)
	TextureId id = minecraft->textures->loadTexture("gui/pi_title.png");
#else
	TextureId id = minecraft->textures->loadTexture("gui/title.png");
#endif
	const TextureData* data = minecraft->textures->getTemporaryTextureData(id);

	if (data) {
		minecraft->textures->bind(id);

		const float x = (float)width / 2;
		const float y = height/16;
		//const float scale = Mth::Min(
		const float wh = Mth::Min((float)width/2.0f, (float)data->w / 2);
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
	}

#if defined(RPI)
	if (Textures::isTextureIdValid(minecraft->textures->loadAndBindTexture("gui/logo/raknet_high_72.png")))
		blit(0, height - 12, 0, 0, 43, 12, 256, 72+72);
#endif

	drawString(font, version, width - font->width(version) - 2, height - 10, 0xffcccccc);//0x666666);
	drawString(font, copyright, 2, height - 20, 0xffffff);
	// Kolyah35 upstream credit + github-link blit removed — this fork has
	// drifted far enough from that base that the attribution no longer fits.
	Screen::render(xm, ym, a);
}

void StartMenuScreen::mouseClicked(int x, int y, int buttonNum) {
	Screen::mouseClicked(x, y, buttonNum);
}

bool StartMenuScreen::handleBackEvent( bool isDown ) {
	minecraft->quit();
	return true;
}
