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
#include "algo_alphabeta.hh"
#include "mainapp.hh"
#include "gtk_appgui_interface.hh"

#include <gtk/gtk.h>

static const int PADDING=8;


static GtkWidget* new_label_left(const char* txt)
{
  GtkWidget* label = gtk_label_new(txt);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  return label;
}


struct aiWidgets
{
  GtkWidget *vbox, *frame;
  GtkWidget *spin_time, *spin_depth;
  GtkWidget *scale_material, *scale_freedom, *scale_mills, *scale_experience;
};


static void enableDualEvalWeights(const aiWidgets* ai, bool enable)
{
  gtk_widget_set_sensitive(GTK_WIDGET(ai->scale_material),   enable);
  gtk_widget_set_sensitive(GTK_WIDGET(ai->scale_freedom),    enable);
  gtk_widget_set_sensitive(GTK_WIDGET(ai->scale_mills),      enable);
  gtk_widget_set_sensitive(GTK_WIDGET(ai->scale_experience), enable);
}


static void cb_aiPref_shareTT(GtkToggleButton *togglebutton, gpointer user_data)
{
  const aiWidgets* aiWidg = (aiWidgets*)(user_data);

  bool share = gtk_toggle_button_get_active(togglebutton);
  //std::cout << (on ? "ON" : "OFF") << "\n";

  enableDualEvalWeights(aiWidg, !share);
}


