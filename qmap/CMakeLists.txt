cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(qmap)

set(SOURCES
	3d.h
	bspfile.h
	bsp.h
	cmdlib.h
	fix.h
	mode.h
	poly.h
	render.h
	scr.h
	s.h
	surface.h
	tmap3d.h
	tm.h
	text.h
	clock.h
	cam.h

	main.c
	clippoly.c
	render.c
	3d.c
	mode.c
	bsp.c
	readbsp.c
	tmap3d.c
	poly.c
	tm_c.c
	surface.c
	clock.c
	cam.c
	iso_font.c)

add_executable(qmap ${SOURCES})

if (NOT EMSCRIPTEN)
	find_package(SDL2 REQUIRED)
	target_link_libraries(qmap
		$<$<PLATFORM_ID:Windows>:SDL2::SDL2main>
		SDL2::SDL2
		$<$<C_COMPILER_ID:Clang,GNU>:m>)
	target_compile_options(qmap PRIVATE
		$<$<C_COMPILER_ID:AppleClang,Clang,GNU>:-Wall -Wextra>)
else()
	set(FILES
		"--embed-file=palette.lmp"
		"--embed-file=colormap.lmp"
		"--embed-file=e1m1.bsp")

	target_compile_options(qmap PRIVATE -O2 -sUSE_SDL=2)
	target_link_options(qmap PRIVATE -sUSE_SDL=2 ${FILES})
endif()
