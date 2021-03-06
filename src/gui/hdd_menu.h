/*
	Neutrino-GUI  -   DBoxII-Project


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


#ifndef __hdd_menu__
#define __hdd_menu__


#include "widget/menue.h"
#include <string>
#include <vector>

using namespace std;

enum {
	fs_ext3,
	fs_ext4,

	fs_max
};

class CHDDDestExec : public CMenuTarget
{
public:
        int exec(CMenuTarget* parent, const std::string&);
};
class CHDDFmtExec : public CMenuTarget
{
public:
        int exec(CMenuTarget* parent, const std::string&);
};
class CHDDChkExec : public CMenuTarget
{
public:
        int exec(CMenuTarget* parent, const std::string&);
};

class CHDDMenuHandler : public CMenuTarget
{
	private:
		int width;
		struct hdd_s {
			std::string devname;
			CMenuForwarder *cmf;
			bool mounted;
		};
		std::vector<hdd_s> hdd_list;
	public:
		CHDDMenuHandler();
		~CHDDMenuHandler();
		int  exec( CMenuTarget* parent,  const std::string &actionkey);
		int  doMenu();
};

#endif