void preferencesDialog_AI()
{
  ApplicationGUI_GtkIF* appgui = dynamic_cast<ApplicationGUI_GtkIF*>(MainApp::app().getApplicationGUI().get());
  GtkWidget* window = appgui->getMainWindow();
  configmanager_ptr config = MainApp::app().getConfigManager();

  GtkWidget* pref_dialog;

  struct aiWidgets ai[2];

  pref_dialog = gtk_dialog_new_with_buttons(_("AI Preferences"),
					    GTK_WINDOW(window),
					    GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
					    GTK_STOCK_OK,     GTK_RESPONSE_ACCEPT,
					    GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
					    NULL);
  gtk_window_set_resizable(GTK_WINDOW(pref_dialog), FALSE);


  // same widgets for both computer AIs
  for (int c=0;c<2;c++)
    {
      static const char* frameName[2] = { N_("Computer A"), N_("Computer B") };

      ai[c].frame = gtk_frame_new( gettext(frameName[c]) );
      ai[c].vbox  = gtk_vbox_new(FALSE,0);

      ai[c].spin_depth = gtk_spin_button_new_with_range(1.0,  50.0, 1.0);
      ai[c].spin_time  = gtk_spin_button_new_with_range(0.0, 999.0, 1.0);
      gtk_spin_button_set_digits(GTK_SPIN_BUTTON(ai[c].spin_time), 1);

      ai[c].scale_material   = gtk_hscale_new_with_range(0.0, 1.0, 0.1);
      ai[c].scale_freedom    = gtk_hscale_new_with_range(0.0, 1.0, 0.1);
      ai[c].scale_mills      = gtk_hscale_new_with_range(0.0, 1.0, 0.1);
      ai[c].scale_experience = gtk_hscale_new_with_range(0.0, 1.0, 0.1);

      gtk_scale_set_value_pos(GTK_SCALE(ai[c].scale_material  ), GTK_POS_RIGHT);
      gtk_scale_set_value_pos(GTK_SCALE(ai[c].scale_freedom   ), GTK_POS_RIGHT);
      gtk_scale_set_value_pos(GTK_SCALE(ai[c].scale_mills     ), GTK_POS_RIGHT);
      gtk_scale_set_value_pos(GTK_SCALE(ai[c].scale_experience), GTK_POS_RIGHT);

      gtk_widget_set_size_request(ai[c].scale_freedom, 75, -1); // set a minimum width

      GtkWidget* label;
      GtkWidget* hbox;
      GtkWidget* table;

      // ---

      table = gtk_table_new(3,3,FALSE);
      gtk_table_set_row_spacing(GTK_TABLE(table), 1, PADDING);

      label = new_label_left(NULL);
      gtk_label_set_markup(GTK_LABEL(label), _("<b>thinking process</b>"));
      gtk_box_pack_start(GTK_BOX(ai[c].vbox), label,  FALSE, TRUE, PADDING/2);

      gtk_table_attach(GTK_TABLE(table), new_label_left(_("max. time" )), 1,2,0,1, GTK_FILL, GTK_FILL, 0,0);
      gtk_table_attach(GTK_TABLE(table), new_label_left(_("max. depth")), 1,2,1,2, GTK_FILL, GTK_FILL, 0,0);

      gtk_table_attach_defaults(GTK_TABLE(table), ai[c].spin_time,  2,3,0,1);
      gtk_table_attach_defaults(GTK_TABLE(table), ai[c].spin_depth, 2,3,1,2);

      gtk_table_set_col_spacings(GTK_TABLE(table), PADDING);
      //gtk_container_set_border_width(GTK_CONTAINER(table), PADDING);
      gtk_box_pack_start(GTK_BOX(ai[c].vbox), table,  FALSE, TRUE, PADDING/2);

      // ---

      label = new_label_left(NULL);
      gtk_label_set_markup(GTK_LABEL(label), _("<b>evaluation weights</b>"));
      gtk_box_pack_start(GTK_BOX(ai[c].vbox), label,  FALSE, TRUE, PADDING/2);


      table = gtk_table_new(3,4,FALSE);

      gtk_table_attach(GTK_TABLE(table), new_label_left(_("material")),  1,2,0,1, GTK_FILL, GTK_FILL, 0,0);
      gtk_table_attach(GTK_TABLE(table), new_label_left(_("freedom")),   1,2,1,2, GTK_FILL, GTK_FILL, 0,0);
      gtk_table_attach(GTK_TABLE(table), new_label_left(_("mills")),     1,2,2,3, GTK_FILL, GTK_FILL, 0,0);
      gtk_table_attach(GTK_TABLE(table), new_label_left(_("experience")),1,2,3,4, GTK_FILL, GTK_FILL, 0,0);

      gtk_table_attach_defaults(GTK_TABLE(table), ai[c].scale_material,   2,3,0,1);
      gtk_table_attach_defaults(GTK_TABLE(table), ai[c].scale_freedom,    2,3,1,2);
      gtk_table_attach_defaults(GTK_TABLE(table), ai[c].scale_mills,      2,3,2,3);
      gtk_table_attach_defaults(GTK_TABLE(table), ai[c].scale_experience, 2,3,3,4);

      gtk_table_set_col_spacings(GTK_TABLE(table), PADDING);
      //gtk_container_set_border_width(GTK_CONTAINER(table), PADDING);
      gtk_box_pack_start(GTK_BOX(ai[c].vbox), table,  FALSE, TRUE, PADDING/2);

      //gtk_box_pack_start(GTK_BOX(ai[c].vbox), ai[c].scale_material,  FALSE, TRUE, PADDING/2);

      gtk_container_add(GTK_CONTAINER(ai[c].frame), ai[c].vbox);
      gtk_container_set_border_width(GTK_CONTAINER(ai[c].vbox), PADDING/2);

      // set default values

      gtk_spin_button_set_value(GTK_SPIN_BUTTON(ai[c].spin_depth),
				config->read_int(ConfigManager::itemComputer_maxDepth[c]));
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(ai[c].spin_time),
				config->read_int(ConfigManager::itemComputer_maxTime[c])/1000.0);

      gtk_range_set_value(GTK_RANGE(ai[c].scale_material),
			  config->read_float(ConfigManager::itemComputer_weightMaterial[c]));
      gtk_range_set_value(GTK_RANGE(ai[c].scale_freedom),
			  config->read_float(ConfigManager::itemComputer_weightFreedom[c]));
      gtk_range_set_value(GTK_RANGE(ai[c].scale_mills),
			  config->read_float(ConfigManager::itemComputer_weightMills[c]));
      gtk_range_set_value(GTK_RANGE(ai[c].scale_experience),
			  config->read_float(ConfigManager::itemComputer_weightExperience[c]));
    }

  GtkWidget* big_hbox = gtk_hbox_new(FALSE,0);

  gtk_box_pack_start(GTK_BOX(big_hbox), ai[0].frame,  FALSE, TRUE, PADDING/2);
  gtk_box_pack_start(GTK_BOX(big_hbox), ai[1].frame,  FALSE, TRUE, PADDING/2);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pref_dialog)->vbox), big_hbox, FALSE, TRUE, PADDING);


  GtkWidget* check_shareTT= gtk_check_button_new_with_label(_("share transposition table"));

  bool share_TT = config->read_bool(ConfigManager::itemComputers_shareTTables);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_shareTT), share_TT);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pref_dialog)->vbox), check_shareTT, FALSE, TRUE, PADDING);
  gtk_signal_connect(GTK_OBJECT(check_shareTT), "toggled", GTK_SIGNAL_FUNC(cb_aiPref_shareTT), &ai[1]);
  enableDualEvalWeights(&ai[1], !share_TT);

  gtk_widget_show_all(pref_dialog);

  // can run it
  gint button = gtk_dialog_run(GTK_DIALOG(pref_dialog));
  switch (button)
    {
    case GTK_RESPONSE_ACCEPT:
      {
	share_TT=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_shareTT));
	config->store(ConfigManager::itemComputers_shareTTables, share_TT);

	for (int c=0;c<2;c++)
	  {
	    MainApp::app().getTTable(c)->clear(); // TODO: in fact, we only have to clear the table, if we changed a crucial parameter

	    config->store(ConfigManager::itemComputer_maxTime[c],
			  int( gtk_spin_button_get_value(GTK_SPIN_BUTTON(ai[c].spin_time)) * 1000.0 ));
	    config->store(ConfigManager::itemComputer_maxDepth[c],
			  int(gtk_spin_button_get_value(GTK_SPIN_BUTTON(ai[c].spin_depth))));


	    // Read evaluation weights. If a shared transposition-table is used, both evaluation weights will be
	    // taken from computer-A to ensure that they are the same.

	    int c2 = c;
	    if (share_TT) { c2=0; }

	    config->store(ConfigManager::itemComputer_weightMaterial[c],
			  float(gtk_range_get_value(GTK_RANGE(ai[c2].scale_material))));
	    config->store(ConfigManager::itemComputer_weightFreedom[c],
			  float(gtk_range_get_value(GTK_RANGE(ai[c2].scale_freedom))));
	    config->store(ConfigManager::itemComputer_weightMills[c],
			  float(gtk_range_get_value(GTK_RANGE(ai[c2].scale_mills))));
	    config->store(ConfigManager::itemComputer_weightExperience[c],
			  float(gtk_range_get_value(GTK_RANGE(ai[c2].scale_experience))));
	  }
      }
      
      break;

    default:
      /* do no changes */
      break;
    }

  gtk_widget_destroy (pref_dialog);
}
