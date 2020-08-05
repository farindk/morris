/***************************************************************************
  This file is part of Morris.
  Copyright (C) 2009 Dirk Farin

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

#include "gconf_configmgr.hh"
#include "mainapp.hh"


static const char* prefix = "/apps/morris";

static std::string gconfKey(const char* item)
{
  std::string path = prefix;
  path += "/";
  path += std::string(item);
  return path;
}


static const char* mapKey(const char* gconfpath)
{
  return &gconfpath[ strlen(prefix)+1 ];
}


void cb_gconf_change_notify(GConfClient *client,
			    guint cnxn_id,
			    GConfEntry *entry,
			    gpointer user_data)
{
  // value was unset
  if (entry==NULL)
    {
      return;
    }

  ConfigManager_GConf* obj = (ConfigManager_GConf*)(user_data);

  //std::cout << "change notify: " << entry->key << "\n";

  switch (entry->value->type)
    {
    case GCONF_VALUE_STRING:
      obj->appConfig.store(mapKey(entry->key), (const char*)(gconf_value_get_string(entry->value)));
      break;
    case GCONF_VALUE_INT:
      obj->appConfig.store(mapKey(entry->key), int(gconf_value_get_int(entry->value)));
      break;
    case GCONF_VALUE_FLOAT:
      obj->appConfig.store(mapKey(entry->key), float(gconf_value_get_float(entry->value)));
      break;
    case GCONF_VALUE_BOOL:
      obj->appConfig.store(mapKey(entry->key), bool(gconf_value_get_bool(entry->value)));
      break;
    }
}


ConfigManager_GConf::ConfigManager_GConf(int argc, char** argv)
{
  gconf_init(argc, argv, NULL);

  client = gconf_client_get_default();

  gconf_client_add_dir(client, prefix,
                       GCONF_CLIENT_PRELOAD_NONE,
                       NULL);

  gconf_client_notify_add(client, prefix,
			  cb_gconf_change_notify, this,
			  NULL, NULL);
}

ConfigManager_GConf::~ConfigManager_GConf()
{
  g_object_unref(client);
}

void ConfigManager_GConf::readInitialBoolValue(const char* key)
{
  appConfig.store(key, read_bool(key));
}

void ConfigManager_GConf::readInitialFloatValue(const char* key)
{
  appConfig.store(key, read_float(key));
}

void ConfigManager_GConf::readInitialIntValue(const char* key)
{
  appConfig.store(key, read_int(key));
}

void ConfigManager_GConf::readInitialValues()
{
  /* TODO: this should somehow be in the base class */

  readInitialBoolValue(itemPref_pauseOnAIPlayer);
  readInitialBoolValue(itemPref_showLogOfMoves);
  readInitialBoolValue(itemDisplay_showGameOverMessageBox);
  readInitialBoolValue(itemDisplayGtk_showCoordinates);
  readInitialBoolValue(itemDisplayGtk_coloredCrossingsWhileDragging);
  readInitialBoolValue(itemDisplayGtk_animateComputerMoves);
  readInitialBoolValue(itemDisplayGtk_animateSettingOfPieces);
  readInitialBoolValue(itemDisplayGtk_animateTakes);

  readInitialFloatValue(itemDisplayGtk_animationSpeed);
  readInitialFloatValue(itemDisplayGtk_takePieceDelay);

  for (int i=0;i<2;i++)
    {
      readInitialIntValue(itemComputer_maxTime[i]);
      readInitialIntValue(itemComputer_maxDepth[i]);
      readInitialFloatValue(itemComputer_weightMaterial[i]);
      readInitialFloatValue(itemComputer_weightFreedom[i]);
      readInitialFloatValue(itemComputer_weightMills[i]);
      readInitialFloatValue(itemComputer_weightExperience[i]);
    }

  readInitialBoolValue(itemComputers_shareTTables);
}

void ConfigManager_GConf::store(const char* key, int   value)
{
  gconf_client_set_int(client, gconfKey(key).c_str(), value, NULL);
}

void ConfigManager_GConf::store(const char* key, bool  value)
{
  gconf_client_set_bool(client, gconfKey(key).c_str(), value, NULL);
}

void ConfigManager_GConf::store(const char* key, float value)
{
  gconf_client_set_float(client, gconfKey(key).c_str(), value, NULL);
}

void ConfigManager_GConf::store(const char* key, const char* value)
{
  assert(0); // TODO
}

int ConfigManager_GConf::read_int(const char* key)
{
  GConfValue* value = gconf_client_get(client, gconfKey(key).c_str(), NULL);
  if (value==NULL || value->type != GCONF_VALUE_INT)
    {
      return appConfig.read_int(key);
    }

  int v = gconf_value_get_int(value);
  gconf_value_free(value);

  return v;
}

bool ConfigManager_GConf::read_bool(const char* key)
{
  GConfValue* value = gconf_client_get(client, gconfKey(key).c_str(), NULL);
  if (value==NULL || value->type != GCONF_VALUE_BOOL)
    {
      return appConfig.read_bool(key);
    }

  bool v = gconf_value_get_bool(value);
  gconf_value_free(value);

  return v;
}

float ConfigManager_GConf::read_float(const char* key)
{
  GConfValue* value = gconf_client_get(client, gconfKey(key).c_str(), NULL);
  if (value==NULL || value->type != GCONF_VALUE_FLOAT)
    {
      return appConfig.read_float(key);
    }

  float v = gconf_value_get_float(value);
  gconf_value_free(value);

  return v;
}

std::string ConfigManager_GConf::read_string(const char* key)
{
  assert(0); // TODO

  /*
  if (str)
    g_free(str);
  */
}
