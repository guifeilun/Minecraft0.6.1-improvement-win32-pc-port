#include "SmallButton.h"

SmallButton::SmallButton( int id, int x, int y, const std::string& msg )
:	super(id, x, y, 150, 20, msg),
	option(NULL)
{
}

SmallButton::SmallButton( int id, int x, int y, int width, int height, const std::string& msg )
:	super(id, x, y, width, height, msg),
	option(NULL)
{
}

SmallButton::SmallButton( int id, int x, int y, Option* item, const std::string& msg )
:	super(id, x, y, 150, 20, msg),
	option(item)
{
}

Option* SmallButton::getOption()
{
	return option;
}
