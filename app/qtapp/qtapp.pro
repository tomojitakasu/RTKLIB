TEMPLATE = subdirs

SUBDIRS= ../../src \
         rtknavi_qt \
         rtkget_qt \
         rtkplot_qt \
         rtkpost_qt \
         rtklaunch_qt \
         srctblbrows_qt \
         strsvr_qt \
         rtkconv_qt



app.depends = ../../src

IERS_MODEL {
    SUBDIRS += lib
    app.depend = lib
}
