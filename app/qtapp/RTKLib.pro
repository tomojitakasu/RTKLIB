TEMPLATE = subdirs

SUBDIRS= src \
	 app

app.depends = src

IERS_MODEL {
    SUBDIRS += lib
    app.depend = lib
}
