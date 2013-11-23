/*
	Based up Neutrino-GUI - Tuxbox-Project
	Copyright (C) 2001 by Steffen Hehn 'McClean'

	Info Clock Window
	based up CComponentsFrmClock
	Copyright (C) 2013, Thilo Graf 'dbt'
	Copyright (C) 2013, Michael Liebmann 'micha-bbg'

	License: GPL

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	General Public License for more details.

	You should have received a copy of the GNU General Public
	License along with this program; if not, write to the
	Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
	Boston, MA  02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <global.h>
#include <neutrino.h>
#include <gui/volumebar.h>
#include <gui/infoclock.h>



CInfoClock::CInfoClock():CComponentsFrmClock( 0, 0, 0, 50, "%H:%M:%S", true, CC_SHADOW_OFF, COL_LIGHT_GRAY, COL_MENUCONTENT_PLUS_0,COL_MENUCONTENTDARK_PLUS_0)
{
	initVarInfoClock();
}

CInfoClock* CInfoClock::getInstance()
{
	static CInfoClock* InfoClock = NULL;
	if(!InfoClock)
		InfoClock = new CInfoClock();
	return InfoClock;
}

void CInfoClock::initVarInfoClock()
{
	cl_font = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE];
	Init();
}

void CInfoClock::Init()
{
	CVolumeHelper::getInstance()->refresh();
	CVolumeHelper::getInstance()->getInfoClockDimensions(&x, &y, &width, &height);
	initCCLockItems();
}

void CInfoClock::ClearDisplay()
{
	kill();
	Init();
}

bool CInfoClock::StartClock()
{
	if (g_settings.infoclock_no_background) {
		setFrameColor(0);
		setBodyColor(0);
		setShadowColor(0);
	} else {
		setFrameColor(COL_LIGHT_GRAY);
		setBodyColor(COL_MENUCONTENT_PLUS_0);
		setShadowColor(COL_MENUCONTENTDARK_PLUS_0);
	}
	if (g_settings.infoclock_with_seconds) {
		setClockFormat("%H:%M:%S");
	} else {
		setClockFormat("%H:%M");
	}
	Init();
	return Start();
}

bool CInfoClock::StopClock()
{
	bool ret = Stop();
	kill();
	frameBuffer->blit();

	return ret;
}

void CInfoClock::paint(bool do_save_bg)
{
	// calculate current x-position of clock (mute icon on/off)
	x = CVolumeHelper::getInstance()->getInfoClockX();

	//prepare items before paint
	initCCLockItems();

	//paint the clock
	paintForm(do_save_bg);
}

bool CInfoClock::enableInfoClock(bool enable)
{
	bool ret = false;
	if (g_settings.mode_clock) {
		if (enable) {
			if (!paintClock)
				ret = StartClock();
		}
		else {
			if (paintClock)
				ret = StopClock();
		}
	}
	return ret;
}
