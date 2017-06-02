TEMPLATE = subdirs

SUBDIRS= src \
         lib \
	 app

app.depends = src lib
