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

#ifndef MAINAPP_HH
#define MAINAPP_HH

#include "ttable.hh"
#include "control.hh"
#include "player.hh"
#include "boardgui.hh"
#include "appgui.hh"
#include "learn.hh"
#include "configmgr.hh"


/* The general state of the main application. This is not at the granularity of
   the state in GameControl, but rather extends it with an higher-level application state.
 */
struct AppState
{
  AppState();

  bool gamePaused;        // whether the game is paused and has to be explicitly continued
  bool networkConnected;  // CURRENTLY UNUSED
};


/* The MainApp is a singleton class that is the central entity of the application.
   This is where all the modules are instantiated and connected together.
 */
class MainApp
{
public:
  // --- initialization ---

  static void     createMainAppSingleton(); // this initialized the global MainApp
  static MainApp& app();                    // the main application singleton (without GUI)

  void init(); // call after the GUI has been initialized
  void setThreadTunnel(ThreadTunnel& tt);

  /* Replace the rules. If significant changes are indicated, the game is also restarted. */
  void setRules(rulespec_ptr, bool significantChange);
  void setPlayer(Player, player_ptr);

  // --- application and game control ---

  void resetGame();
  void undo();
  void redo();

  void setPause(bool flag=true);
  void continueGame(); // resume from pause

  GameControl& getControl() { return control; }

  /* Start the next move. Note that the next move is not started immediately, but
     simply marked down that it should be started. It will be started the next time
     the program gets idle. */
  void startNextMove();

  /* Graphically visualize the move and then forward it to the GameControl via the doMove() below.
     This method is usually used for non-interactive players.
  */
  void visualizeMove(const Move& m, int gameID); // called from the player algorithm

  /* Immediately perform the move without any graphical feedback. This the usual way
     interactive moves are executed.
  */
  int doMove(Move m); // returns the number of takes that still have to be carried out (see GameControl)

  // --- options ---

  void registerConfigManager(configmanager_ptr p) { m_configManager=p; }
  configmanager_ptr getConfigManager() { return m_configManager; }

  struct Options
  {
    Options();

    bool alwaysPauseOnAIPlayer;
  };

  Options options;

  // --- AI players ---

  player_ptr& getAIPlayer(int p) { return player_computer[p]; }
  ttable_ptr& getTTable(int p)   { return ttable[p]; }
  bool        getShareTTables()  { return share_TT; }
  void        setShareTTables(bool share);

  // --- hint AI ---

  void computeHint();

  // --- network ---

  // ... TODO ...

  // --- GUI ---

  void       setApplicationGUI(appgui_ptr gui) { gui_application=gui; }
  appgui_ptr getApplicationGUI() const { return gui_application; }

  boardgui_ptr getBoardGUI() const { return gui_application->getBoardGUI(); }

  void setThinkingInfo(const std::string&);


  // --- thread-tunnel ---

  // ThreadTunnel* getThreadTunnel() { return threadTunnel; }  // currently unused


  // --- signals ---

  boost::signal<void (bool)>& getSignal_pauseChanged() { return m_signal_pauseChanged; }

private:
  MainApp();

  // game control

  AppState    appState;
  GameControl control;

  // players

  player_ptr     player_computer[2]; // A and B
  bool           share_TT;
  ttable_ptr     ttable[2];
  experience_ptr experience;

  // hint

  player_ptr hint_computer;
  int        hintID, hint_gameID;
  ttable_ptr hint_ttable;

  // configuration

  configmanager_ptr m_configManager;

  // GUI

  appgui_ptr    gui_application;
  ThreadTunnel* threadTunnel;

  void nextMove_butPauseIfAIPlayer();

  // callbacks
  void setStatusbarText(); // check game state and set statusbar text accordingly
  void setStatusbarText_withThinking(const std::string& suffix); // check game state and set statusbar text accordingly
  void startMove(player_ptr);
  void endMove(player_ptr);

  boost::signal<void (bool)> m_signal_pauseChanged;
};

#endif
