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


const char* ConfigManager::itemComputer_maxTime[2] = { "ai/computerA/max_time","ai/computerB/max_time" };
const char* ConfigManager::itemComputer_maxDepth[2] = { "ai/computerA/max_depth", "ai/computerB/max_depth" };
const char* ConfigManager::itemComputer_weightMaterial[2] = { "ai/computerA/weights/material","ai/computerB/weights/material" };
const char* ConfigManager::itemComputer_weightFreedom[2] = { "ai/computerA/weights/freedom","ai/computerB/weights/freedom" };
const char* ConfigManager::itemComputer_weightMills[2] = { "ai/computerA/weights/mills","ai/computerB/weights/mills" };
const char* ConfigManager::itemComputer_weightExperience[2] = { "ai/computerA/weights/experience","ai/computerB/weights/experience" };

const char* ConfigManager::itemComputers_shareTTables = "ai/share_transposition_tables";


const char* ConfigManager::itemDisplay_showGameOverMessageBox = "display/show_game_over_message_box";
const char* ConfigManager::itemDisplayGtk_showCoordinates = "display/show_board_coordinates";
const char* ConfigManager::itemDisplayGtk_coloredCrossingsWhileDragging = "display/colored_crossings_while_dragging";
const char* ConfigManager::itemDisplayGtk_animateComputerMoves = "display/animate_computer_moves";
const char* ConfigManager::itemDisplayGtk_animateSettingOfPieces = "display/animate_setting_of_pieces";
const char* ConfigManager::itemDisplayGtk_animateTakes   = "display/animate_takes";
const char* ConfigManager::itemDisplayGtk_animationSpeed = "display/animation_speed";
const char* ConfigManager::itemDisplayGtk_takePieceDelay = "display/take_piece_delay";


const char* ConfigManager::itemPref_pauseOnAIPlayer = "pause_on_ai_player";
const char* ConfigManager::itemPref_showLogOfMoves  = "show_log_of_moves";


bool ConfigManager::cmp(const char* a, const char* b)
{
  if (a==b) return true;

  return strcmp(a,b)==0;
}
