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


void ConfigManager_Application::readInitialValues()
{
}

void ConfigManager_Application::store(const char* key, int   value)
{
  if (read_int(key)==value)
    return;

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
      m_delegate->store(key, value);
    }
}

void ConfigManager_Application::store(const char* key, bool  value)
{
  if (read_bool(key)==value)
    return;

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
      m_delegate->store(key,value);
    }
}

void ConfigManager_Application::store(const char* key, float value)
{
  if (read_float(key)==value)
    return;

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
      m_delegate->store(key,value);
    }
}

void ConfigManager_Application::store(const char* key, const char* value)
{
  if (strcmp(read_string(key).c_str(),value)==0)
    return;

  if (m_delegate != NULL)
    {
      m_delegate->store(key,value);
    }
}

int ConfigManager_Application::read_int(const char* key)
{
  {
    const PlayerIF_AlgoAB* p;
    for (int i=0;i<2;i++)
      {
	p = dynamic_cast<const PlayerIF_AlgoAB*>(MainApp::app().getAIPlayer(i).get());

	if (cmp(key,itemComputer_maxTime[i]))
	  {
	    return p->askMaxTime_msec();
	  }
	else if (cmp(key,itemComputer_maxDepth[i]))
	  {
	    return p->askMaxDepth();
	  }
      }
  }


  if (m_delegate != NULL)
    {
      return m_delegate->read_int(key);
    }
}

bool ConfigManager_Application::read_bool(const char* key)
{
  if (cmp(key,itemPref_pauseOnAIPlayer))
    {
      return MainApp::app().options.alwaysPauseOnAIPlayer;
    }
  else if (cmp(key,itemComputers_shareTTables))
    {
      return MainApp::app().getShareTTables();
    }

  if (m_delegate != NULL)
    {
      return m_delegate->read_bool(key);
    }
}

float ConfigManager_Application::read_float(const char* key)
{
  for (int i=0;i<2;i++)
    {
      const PlayerIF_AlgoAB* p = dynamic_cast<const PlayerIF_AlgoAB*>(MainApp::app().getAIPlayer(i).get());

      if (cmp(key,itemComputer_weightMaterial[i]))
	{
	  return p->askEvalWeight(PlayerIF_AlgoAB::Weight_Material);
	}
      else if (cmp(key,itemComputer_weightFreedom[i]))
	{
	  return p->askEvalWeight(PlayerIF_AlgoAB::Weight_Freedom);
	}
      else if (cmp(key,itemComputer_weightMills[i]))
	{
	  return p->askEvalWeight(PlayerIF_AlgoAB::Weight_Mills);
	}
      else if (cmp(key,itemComputer_weightExperience[i]))
	{
	  return p->askEvalWeight(PlayerIF_AlgoAB::Weight_Experience);
	}
    }

  if (m_delegate != NULL)
    {
      return m_delegate->read_float(key);
    }
}

std::string ConfigManager_Application::read_string(const char* key)
{
  if (m_delegate != NULL)
    {
      return m_delegate->read_string(key);
    }
}
