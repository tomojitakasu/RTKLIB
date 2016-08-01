TEMPLATE = subdirs

SUBDIRS= src \
	 app

app.depends = src
