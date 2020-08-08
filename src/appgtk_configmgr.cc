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

#include "appgtk_configmgr.hh"
#include "mainapp.hh"
#include "gtk_menutoolbar.hh"
#include "gtk_appgui.hh"
#include "gtkcairo_boardgui.hh"


void ConfigManager_AppGtk::store(GSettings* settings, const char* key, int   value)
{
  if (m_delegate != NULL)
    {
      m_delegate->store(settings, key, value);
    }
}

void ConfigManager_AppGtk::store(GSettings* settings, const char* key, bool  value)
{
  if (cmp(key,itemPref_showLogOfMoves))
    {
      MainApp::app().getApplicationGUI()->showMoveLog(value, false);
    }
  else if (cmp(key,itemDisplayGtk_showCoordinates))
    {
      dynamic_cast<BoardGUI_GtkCairo*>(MainApp::app().getBoardGUI().get())->getOptions().showCoordinates = value;
      MainApp::app().getBoardGUI()->resetDisplay();
    }
  else if (cmp(key,itemDisplayGtk_coloredCrossingsWhileDragging))
    {
      dynamic_cast<BoardGUI_GtkCairo*>(MainApp::app().getBoardGUI().get())->getOptions().coloredCrossingWhileDragging = value;
    }
  else if (cmp(key,itemDisplayGtk_animateComputerMoves))
    {
      dynamic_cast<BoardGUI_GtkCairo*>(MainApp::app().getBoardGUI().get())->getOptions().animateComputerMoves = value;
    }
  else if (cmp(key,itemDisplayGtk_animateSettingOfPieces))
    {
      dynamic_cast<BoardGUI_GtkCairo*>(MainApp::app().getBoardGUI().get())->getOptions().set_moveIn = value;
    }
  else if (cmp(key,itemDisplayGtk_animateTakes))
    {
      dynamic_cast<BoardGUI_GtkCairo*>(MainApp::app().getBoardGUI().get())->getOptions().take_moveOut = value;
    }
  else if (cmp(key,itemDisplay_showGameOverMessageBox))
    {
      ApplicationGUI_Gtk* gui = dynamic_cast<ApplicationGUI_Gtk*>(MainApp::app().getApplicationGUI().get());
      gui->enableGameOverDialog(value);
    }
  else if (m_delegate != NULL)
    {
      m_delegate->store(settings,key,value);
    }
}

void ConfigManager_AppGtk::store(GSettings* settings, const char* key, float value)
{
  if (cmp(key,itemDisplayGtk_animationSpeed))
    {
      dynamic_cast<BoardGUI_GtkCairo*>(MainApp::app().getBoardGUI().get())->getOptions().animationSpeed = value;
    }
  else if (cmp(key,itemDisplayGtk_takePieceDelay))
    {
      dynamic_cast<BoardGUI_GtkCairo*>(MainApp::app().getBoardGUI().get())->getOptions().take_delay = value;
    }
  else if (m_delegate != NULL)
    {
      m_delegate->store(settings,key,value);
    }
}

void ConfigManager_AppGtk::store(GSettings* settings, const char* key, const char* value)
{
  if (m_delegate != NULL)
    {
      m_delegate->store(settings,key,value);
    }
}

int ConfigManager_AppGtk::read_int(GSettings* settings, const char* key)
{
  if (m_delegate != NULL)
    {
      return m_delegate->read_int(settings,key);
    }
}

bool ConfigManager_AppGtk::read_bool(GSettings* settings, const char* key)
{
  if (cmp(key,itemPref_showLogOfMoves))
    {
      return MainApp::app().getApplicationGUI()->isMoveLogShown();
    }
  else if (cmp(key,itemDisplayGtk_showCoordinates))
    {
      return dynamic_cast<BoardGUI_GtkCairo*>(MainApp::app().getBoardGUI().get())->getOptions().showCoordinates;
    }
  else if (cmp(key,itemDisplayGtk_coloredCrossingsWhileDragging))
    {
      return dynamic_cast<BoardGUI_GtkCairo*>(MainApp::app().getBoardGUI().get())->getOptions().coloredCrossingWhileDragging;
    }
  else if (cmp(key,itemDisplayGtk_animateComputerMoves))
    {
      return dynamic_cast<BoardGUI_GtkCairo*>(MainApp::app().getBoardGUI().get())->getOptions().animateComputerMoves;
    }
  else if (cmp(key,itemDisplayGtk_animateSettingOfPieces))
    {
      return dynamic_cast<BoardGUI_GtkCairo*>(MainApp::app().getBoardGUI().get())->getOptions().set_moveIn;
    }
  else if (cmp(key,itemDisplayGtk_animateTakes))
    {
      return dynamic_cast<BoardGUI_GtkCairo*>(MainApp::app().getBoardGUI().get())->getOptions().take_moveOut;
    }
  else if (cmp(key,itemDisplay_showGameOverMessageBox))
    {
      ApplicationGUI_Gtk* gui = dynamic_cast<ApplicationGUI_Gtk*>(MainApp::app().getApplicationGUI().get());
      return gui->getGameOverDialogFlag();
    }

  if (m_delegate != NULL)
    {
      return m_delegate->read_bool(settings,key);
    }
}

float ConfigManager_AppGtk::read_float(GSettings* settings, const char* key)
{
  if (cmp(key,itemDisplayGtk_animationSpeed))
    {
      return dynamic_cast<BoardGUI_GtkCairo*>(MainApp::app().getBoardGUI().get())->getOptions().animationSpeed;
    }
  else if (cmp(key,itemDisplayGtk_takePieceDelay))
    {
      return dynamic_cast<BoardGUI_GtkCairo*>(MainApp::app().getBoardGUI().get())->getOptions().take_delay;
    }

  if (m_delegate != NULL)
    {
      return m_delegate->read_float(settings,key);
    }
}

std::string ConfigManager_AppGtk::read_string(GSettings* settings, const char* key)
{
  if (m_delegate != NULL)
    {
      return m_delegate->read_string(settings,key);
    }
}
