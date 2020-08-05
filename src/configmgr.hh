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

#ifndef CONFIGMGR_HH
#define CONFIGMGR_HH

#include <string>
#include <boost/shared_ptr.hpp>


/* The config manager organizes all the configurable options.
   The architecture is as follows:
   - An application config-manager establishes the connection between
     the config and the application (i.e. it knows how to read options
     out of the application and send changed values to it).
   - Application config-managers can be chained, so that a main manager
     can handle all the generic options, while a platform dependent
     manager can handle the additional specific options.
   - On top of the application config-manager, another config manager
     can handle saving and loading of the configuration (such as
     through gconf). Changed parameters are then forwarded via the
     application config-manager.

   The idea of this architecture is to employ gconf-like configuration
   if it is supported by the system and some other scheme if it is not.
   In the simplest case, the application config-manager can be used
   directly, which results in the default behaviour of not saving the
   configuration, but be otherwise fully functional.
 */
class ConfigManager
{
public:
  virtual ~ConfigManager() { }

  virtual void readInitialValues() { }

  virtual void store(const char* key, int   value) = 0;
  virtual void store(const char* key, bool  value) = 0;
  virtual void store(const char* key, float value) = 0;
  virtual void store(const char* key, const char* value) = 0;

  virtual int   read_int   (const char* key) = 0;
  virtual bool  read_bool  (const char* key) = 0;
  virtual float read_float (const char* key) = 0;
  virtual std::string read_string(const char* key) = 0;


  // constant strings for the configurable items

  static const char* itemComputer_maxTime[2];
  static const char* itemComputer_maxDepth[2];
  static const char* itemComputer_weightMaterial[2];
  static const char* itemComputer_weightFreedom[2];
  static const char* itemComputer_weightMills[2];
  static const char* itemComputer_weightExperience[2];

  static const char* itemComputers_shareTTables;

  static const char* itemDisplay_showGameOverMessageBox;
  static const char* itemDisplayGtk_showCoordinates;
  static const char* itemDisplayGtk_coloredCrossingsWhileDragging;
  static const char* itemDisplayGtk_animateComputerMoves;
  static const char* itemDisplayGtk_animateSettingOfPieces;
  static const char* itemDisplayGtk_animateTakes;
  static const char* itemDisplayGtk_animationSpeed;
  static const char* itemDisplayGtk_takePieceDelay;

  static const char* itemPref_pauseOnAIPlayer;
  static const char* itemPref_showLogOfMoves;

protected:
  // Compare two config-keys. This function is optimized for the comparisons of the keys.
  static bool cmp(const char*, const char*);
};

typedef boost::shared_ptr<ConfigManager> configmanager_ptr;

#endif
