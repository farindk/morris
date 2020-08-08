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

#ifndef APP_CONFIGMGR_HH
#define APP_CONFIGMGR_HH

#include "configmgr.hh"


// Helper class.
class ConfigManager_Chained : public ConfigManager
{
public:
  void setDelegate(configmanager_ptr c) { m_delegate=c; }

protected:
  configmanager_ptr m_delegate;
};



class ConfigManager_Application : public ConfigManager_Chained
{
public:
  ConfigManager_Application();
  ~ConfigManager_Application();

  virtual void readInitialValues();

  virtual void store(GSettings* settings, const char* key, int   value);
  virtual void store(GSettings* settings, const char* key, bool  value);
  virtual void store(GSettings* settings, const char* key, float value);
  virtual void store(GSettings* settings, const char* key, const char* value);

  virtual int   read_int   (GSettings* settings, const char* key);
  virtual bool  read_bool  (GSettings* settings, const char* key);
  virtual float read_float (GSettings* settings, const char* key);
  virtual std::string read_string(GSettings* settings, const char* key);
};

#endif
