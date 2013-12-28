/*
	$port: user_menue_setup.cpp,v 1.1 2010/07/30 20:52:31 tuxbox-cvs Exp $

	user_menue setup implementation - Neutrino-GUI
	based up implementation by Günther

	Copyright (C) 2001 Steffen Hehn 'McClean'
	and some other guys
	Homepage: http://dbox.cyberphoria.org/

	Copyright (C) 2010 T. Graf 'dbt'
	Homepage: http://www.dbox2-tuning.net/


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "user_menue_setup.h"

#include <global.h>
#include <neutrino.h>
#include <neutrino_menue.h>
#include <gui/widget/icons.h>
#include <gui/widget/stringinput.h>
#include <gui/widget/keychooser.h>
#include <gui/widget/messagebox.h>

#include <driver/screen_max.h>

#include <system/debug.h>

CUserMenuSetup::CUserMenuSetup(neutrino_locale_t menue_title, int menue_button)
{
	local = menue_title;
	button = menue_button;
	max_char = 24;
	width = w_max (40, 10);
	pref_name = g_settings.usermenu_text[button]; //set current button name as prefered name
	ums = NULL;
}

CUserMenuSetup::~CUserMenuSetup()
{
	delete ums;
}

int CUserMenuSetup::exec(CMenuTarget* parent, const std::string &)
{
	if(parent != NULL)
		parent->hide();
	
	int res = showSetup();
	checkButtonName();
	
	return res; 
}

static bool usermenu_show = true;
#if HAVE_SPARK_HARDWARE
static bool usermenu_show_three_d_mode = true;
#else
static bool usermenu_show_three_d_mode = false;
#endif
#if HAVE_SPARK_HARDWARE
static bool usermenu_show_cam = false; // FIXME -- use hwcaps?
#else
static bool usermenu_show_cam = true; // FIXME -- use hwcaps?
#endif
struct keyvals
{
	const int key;
	const neutrino_locale_t value;
	bool &show;
};

static keyvals usermenu_items[] =
{
	{ SNeutrinoSettings::ITEM_BAR,			LOCALE_USERMENU_ITEM_BAR,		usermenu_show },
	{ SNeutrinoSettings::ITEM_EPG_LIST,		LOCALE_EPGMENU_EVENTLIST,		usermenu_show },
	{ SNeutrinoSettings::ITEM_EPG_SUPER,		LOCALE_EPGMENU_EPGPLUS,			usermenu_show },
	{ SNeutrinoSettings::ITEM_EPG_INFO,		LOCALE_EPGMENU_EVENTINFO,		usermenu_show },
	{ SNeutrinoSettings::ITEM_EPG_MISC,		LOCALE_USERMENU_ITEM_EPG_MISC,		usermenu_show },
	{ SNeutrinoSettings::ITEM_AUDIO_SELECT,		LOCALE_AUDIOSELECTMENUE_HEAD,		usermenu_show },
	{ SNeutrinoSettings::ITEM_SUBCHANNEL,		LOCALE_INFOVIEWER_SUBSERVICE,		usermenu_show },
	{ SNeutrinoSettings::ITEM_MOVIEPLAYER_MB,	LOCALE_MOVIEBROWSER_HEAD,		usermenu_show },
	{ SNeutrinoSettings::ITEM_TIMERLIST,		LOCALE_TIMERLIST_NAME,			usermenu_show },
	{ SNeutrinoSettings::ITEM_REMOTE,		LOCALE_RCLOCK_MENUEADD,			usermenu_show },
	{ SNeutrinoSettings::ITEM_FAVORITS,		LOCALE_FAVORITES_MENUEADD,		usermenu_show },
	{ SNeutrinoSettings::ITEM_TECHINFO,		LOCALE_EPGMENU_STREAMINFO,		usermenu_show },
	{ SNeutrinoSettings::ITEM_PLUGIN,		LOCALE_TIMERLIST_PLUGIN,		usermenu_show },
	{ SNeutrinoSettings::ITEM_VTXT,			LOCALE_USERMENU_ITEM_VTXT,		usermenu_show },
	{ SNeutrinoSettings::ITEM_IMAGEINFO,		LOCALE_SERVICEMENU_IMAGEINFO,		usermenu_show },
	{ SNeutrinoSettings::ITEM_BOXINFO,		LOCALE_EXTRA_DBOXINFO,			usermenu_show },
	{ SNeutrinoSettings::ITEM_CAM,			LOCALE_CI_SETTINGS,			usermenu_show_cam },
	{ SNeutrinoSettings::ITEM_CLOCK,		LOCALE_CLOCK_SWITCH_ON,			usermenu_show },
	{ SNeutrinoSettings::ITEM_GAMES,		LOCALE_MAINMENU_GAMES,			usermenu_show },
	{ SNeutrinoSettings::ITEM_SCRIPTS,		LOCALE_MAINMENU_SCRIPTS,		usermenu_show },
	{ SNeutrinoSettings::ITEM_ADZAP,		LOCALE_USERMENU_ITEM_ADZAP,		usermenu_show },
	{ SNeutrinoSettings::ITEM_EMU_RESTART,		LOCALE_SERVICEMENU_RESTART_CAM,		usermenu_show },
	{ SNeutrinoSettings::ITEM_TUNER_RESTART,	LOCALE_SERVICEMENU_RESTART_TUNER,	usermenu_show },
	{ SNeutrinoSettings::ITEM_THREE_D_MODE,		LOCALE_THREE_D_SETTINGS,		usermenu_show_three_d_mode },
	{ SNeutrinoSettings::ITEM_RASS,			LOCALE_RASS_HEAD,			usermenu_show },
	{ SNeutrinoSettings::ITEM_YOUTUBE,		LOCALE_MOVIEPLAYER_YTPLAYBACK,		usermenu_show },
	{ SNeutrinoSettings::ITEM_NETZKINO,		LOCALE_MOVIEPLAYER_NKPLAYBACK,		usermenu_show },
	{ SNeutrinoSettings::ITEM_RECORD,		LOCALE_TIMERLIST_TYPE_RECORD,		usermenu_show },
// order is not important, but ITEM_NONE needs to be last
	{ SNeutrinoSettings::ITEM_NONE,			LOCALE_USERMENU_ITEM_NONE,		usermenu_show }
};

int CUserMenuSetup::showSetup()
{
	std::map<std::string,int> locmap;
	unsigned int i = 0;
	unsigned USERMENU_ITEM_OPTION_COUNT = 0;
	for (; usermenu_items[i].key != SNeutrinoSettings::ITEM_NONE; i++)
		if (usermenu_items[i].show) {
			locmap[std::string(g_Locale->getText(usermenu_items[i].value))] = i;
			USERMENU_ITEM_OPTION_COUNT++;
		}

	struct keyval
	{
		int key;
		neutrino_locale_t value;
	};

	std::vector<CMenuOptionChooser::keyval_ext> vec;
	CMenuOptionChooser::keyval_ext kext;
	kext.valname = NULL;
	kext.key = usermenu_items[i].key;
	kext.value = usermenu_items[i].value;
	vec.push_back(kext);

	i = 1;
	for(std::map<string,int>::iterator it = locmap.begin(); it != locmap.end(); ++it) {
                kext.key = usermenu_items[(*it).second].key;
                kext.value = usermenu_items[(*it).second].value;
		vec.push_back(kext);
		i++;
	}

	if (ums == NULL) {
		mn_widget_id_t widget_id = MN_WIDGET_ID_USERMENU_RED + button; //add up ''button'' and becomes to MN_WIDGET_ID_USERMENU_ GREEN, MN_WIDGET_ID_USERMENU_ YELLOW, MN_WIDGET_ID_USERMENU_BLUE
		ums = new CMenuWidget(local, NEUTRINO_ICON_KEYBINDING, width, widget_id);
	}else{ 
		//if widget not clean, ensure that we have an empty widget without any item and set the last selected item
		int sel = ums->getSelected(); 
		ums->resetWidget(true);
		ums->setSelected(sel);
	}
	
	//CUserMenuNotifier *notify = new CUserMenuNotifier();
	CStringInputSMS name(LOCALE_USERMENU_NAME, &g_settings.usermenu_text[button], 11, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzäöüß/- "/*, notify*/);

	CMenuForwarder * mf = new CMenuForwarder(LOCALE_USERMENU_NAME, true, g_settings.usermenu_text[button],&name);
	
	//-------------------------------------
	ums->addIntroItems();
	//-------------------------------------
	ums->addItem(mf);
	ums->addItem(GenericMenuSeparatorLine);
	//-------------------------------------
	for(int item = 0; item < SNeutrinoSettings::ITEM_MAX && item <13; item++) // Do not show more than 13 items
	{
		char text[max_char];
		snprintf(text,max_char,"%d.",item+1);
		text[max_char-1]=0;// terminate for sure
		ums->addItem(new CMenuOptionChooser(text, &g_settings.usermenu[button][item], vec, true, NULL, CRCInput::RC_nokey, "", true ));	
	}
	
	int res = ums->exec(NULL, "");

	return res;
}

