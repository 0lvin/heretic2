all: view-quake iqm-demo

view-quake: mdl2.c ../src/common/header/files.h
	gcc mdl2.c -g -Wall -lGL -lGLU -lglut -o view-quake

iqm-demo: demo.cpp util.h geom.h scale.h iqm.h
	g++ demo.cpp -g -Wall -lGL -lGLU -lglut -o iqm-demo
