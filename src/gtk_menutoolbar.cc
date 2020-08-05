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

#include "gtk_menutoolbar.hh"
#include "boardgui.hh"
#include "mainapp.hh"


static void quit_callback(gpointer)
{
  MainApp::app().getApplicationGUI()->quitApplication();
}

static void new_game_callback(gpointer)
{
  MainApp::app().resetGame();
}

static void undo_move_callback(gpointer)
{
  MainApp::app().undo();
}

static void redo_move_callback(gpointer)
{
  MainApp::app().redo();
}

static void cb_continue_game(gpointer)
{
  MainApp::app().continueGame();
}

static void cb_forceMove(gpointer)
{
  MainApp::app().getControl().forceMove();
}

static void hint_callback(gpointer)
{
  MainApp::app().computeHint();
}


void cb_gtk_select_player(GtkAction*);
void gtk_swap_players(gpointer spec);
void cb_config_customRules();
void cb_gtk_setBoard(GtkAction*);
extern bool preferencesDialog_ruleSpec();
extern void preferencesDialog_AI();

static void cb_showAboutDialog()
{
  MainApp::app().getApplicationGUI()->showAboutDialog();
}


static void preferencesDialog_Display()
{
  appgui_ptr gui = MainApp::app().getApplicationGUI();

  gui->preferencesDialog_Display();
}

static void cb_pauseOnAI(GtkToggleAction* a)
{
  MainApp::app().getConfigManager()->store(ConfigManager::itemPref_pauseOnAIPlayer,
					   bool(gtk_toggle_action_get_active(a)));
}

static void cb_showMoveLog(GtkToggleAction* a)
{
  MainApp::app().getConfigManager()->store(ConfigManager::itemPref_showLogOfMoves,
					   bool(gtk_toggle_action_get_active(a)));
}


static void cb_animated_moves(GtkWidget* w, gpointer spec)
{
}


static const char* menu_toolbar_spec =
  "<ui>"
  "  <menubar name='menubar'>"
  "    <menu action='GameMenu'>"
  "      <menuitem action='NewGame'/>"
  "      <menuitem action='ContinueGame'/>"
  "      <menuitem action='ForceMove'/>"
  "      <separator/>"
  "      <menuitem action='UndoMove'/>"
  "      <menuitem action='RedoMove'/>"
  "      <menuitem action='Hint'/>"
  "      <separator/>"
  "      <menuitem action='QuitPrg'/>"
  "    </menu>"
  "    <menu action='PropertiesMenu'>"
  "      <menu action='whtPlayer'>"
  "        <menuitem action='whtPlayer-human'/>"
  "        <menuitem action='whtPlayer-compA'/>"
  "        <menuitem action='whtPlayer-compB'/>"
  //"        <menuitem action='whtPlayer-network'/>"
  "      </menu>"
  "      <menu action='blkPlayer'>"
  "        <menuitem action='blkPlayer-human'/>"
  "        <menuitem action='blkPlayer-compA'/>"
  "        <menuitem action='blkPlayer-compB'/>"
  //"        <menuitem action='blkPlayer-network'/>"
  "      </menu>"
  "      <menuitem action='swap'/>"
  "      <menuitem action='confAI'/>"
  "      <separator/>"
  "      <menuitem action='rule-std'/>"
  "      <menuitem action='rule-lasker'/>"
  "      <menuitem action='rule-moebius'/>"
  "      <menuitem action='rule-morabaraba'/>"
  "      <menuitem action='rule-windmill'/>"
  "      <menuitem action='rule-sunmill'/>"
  "      <menuitem action='rule-poly3'/>"
  "      <menuitem action='rule-poly5'/>"
  "      <menuitem action='rule-poly6'/>"
  "      <menuitem action='rule-6mm'/>"
  "      <menuitem action='rule-7mm'/>"
  "      <menuitem action='rule-tapatan'/>"
  "      <menuitem action='rule-achi'/>"
  "      <menuitem action='rule-minitri'/>"
  "      <menuitem action='rule-9holes'/>"
  "      <menuitem action='rule-custom'/>"
  "      <menuitem action='configure-rules'/>"
  "    </menu>"
  "    <menu action='PreferencesMenu'>"
  "      <menuitem action='prefDisplay'/>"
  "      <menuitem action='pauseOnAI'/>"
  "      <menuitem action='moveLog'/>"
  "    </menu>"
  "    <menu action='HelpMenu'>"
  "      <menuitem action='aboutDialog'/>"
  "    </menu>"
  "  </menubar>"
  "</ui>";

