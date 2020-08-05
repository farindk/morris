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
#include <gtk/gtk.h>

static const int PADDING=8;


bool confirmationRequester(GtkWindow* parent, const char* primaryText, const char* secondaryText)
{
  GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(parent),
					     GTK_DIALOG_DESTROY_WITH_PARENT,
					     GTK_MESSAGE_QUESTION,
					     GTK_BUTTONS_YES_NO,
					     primaryText);
  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), secondaryText);
  gint button = gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy (dialog);

  switch (button)
    {
    case GTK_RESPONSE_YES: return true;
    case GTK_RESPONSE_NO:  return false;
    default: assert(0);
    }
}


bool preferencesDialog_ruleSpec()
{
  ApplicationGUI_GtkIF* appgui = dynamic_cast<ApplicationGUI_GtkIF*>(MainApp::app().getApplicationGUI().get());
  GtkWidget* window = appgui->getMainWindow();

  struct BoardOption
  {
    const char* name;
    BoardSpec::BoardPreset boardSpec;
    GtkWidget*  radioButton;
  } boardSpec_radio[] =
      {
	{ N_("Nine Men's Morris"), BoardSpec::Board_Standard9MM },
	{ N_("Moebius board"),     BoardSpec::Board_Moebius },
	{ N_("Morabaraba"),        BoardSpec::Board_Morabaraba },
	{ N_("Windmill"),          BoardSpec::Board_Windmill },
	{ N_("Sunmill"),           BoardSpec::Board_Sunmill },
	{ N_("Triangle"),          BoardSpec::Board_Polygon3 },
	{ N_("Pentagon"),          BoardSpec::Board_Polygon5 },
	{ N_("Hexagon"),           BoardSpec::Board_Polygon6 },
	{ N_("Seven Men's Morris"),BoardSpec::Board_7MM },
	{ N_("Six Men's Morris"),  BoardSpec::Board_6MM },
	{ N_("Small square"),      BoardSpec::Board_SmallSq },
	{ N_("Small square with diagonals"), BoardSpec::Board_SmallSqWithDiag },
	{ N_("Small triangle"),    BoardSpec::Board_SmallTri },
	{ NULL }
      };

  GtkWidget *pref_dialog, *vbox1, *vbox2;
  GtkWidget *frame1, *frame2;

  rulespec_ptr oldRuleSpec = MainApp::app().getControl().getRuleSpec();

  pref_dialog = gtk_dialog_new_with_buttons(_("Rule Preferences"),
					    GTK_WINDOW(window),
					    GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
					    GTK_STOCK_OK,     GTK_RESPONSE_ACCEPT,
					    GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
					    NULL);
  gtk_window_set_resizable(GTK_WINDOW(pref_dialog), FALSE);

  // board selection

  frame1   = gtk_frame_new(_("Board Type"));
  vbox1    = gtk_vbox_new(FALSE,0);

  GtkWidget* radioButton=NULL;
  for (int i=0; boardSpec_radio[i].name != NULL; i++)
    {
      radioButton = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radioButton),
								gettext(boardSpec_radio[i].name));
      gtk_box_pack_start(GTK_BOX(vbox1), radioButton,  FALSE, TRUE, PADDING*0/2);

      if (oldRuleSpec->boardSpec->getBoardPresetID() == boardSpec_radio[i].boardSpec)
	{ gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radioButton), TRUE); }

      boardSpec_radio[i].radioButton = radioButton;
    }

 
  //
  // setup the rule variations
  //
  frame2         = gtk_frame_new(_("Rule Variations"));
  vbox2          = gtk_vbox_new(FALSE,0);

  GtkWidget* check_lasker   = gtk_check_button_new_with_label(_("Lasker variant"));
  GtkWidget* check_mayjump  = gtk_check_button_new_with_label(_("May jump with three pieces"));
  GtkWidget* check_takemills= gtk_check_button_new_with_label(_("May take from mills always"));
  GtkWidget* check_takemulti= gtk_check_button_new_with_label(_("May take multiple pieces"));

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_lasker),   oldRuleSpec->laskerVariant);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_mayjump),  oldRuleSpec->mayJump);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_takemills),oldRuleSpec->mayTakeFromMillsAlways);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_takemulti),oldRuleSpec->mayTakeMultiple);


  GtkWidget* hbox_pieces = gtk_hbox_new(FALSE,0);
  GtkWidget* label_pieces = gtk_label_new(_("Number of pieces"));

  GtkObject* adj_pieces = gtk_adjustment_new(oldRuleSpec->nPieces, // initial value
					     3.0,  // lower
					     20.0, // upper
					     1.0,  // step_increment
					     1.0,  // page_increment
					     0.0); // page_size

  GtkWidget* spin_pieces = gtk_spin_button_new(GTK_ADJUSTMENT(adj_pieces),0.0,0);

  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(spin_pieces),TRUE);
  gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(spin_pieces),TRUE);

  gtk_box_pack_start(GTK_BOX(hbox_pieces), label_pieces,FALSE, TRUE, PADDING*0);
  gtk_box_pack_start(GTK_BOX(hbox_pieces), spin_pieces, FALSE, TRUE, PADDING);



  GtkWidget* hbox_tie = gtk_hbox_new(FALSE,0);
  GtkWidget* label_tie = gtk_label_new(_("Declare tie after how many repeats"));

  GtkObject* adj_tie = gtk_adjustment_new(oldRuleSpec->tieAfterNRepeats, // initial value
					  0.0,  // lower
					  5.0, // upper
					  1.0,  // step_increment
					  1.0,  // page_increment
					  0.0); // page_size

  GtkWidget* spin_tie = gtk_spin_button_new(GTK_ADJUSTMENT(adj_tie),0.0,0);

  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(spin_tie),TRUE);
  gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(spin_tie),TRUE);

  gtk_box_pack_start(GTK_BOX(hbox_tie), label_tie,FALSE, TRUE, PADDING*0);
  gtk_box_pack_start(GTK_BOX(hbox_tie), spin_tie, FALSE, TRUE, PADDING);

  gtk_box_pack_start(GTK_BOX(vbox2), hbox_pieces, FALSE, TRUE, PADDING/2);
  gtk_box_pack_start(GTK_BOX(vbox2), check_lasker,    FALSE, TRUE, PADDING/2);
  gtk_box_pack_start(GTK_BOX(vbox2), check_mayjump,   FALSE, TRUE, PADDING/2);
  gtk_box_pack_start(GTK_BOX(vbox2), check_takemills, FALSE, TRUE, PADDING/2);
  gtk_box_pack_start(GTK_BOX(vbox2), check_takemulti, FALSE, TRUE, PADDING/2);
  gtk_box_pack_start(GTK_BOX(vbox2), hbox_tie, FALSE, TRUE, PADDING/2);

  gtk_container_add(GTK_CONTAINER(frame1),vbox1);
  gtk_container_add(GTK_CONTAINER(frame2),vbox2);

  GtkWidget* hbox = gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox), frame1, FALSE, TRUE, PADDING);
  gtk_box_pack_start(GTK_BOX(hbox), frame2, FALSE, TRUE, PADDING);

  gtk_container_set_border_width(GTK_CONTAINER(vbox1), PADDING/2);
  gtk_container_set_border_width(GTK_CONTAINER(vbox2), PADDING/2);

  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(pref_dialog)->vbox), hbox, FALSE, TRUE, PADDING);


  gtk_widget_show_all(pref_dialog);

  // can run it
  bool replaceRules=false;
  bool runDialog=true;
  while (runDialog)
    {
      gint button = gtk_dialog_run(GTK_DIALOG(pref_dialog));
      switch (button)
	{
	case GTK_RESPONSE_ACCEPT:
	  {
	    rulespec_ptr newRuleSpec = rulespec_ptr(new RuleSpec);
	    *newRuleSpec = *oldRuleSpec;
	    
	    newRuleSpec->laskerVariant   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_lasker));
	    newRuleSpec->mayJump         = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_mayjump));
	    newRuleSpec->mayTakeMultiple = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_takemulti));
	    newRuleSpec->mayTakeFromMillsAlways = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_takemills));
	    newRuleSpec->tieAfterNRepeats = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_tie));
	    newRuleSpec->nPieces          = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_pieces));

	    for (int i=0; boardSpec_radio[i].name != NULL; i++)
	      {
		GtkWidget* button = boardSpec_radio[i].radioButton;

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
		  { newRuleSpec->boardSpec = BoardSpec::boardFactory( boardSpec_radio[i].boardSpec ); }
	      }

	    bool significantChange=false;
	    if (oldRuleSpec->nPieces != newRuleSpec->nPieces) significantChange=true;
	    if (oldRuleSpec->boardSpec->getBoardPresetID() != newRuleSpec->boardSpec->getBoardPresetID())
	      { significantChange=true; }

	    bool sure = true;
	    if (significantChange)
	      {
		sure = confirmationRequester(GTK_WINDOW(pref_dialog),
					     _("Your changes require to end the currently running game."),
					     _("Do you want to restart with the changed rules?"));
	      }

	    if (sure)
	      {
		MainApp::app().setRules(newRuleSpec, significantChange);
		replaceRules = true;
		runDialog=false;
	      }
	  }
      
	  break;

	default:
	  /* do no changes */
	  runDialog=false;
	  break;
	}
    }

  gtk_widget_destroy (pref_dialog);

  return replaceRules;
}

