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

#ifndef GCONF_CONFIGMGR_HH
#define GCONF_CONFIGMGR_HH

#include "configmgr.hh"
#include "app_configmgr.hh"

#include <gconf/gconf-client.h>


class ConfigManager_GConf : public ConfigManager
{
public:
  ConfigManager_GConf(int argc, char** argv);
  ~ConfigManager_GConf();

  virtual void readInitialValues();

  virtual void store(const char* key, int   value);
  virtual void store(const char* key, bool  value);
  virtual void store(const char* key, float value);
  virtual void store(const char* key, const char* value);

  virtual int   read_int   (const char* key);
  virtual bool  read_bool  (const char* key);
  virtual float read_float (const char* key);
  virtual std::string read_string(const char* key);

  void setAppConfigDelegate(configmanager_ptr c) { appConfig.setDelegate(c); }

private:
  ConfigManager_Application appConfig;
  GConfClient* client;

  friend void cb_gconf_change_notify(GConfClient *client,
				     guint cnxn_id,
				     GConfEntry *entry,
				     gpointer user_data);

  void readInitialBoolValue(const char* key);
  void readInitialFloatValue(const char* key);
  void readInitialIntValue(const char* key);
};

#endif