static const char* toolbar_spec =
  "<ui>"
  "  <toolbar name='toolbar1'>"
  "    <toolitem name='ng' action='NewGame'/>"
  "    <toolitem action='ContinueGame'/>"
  "    <toolitem action='ForceMove'/>"
  "    <toolitem action='UndoMove'/>"
  "    <toolitem action='RedoMove'/>"
  "    <separator/>"
  "    <toolitem action='Hint'/>"
  "  </toolbar>"
  "</ui>";


static GtkActionEntry actions[] =
  {
    { "GameMenu",     NULL,                N_("Game"),          NULL,        N_("menu for game-related functions."), NULL },
    { "NewGame",      GTK_STOCK_REFRESH,   N_("New game"),      "<Ctrl>n",   N_("Start a new game."), (GCallback)new_game_callback },
    { "ContinueGame", GTK_STOCK_MEDIA_PLAY,N_("Continue game"), "<Ctrl>c",   N_("Continue a paused game."), (GCallback)cb_continue_game },
    { "ForceMove",    GTK_STOCK_STOP, N_("Force move"), "<Ctrl>f",           N_("Force computer move (immediately stop calculation)."), (GCallback)cb_forceMove },
    { "UndoMove",     GTK_STOCK_UNDO, N_("Undo move"),     "<Ctrl>z",        N_("Undo last move."), (GCallback)undo_move_callback },
    { "RedoMove",     GTK_STOCK_REDO, N_("Redo move"),     "<Ctrl><Shift>z", N_("Redo last move."), (GCallback)redo_move_callback },
    { "Hint",         GTK_STOCK_HELP, N_("Hint"),          "<Ctrl>h",        N_("Give a hint for the next move."), (GCallback)hint_callback },
    { "QuitPrg",      GTK_STOCK_QUIT, NULL, NULL, NULL, (GCallback)quit_callback },

    { "PropertiesMenu", NULL, N_("Options"), NULL, NULL, NULL },
    { "whtPlayer",    NULL, N_("White player"), 0,0,0 },
    { "blkPlayer",    NULL, N_("Black player"), 0,0,0 },
    { "swap",       NULL, N_("Swap players"), "<Alt>s",0,(GCallback)gtk_swap_players },
    { "confAI",     NULL, N_("Configure AI ..."), 0,0,(GCallback)preferencesDialog_AI },
    { "prefDisplay",NULL, N_("Configure display ..."), 0,0,(GCallback)preferencesDialog_Display },
    { "configure-rules",NULL, N_("Configure custom rules ..."), 0,0,(GCallback)cb_config_customRules },
    { "HelpMenu",    NULL, N_("Help"), 0,0,0 },
    { "aboutDialog", GTK_STOCK_ABOUT, N_("About Morris..."), 0,0,(GCallback)cb_showAboutDialog },
    { "PreferencesMenu", NULL, N_("Preferences"), NULL, NULL, NULL }
  };

static GtkRadioActionEntry whtPlayerRadio[] =
  {
    { "whtPlayer-human",   NULL, N_("Human"), 0,0,1+256 },
    { "whtPlayer-compA",   NULL, N_("Computer A"), 0,0,2+256 },
    { "whtPlayer-compB",   NULL, N_("Computer B"), 0,0,4+256 },
    { "whtPlayer-network", NULL, N_("Network"), 0,0,8+256 }
  };

static GtkRadioActionEntry blkPlayerRadio[] =
  {
    { "blkPlayer-human",   NULL, N_("Human"), 0,0,1 },
    { "blkPlayer-compA",   NULL, N_("Computer A"), 0,0,2 },
    { "blkPlayer-compB",   NULL, N_("Computer B"), 0,0,4 },
    { "blkPlayer-network", NULL, N_("Network"), 0,0,8 },
  };

