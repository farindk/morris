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

#include "util.hh"
#include "mainapp.hh"
#include "gtk_appgui_interface.hh"
#include "gtk_prefDisplay.hh"
#include <gtk/gtk.h>

static const int PADDING=8;


bool preferencesDialog_Display()
{
  ApplicationGUI_GtkIF* appgui = dynamic_cast<ApplicationGUI_GtkIF*>(MainApp::app().getApplicationGUI().get());
  GtkWidget* window = appgui->getMainWindow();
  configmanager_ptr config = MainApp::app().getConfigManager();

  GtkWidget *pref_dialog, *vbox;

  pref_dialog = gtk_dialog_new_with_buttons(_("Display Preferences"),
					    GTK_WINDOW(window),
					    GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
					    GTK_STOCK_OK,     GTK_RESPONSE_ACCEPT,
					    GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
					    NULL);
  gtk_window_set_resizable(GTK_WINDOW(pref_dialog), FALSE);

  // board selection

  vbox = gtk_vbox_new(FALSE,0);

  GtkWidget* check_animComp = gtk_check_button_new_with_label(_("animate computer moves"));
  GtkWidget* check_animSet  = gtk_check_button_new_with_label(_("animate setting of pieces"));
  GtkWidget* check_animTake = gtk_check_button_new_with_label(_("animate takes"));
  GtkWidget* check_colCross = gtk_check_button_new_with_label(_("colored crossings while dragging"));
  GtkWidget* check_gameOverReq = gtk_check_button_new_with_label(_("show game-over message box"));
  GtkWidget* check_showCoords  = gtk_check_button_new_with_label(_("show coordinates"));

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_gameOverReq),
			       config->read_bool(ConfigManager::itemDisplay_showGameOverMessageBox));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_animComp),
			       config->read_bool(ConfigManager::itemDisplayGtk_animateComputerMoves));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_animSet),
			       config->read_bool(ConfigManager::itemDisplayGtk_animateSettingOfPieces));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_animTake),
			       config->read_bool(ConfigManager::itemDisplayGtk_animateTakes));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_showCoords),
			       config->read_bool(ConfigManager::itemDisplayGtk_showCoordinates));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_colCross),
			       config->read_bool(ConfigManager::itemDisplayGtk_coloredCrossingsWhileDragging));

  gtk_box_pack_start(GTK_BOX(vbox), check_gameOverReq, FALSE, TRUE, PADDING/2);
  gtk_box_pack_start(GTK_BOX(vbox), check_showCoords,  FALSE, TRUE, PADDING/2);
  gtk_box_pack_start(GTK_BOX(vbox), check_colCross,    FALSE, TRUE, PADDING/2);
  gtk_box_pack_start(GTK_BOX(vbox), check_animComp,    FALSE, TRUE, PADDING/2);
  gtk_box_pack_start(GTK_BOX(vbox), check_animSet,     FALSE, TRUE, PADDING/2);
  gtk_box_pack_start(GTK_BOX(vbox), check_animTake,    FALSE, TRUE, PADDING/2);

  GtkWidget* hbox  = gtk_hbox_new(FALSE,0);
  GtkWidget* label = gtk_label_new(_("animation speed"));


  float animSpeed = config->read_float(ConfigManager::itemDisplayGtk_animationSpeed);
  animSpeed = 100-animSpeed/20;
  GtkObject* adj_animSpeed = gtk_adjustment_new(animSpeed,
						0.0,  // lower
						100.0, // upper
						1.0,  // step_increment
						10.0, // page_increment
						0.0); // page_size

  GtkWidget* scale_animSpeed = gtk_hscale_new( GTK_ADJUSTMENT(adj_animSpeed) );

  gtk_box_pack_start(GTK_BOX(hbox), label,           FALSE, TRUE, PADDING/2);
  gtk_box_pack_start(GTK_BOX(hbox), scale_animSpeed, TRUE, TRUE, PADDING);
  gtk_box_pack_start(GTK_BOX(vbox), hbox,    FALSE, TRUE, PADDING/2);


  hbox  = gtk_hbox_new(FALSE,0);
  label = gtk_label_new(_("take piece delay"));

  float takeDelay = config->read_float(ConfigManager::itemDisplayGtk_takePieceDelay);
  takeDelay = takeDelay/10;
  GtkObject* adj_takeDelay = gtk_adjustment_new(takeDelay, // initial value
						0.0,  // lower
						100.0, // upper
						1.0,  // step_increment
						10.0, // page_increment
						0.0); // page_size

  GtkWidget* scale_takeDelay = gtk_hscale_new(GTK_ADJUSTMENT(adj_takeDelay));

  gtk_box_pack_start(GTK_BOX(hbox), label,           FALSE, TRUE, PADDING/2);
  gtk_box_pack_start(GTK_BOX(hbox), scale_takeDelay, TRUE, TRUE, PADDING);
  gtk_box_pack_start(GTK_BOX(vbox), hbox,    FALSE, TRUE, PADDING/2);

  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pref_dialog)->vbox), vbox, FALSE, TRUE, PADDING);

  gtk_scale_set_value_pos(GTK_SCALE(scale_animSpeed), GTK_POS_RIGHT);
  gtk_scale_set_value_pos(GTK_SCALE(scale_takeDelay), GTK_POS_RIGHT);

  gtk_widget_show_all(pref_dialog);

  // can run it
  gint button = gtk_dialog_run(GTK_DIALOG(pref_dialog));
  bool newValues = false;
  switch (button)
    {
    case GTK_RESPONSE_ACCEPT:
      {
	config->store(ConfigManager::itemDisplay_showGameOverMessageBox,
		      bool(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_gameOverReq))));
	config->store(ConfigManager::itemDisplayGtk_coloredCrossingsWhileDragging,
		      bool(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_colCross))));
	config->store(ConfigManager::itemDisplayGtk_animateComputerMoves,
		      bool(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_animComp))));
	config->store(ConfigManager::itemDisplayGtk_animateSettingOfPieces,
		      bool(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_animSet))));
	config->store(ConfigManager::itemDisplayGtk_animateTakes,
		      bool(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_animTake))));
	config->store(ConfigManager::itemDisplayGtk_showCoordinates,
		      bool(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_showCoords))));

	config->store(ConfigManager::itemDisplayGtk_animationSpeed,
		      float( 20*(100-gtk_range_get_value(GTK_RANGE(scale_animSpeed))) ));
	config->store(ConfigManager::itemDisplayGtk_takePieceDelay,
		      float( 10*     gtk_range_get_value(GTK_RANGE(scale_takeDelay)) ));

	newValues = true;
      }
      break;

    default:
      /* do no changes */
      break;
    }

  gtk_widget_destroy (pref_dialog);

  return newValues;
}

