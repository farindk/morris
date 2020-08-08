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

#include "configmgr.hh"
#include <string.h>


const char* ConfigManager::itemComputer_maxTime[2] = { "max-time","max-time" };
const char* ConfigManager::itemComputer_maxDepth[2] = { "max-depth", "max-depth" };
const char* ConfigManager::itemComputer_weightMaterial[2] = { "material","material" };
const char* ConfigManager::itemComputer_weightFreedom[2] = { "freedom","freedom" };
const char* ConfigManager::itemComputer_weightMills[2] = { "mills","mills" };
const char* ConfigManager::itemComputer_weightExperience[2] = { "experience","experience" };

const char* ConfigManager::itemComputers_shareTTables = "share-transposition-tables";


const char* ConfigManager::itemDisplay_showGameOverMessageBox = "show-game-over-message-box";
const char* ConfigManager::itemDisplayGtk_showCoordinates = "show-board-coordinates";
const char* ConfigManager::itemDisplayGtk_coloredCrossingsWhileDragging = "colored-crossings-while-dragging";
const char* ConfigManager::itemDisplayGtk_animateComputerMoves = "animate-computer-moves";
const char* ConfigManager::itemDisplayGtk_animateSettingOfPieces = "animate-setting-of-pieces";
const char* ConfigManager::itemDisplayGtk_animateTakes   = "animate-takes";
const char* ConfigManager::itemDisplayGtk_animationSpeed = "animation-speed";
const char* ConfigManager::itemDisplayGtk_takePieceDelay = "take-piece-delay";


const char* ConfigManager::itemPref_pauseOnAIPlayer = "pause-on-ai-player";
const char* ConfigManager::itemPref_showLogOfMoves  = "show-log-of-moves";

ConfigManager::ConfigManager()
{
  settings = g_settings_new("net.nine-mens-morris");
  disp_settings = g_settings_get_child(settings, "display");
  ai_settings = g_settings_get_child(settings, "ai");
  compA_settings = g_settings_get_child(ai_settings, "computer-a");
  compB_settings = g_settings_get_child(ai_settings, "computer-b");
  weightsA_settings = g_settings_get_child(compA_settings, "weights");
  weightsB_settings = g_settings_get_child(compB_settings, "weights");
}

bool ConfigManager::cmp(const char* a, const char* b)
{
  if (a==b) return true;

  return strcmp(a,b)==0;
}