//check items of current button menu and set prefered menue name
void CUserMenuSetup::checkButtonItems()
{
	//count of all items of widget
	int count = ums->getItemsCount();
	
	//count of configured items
	int used_items = getUsedItemsCount();
	
	//warn if no items defined and reset menu name, if empty
	if (used_items == 0){
		if (!g_settings.usermenu_text[button].empty()){
			DisplayInfoMessage(g_Locale->getText(LOCALE_USERMENU_MSG_WARNING_NO_ITEMS));
			g_settings.usermenu_text[button] = "";
		}
		return;
	}
	
	//found configured items and set as prefered name
	for(int i = 0; i < count ; i++)
	{
		if (ums->getItem(i)->isMenueOptionChooser()) //choosers only
		{
			CMenuOptionChooser * opt_c = NULL;
			opt_c = static_cast <CMenuOptionChooser*>(ums->getItem(i));
			neutrino_locale_t opt_locale = usermenu_items[opt_c->getOption()].value;
			int set_key = usermenu_items[opt_c->getOption()].key;
			opt_c = NULL;
			
			if (set_key != SNeutrinoSettings::ITEM_NONE)
				pref_name = g_Locale->getText(opt_locale);
			
			//warn if we have more than 1 items and the name of usermenu ist the same like before, exit function and let user decide, what to do 
			if (used_items > 1 && g_settings.usermenu_text[button]==pref_name){
				DisplayInfoMessage(g_Locale->getText(LOCALE_USERMENU_MSG_WARNING_NAME));
				return;
			}
		}
	}
		
	if (used_items == 1)
		g_settings.usermenu_text[button] = pref_name; //if found only 1 configured item, ensure that the caption of usermenu is the same like this	
}

//check button name for details like empty string and show an user message on issue
void CUserMenuSetup::checkButtonName()
{	
	checkButtonItems();
	
	//exit function, if no items found
	if (getUsedItemsCount() == 0)
		return;
	
	bool is_empty = g_settings.usermenu_text[button].empty();
	if (is_empty)
	{
		std::string 	msg = g_Locale->getText(LOCALE_USERMENU_MSG_INFO_IS_EMPTY);
				msg += g_Locale->getText(usermenu[button].def_name);
				
		DisplayInfoMessage(msg.c_str());
		g_settings.usermenu_text[button] = is_empty ? g_Locale->getText(usermenu[button].def_name) : g_settings.usermenu_text[button].c_str();
	}
}


//get count of used items
int CUserMenuSetup::getUsedItemsCount()
{
	int def_items = 0;
	for(int item = 0; item < SNeutrinoSettings::ITEM_MAX; item++)
		if (g_settings.usermenu[button][item] != 0)
			def_items++;
	
	return def_items;
}