static GtkRadioActionEntry rulesRadio[] =
  {
    { "rule-std",       NULL, N_("Standard rules"),    0,0,RuleSpec::Preset_Standard },
    { "rule-lasker",    NULL, N_("Lasker variant"),    0,0,RuleSpec::Preset_Lasker },
    { "rule-moebius",   NULL, N_("Moebius board"),     0,0,RuleSpec::Preset_Moebius },
    { "rule-morabaraba",NULL, N_("Morabaraba"),        0,0,RuleSpec::Preset_Morabaraba },
    { "rule-windmill",  NULL, N_("Windmill"),          0,0,RuleSpec::Preset_Windmill },
    { "rule-sunmill",   NULL, N_("Sunmill"),           0,0,RuleSpec::Preset_Sunmill },
    { "rule-6mm",       NULL, N_("Six Men's Morris"),  0,0,RuleSpec::Preset_6MM },
    { "rule-7mm",       NULL, N_("Seven Men's Morris"),0,0,RuleSpec::Preset_7MM },
    { "rule-poly3",     NULL, N_("Triangle board"),    0,0,RuleSpec::Preset_Polygon3 },
    { "rule-poly5",     NULL, N_("Pentagon board"),    0,0,RuleSpec::Preset_Polygon5 },
    { "rule-poly6",     NULL, N_("Hexagon board"),     0,0,RuleSpec::Preset_Polygon6 },
    { "rule-tapatan",   NULL, N_("Tapatan"),           0,0,RuleSpec::Preset_Tapatan },
    { "rule-achi",      NULL, N_("Achi"),              0,0,RuleSpec::Preset_Achi },
    { "rule-minitri",   NULL, N_("Small triangle"),    0,0,RuleSpec::Preset_SmallTri },
    { "rule-9holes",    NULL, N_("Nine Holes"),        0,0,RuleSpec::Preset_NineHoles },
    { "rule-custom",    NULL, N_("Custom rules"),  0,0,1000 }
  };

static GtkToggleActionEntry toggleActions[] =
  {
    { "pauseOnAI", NULL, N_("Pause on AI-player"), 0,N_("Pause before each AI move."),
      (GCallback)cb_pauseOnAI, FALSE /* default */ },
    { "moveLog", NULL, N_("Show log of moves"), 0,0,
      (GCallback)cb_showMoveLog, FALSE /* default */ }
  };


static GtkUIManager* uiManager;
static GtkActionGroup* actiongroup;

void gtk_initMainMenu(GtkWidget* vbox, GtkWidget* window)
{
  actiongroup = gtk_action_group_new("morrisActions");
  gtk_action_group_set_translation_domain(actiongroup, PACKAGE);

  gtk_action_group_add_actions(actiongroup, actions, G_N_ELEMENTS(actions), NULL /*user-data*/);
  gtk_action_group_add_radio_actions(actiongroup, whtPlayerRadio, G_N_ELEMENTS(whtPlayerRadio), 1, (GCallback)cb_gtk_select_player /*cb*/, 0/*data*/);
  gtk_action_group_add_radio_actions(actiongroup, blkPlayerRadio, G_N_ELEMENTS(blkPlayerRadio), 4, (GCallback)cb_gtk_select_player /*cb*/, 0/*data*/);
  gtk_action_group_add_radio_actions(actiongroup, rulesRadio, G_N_ELEMENTS(rulesRadio), 0, (GCallback)cb_gtk_setBoard /*cb*/, 0/*data*/);
  gtk_action_group_add_toggle_actions(actiongroup, toggleActions, G_N_ELEMENTS(toggleActions), 0);

  uiManager = gtk_ui_manager_new();

  gtk_ui_manager_insert_action_group  (uiManager, actiongroup, 0);

  GError* error = NULL;
  gtk_ui_manager_add_ui_from_string(uiManager, menu_toolbar_spec, -1, &error);
  if (error)
    std::cout << "ERROR: " << error->message << "\n";

  gtk_ui_manager_add_ui_from_string(uiManager, toolbar_spec, -1, &error);
  if (error)
    std::cout << "ERROR: " << error->message << "\n";

  gtk_window_add_accel_group (GTK_WINDOW (window), gtk_ui_manager_get_accel_group (uiManager));

  // menu

  GtkWidget* menu = gtk_ui_manager_get_widget(uiManager, "/menubar");
  gtk_box_pack_start(GTK_BOX(vbox), menu, false,true,0);

  // toolbar

  GtkWidget* toolbar = gtk_ui_manager_get_widget(uiManager, "/toolbar1");
  gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH);
  gtk_box_pack_start(GTK_BOX(vbox), toolbar, false,true,0);

  // set defaults

  gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "ContinueGame"), false);
  gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "ForceMove"), false);
  gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "UndoMove"), false);
  gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "RedoMove"), false);
  gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "Hint"), true);

  gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "whtPlayer-compB"), false);
  gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "rule-custom"), false);
}


