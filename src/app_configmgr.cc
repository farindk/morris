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

#include "app_configmgr.hh"
#include "mainapp.hh"
#include "gtk_menutoolbar.hh"  // TODO: gtk is a special implementation !
#include "algo_alphabeta.hh"

ConfigManager_Application::ConfigManager_Application()
{
}

ConfigManager_Application::~ConfigManager_Application()
{
  g_object_unref(settings);
  g_object_unref(disp_settings);
  g_object_unref(ai_settings);
  g_object_unref(compA_settings);
  g_object_unref(compB_settings);
  g_object_unref(weightsA_settings);
  g_object_unref(weightsB_settings);
}

void ConfigManager_Application::readInitialValues()
{
  /* TODO: this should somehow be in the base class */

  store(settings, itemPref_pauseOnAIPlayer,
        read_bool(settings, itemPref_pauseOnAIPlayer));
  store(settings, itemPref_showLogOfMoves,
        read_bool(settings, itemPref_showLogOfMoves));
  store(disp_settings, itemDisplay_showGameOverMessageBox,
        read_bool(disp_settings, itemDisplay_showGameOverMessageBox));
  store(disp_settings, itemDisplayGtk_showCoordinates,
        read_bool(disp_settings, itemDisplayGtk_showCoordinates));
  store(disp_settings, itemDisplayGtk_coloredCrossingsWhileDragging,
        read_bool(disp_settings, itemDisplayGtk_coloredCrossingsWhileDragging));
  store(disp_settings, itemDisplayGtk_animateComputerMoves,
        read_bool(disp_settings, itemDisplayGtk_animateComputerMoves));
  store(disp_settings, itemDisplayGtk_animateSettingOfPieces,
        read_bool(disp_settings, itemDisplayGtk_animateSettingOfPieces));
  store(disp_settings, itemDisplayGtk_animateTakes,
        read_bool(disp_settings, itemDisplayGtk_animateTakes));

  store(disp_settings, itemDisplayGtk_animationSpeed,
        read_float(disp_settings, itemDisplayGtk_animationSpeed));
  store(disp_settings, itemDisplayGtk_takePieceDelay,
        read_float(disp_settings, itemDisplayGtk_takePieceDelay));

  for (int i=0;i<2;i++)
    {
      GSettings* obj;

      if (i == 0)
        obj = compA_settings;
      else
        obj = compB_settings;

      store(obj, itemComputer_maxTime[i],
            read_int(obj, itemComputer_maxTime[i]));
      store(obj, itemComputer_maxDepth[i],
            read_int(obj, itemComputer_maxDepth[i]));

      if (i == 0)
        obj = weightsA_settings;
      else
        obj = weightsB_settings;

      store(obj, itemComputer_weightMaterial[i],
            read_float(obj, itemComputer_weightMaterial[i]));
      store(obj, itemComputer_weightFreedom[i],
            read_float(obj, itemComputer_weightFreedom[i]));
      store(obj, itemComputer_weightMills[i],
            read_float(obj, itemComputer_weightMills[i]));
      store(obj, itemComputer_weightExperience[i],
            read_float(obj, itemComputer_weightExperience[i]));
    }

  store(ai_settings, itemComputers_shareTTables,
        read_bool(ai_settings, itemComputers_shareTTables));
}

void ConfigManager_Application::store(GSettings* settings, const char* key, int   value)
{
  if (read_int(settings,key)==value)
    return;

  g_settings_set_int(settings,key,value);

  {
    PlayerIF_AlgoAB* p;
    for (int i=0;i<2;i++)
      {
	p = dynamic_cast<PlayerIF_AlgoAB*>(MainApp::app().getAIPlayer(i).get());

	if (cmp(key,itemComputer_maxTime[i]))
	  {
	    p->setMaxTime_msec(value);
	    return;
	  }
	else if (cmp(key,itemComputer_maxDepth[i]))
	  {
	    p->setMaxDepth(value);
	    return;
	  }
      }
  }

  if (m_delegate != NULL)
    {
      m_delegate->store(settings, key, value);
    }
}

void ConfigManager_Application::store(GSettings* settings, const char* key, bool  value)
{
  if (read_bool(settings,key)==value)
    return;

  g_settings_set_boolean(settings,key,value);

  if (cmp(key,itemPref_pauseOnAIPlayer))
    {
      MainApp::app().options.alwaysPauseOnAIPlayer = value;
      menu_setPauseOnAI(value);
    }
  else if (cmp(key,itemComputers_shareTTables))
    {
      MainApp::app().setShareTTables(value);
    }
  else if (m_delegate != NULL)
    {
      m_delegate->store(settings,key,value);
    }
}

void ConfigManager_Application::store(GSettings* settings, const char* key, float value)
{
  if (read_float(settings,key)==value)
    return;

  g_settings_set_double(settings,key,value);

  for (int i=0;i<2;i++)
    {
      PlayerIF_AlgoAB* p = dynamic_cast<PlayerIF_AlgoAB*>(MainApp::app().getAIPlayer(i).get());

      if (cmp(key,itemComputer_weightMaterial[i]))
	{
	  p->setEvalWeight(PlayerIF_AlgoAB::Weight_Material, value);
	  return;
	}
      else if (cmp(key,itemComputer_weightFreedom[i]))
	{
	  p->setEvalWeight(PlayerIF_AlgoAB::Weight_Freedom, value);
	  return;
	}
      else if (cmp(key,itemComputer_weightMills[i]))
	{
	  p->setEvalWeight(PlayerIF_AlgoAB::Weight_Mills, value);
	  return;
	}
      else if (cmp(key,itemComputer_weightExperience[i]))
	{
	  p->setEvalWeight(PlayerIF_AlgoAB::Weight_Experience, value);
	  return;
	}
    }


  // delegate

  if (m_delegate != NULL)
    {
      m_delegate->store(settings,key,value);
    }
}

void ConfigManager_Application::store(GSettings* settings, const char* key, const char* value)
{
  if (strcmp(read_string(settings,key).c_str(),value)==0)
    return;

  if (m_delegate != NULL)
    {
      m_delegate->store(settings,key,value);
    }
}

int ConfigManager_Application::read_int(GSettings* settings, const char* key)
{
  return g_settings_get_int(settings,key);
}

bool ConfigManager_Application::read_bool(GSettings* settings, const char* key)
{
  return g_settings_get_boolean(settings,key);
}

float ConfigManager_Application::read_float(GSettings* settings, const char* key)
{
  return g_settings_get_double(settings,key);
}

std::string ConfigManager_Application::read_string(GSettings* settings, const char* key)
{
  if (m_delegate != NULL)
    {
      return m_delegate->read_string(settings,key);
    }
}
