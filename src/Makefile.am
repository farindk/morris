## Makefile.am for morris/src

bin_PROGRAMS = morris

morris_SOURCES = morris.cc morris.hh board.cc board.hh control.hh control.cc \
  gtkcairo_boardgui.cc gtkcairo_boardgui.hh boardgui.cc boardgui.hh \
  algo_random.hh algo_random.cc ttable.cc ttable.hh learn.hh \
  algo_alphabeta.hh algo_alphabeta.cc threadtunnel.hh \
  player.hh gtk_prefAI.cc gtk_prefRules.cc mainapp.hh mainapp.cc \
  util.hh boardspec.hh rules.hh boardspec.cc rules.cc constants.hh \
  appgui.hh  gtk_appgui.hh gtk_appgui.cc gtk_appgui_interface.hh \
  gtk_menutoolbar.cc gtk_menutoolbar.hh \
  gtk_threadtunnel.hh gtk_threadtunnel.cc \
  gtk_prefDisplay.cc gtk_prefDisplay.hh \
  movelog.hh gtk_movelog.hh gtk_movelog.cc \
  configmgr.hh configmgr.cc app_configmgr.hh app_configmgr.cc \
  appgtk_configmgr.hh appgtk_configmgr.cc \
  gettext.h

# gnome_appgui.hh gnome_appgui.cc
# gnome_menu.hh gnome_menu.cc

if OS_WIN32
win32_ldflags = -mwindows # $(WIN32_LARGE_ADDRESS_AWARE)
endif

morris_LDFLAGS = $(BOOST_SIGNALS2_LDFLAGS) $(win32_ldflags)
morris_LDADD = $(GTK_LIBS)  $(GCONF_LIBS) $(BOOST_SIGNALS2_LIBS) $(LIBINTL)


AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\" \
	$(GTK_CFLAGS)  $(GCONF_CFLAGS) $(BOOST_CPPFLAGS)

if HAVE_GCONF
  morris_LDADD += $(GCONF_LIBS)
  AM_CPPFLAGS += $(GCONF_CFLAGS)
  morris_SOURCES += gconf_configmgr.hh gconf_configmgr.cc
endif

#	-I$(top_srcdir) \
#        -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
#	-DGNOME_ICONDIR=\""$(datadir)/pixmaps"\"