void cb_gtk_select_player(GtkAction* a)
{
  int idx = gtk_radio_action_get_current_value(GTK_RADIO_ACTION(a));

  Player player = ((idx & 256) ? PL_White : PL_Black);
  idx &= 255;

  if (player==PL_White)
    {
      gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "blkPlayer-compA"), !(idx==2));
      gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "blkPlayer-compB"), !(idx==4));
    }
  else
    {
      gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "whtPlayer-compA"), !(idx==2));
      gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "whtPlayer-compB"), !(idx==4));
    }

  MainApp& app = MainApp::app();
  /**/ if (idx==1) { app.setPlayer(player, player_ptr(new PlayerIF_Human)); }
  else if (idx==2) { app.setPlayer(player, app.getAIPlayer(0)); }
  else if (idx==4) { app.setPlayer(player, app.getAIPlayer(1)); }
  else { assert(0); }
}


void gtk_swap_players(gpointer spec)
{
  GtkAction* whtAction = gtk_action_group_get_action(actiongroup, "whtPlayer-human");
  GtkAction* blkAction = gtk_action_group_get_action(actiongroup, "blkPlayer-human");

  int whtIdx = gtk_radio_action_get_current_value(GTK_RADIO_ACTION(whtAction)) & 0xFF;
  int blkIdx = gtk_radio_action_get_current_value(GTK_RADIO_ACTION(blkAction)) & 0xFF;

  gtk_radio_action_set_current_value(GTK_RADIO_ACTION(whtAction), blkIdx + 256);
  gtk_radio_action_set_current_value(GTK_RADIO_ACTION(blkAction), whtIdx);
}

void cb_gtk_setBoard(GtkAction* a)
{
  int idx = gtk_radio_action_get_current_value(GTK_RADIO_ACTION(a));

  if (idx==1000)
    {
    }
  else
    {
      rulespec_ptr rule;

      rule=RuleSpec::createPresetRule(RuleSpec::RulePreset(idx));
      MainApp::app().setRules(rule,true);

      gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "rule-custom"), false);
    }
}

void cb_config_customRules()
{
  if (preferencesDialog_ruleSpec())
    {
      GtkAction* action = gtk_action_group_get_action(actiongroup, "rule-custom");
      gtk_action_set_sensitive( action, true );

      gtk_radio_action_set_current_value(GTK_RADIO_ACTION(action), 1000);
    }
}

static void cb_menu_pause(bool pause)
{
  gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "ContinueGame"), pause);
}

static void menu_changeState(const GameState& state)
{
  gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "Hint"),  state.state!=GameState::Ended);
}

static void menu_changeBoard()
{
  gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "UndoMove"),  MainApp::app().getControl().getHistoryPos()>0);
  gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "RedoMove"),  MainApp::app().getControl().getHistoryPos()<MainApp::app().getControl().getHistorySize()-1);
  gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "ForceMove"), false);
}

static void menu_startMove(player_ptr p)
{
  bool computerMove = (MainApp::app().getControl().getGameState().state==GameState::Moving &&
		       p->isInteractivePlayer()==false);

  gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "ForceMove"), computerMove);
}

static void menu_endMove(player_ptr p)
{
  gtk_action_set_sensitive( gtk_action_group_get_action(actiongroup, "ForceMove"), false);
}

void menu_closedMoveLog(bool active)
{
  gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(gtk_action_group_get_action(actiongroup, "moveLog")), active);
}

void menu_setPauseOnAI(bool active)
{
  gtk_toggle_action_set_active( GTK_TOGGLE_ACTION(gtk_action_group_get_action(actiongroup, "pauseOnAI")), active);
}

void gtk_connectMenuSignals()
{
  // connect signals
  GameControl& control = MainApp::app().getControl();

  control.getSignal_changeState().connect(menu_changeState);
  control.getSignal_changeBoard().connect(menu_changeBoard);
  control.getSignal_startMove()  .connect(menu_startMove);
  control.getSignal_endMove()    .connect(menu_endMove);

  MainApp::app().getSignal_pauseChanged().connect(cb_menu_pause);
}

