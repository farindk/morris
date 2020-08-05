/***************************************************************************
  This file is part of Morris.
  Copyright (C) 2009 Dirk Farin <dirk.farin@gmail.com>

  Morris is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

#ifndef APPGTK_CONFIGMGR_HH
#define APPGTK_CONFIGMGR_HH

#include "app_configmgr.hh"


class ConfigManager_AppGtk : public ConfigManager_Chained
{
public:
  virtual void store(const char* key, int   value);
  virtual void store(const char* key, bool  value);
  virtual void store(const char* key, float value);
  virtual void store(const char* key, const char* value);

  virtual int   read_int   (const char* key);
  virtual bool  read_bool  (const char* key);
  virtual float read_float (const char* key);
  virtual std::string read_string(const char* key);
};

#endif
