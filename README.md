Customized Quake2 engine with Heretic2 code parts.

Heretic 2 SDK code based on [Ht2Toolkit](https://www.quaddicted.com/files/idgames2/planetquake/hereticii/files/Ht2Toolkit_v1.06.exe)

Updated code based on [Heretic 2 Reconstruction Project](https://github.com/jmarshall23/Heretic2/tree/Engine-DLL)

Tested with [Heretic 2 Loki](https://archive.org/details/heretic-2-linux) release.

# cleanup code
```shell
sed -i 's/[[:blank:]]*$//' */*.{c,h}
```

Comments:

It's on really initial steps, it could be complied ;-), it runs mostly without
crashes. That's all what is good.

Drawbacks:
* code use diffent angles values to quake2
* huge amount of possibly dead code
* broken jumps
* message id to text procesed in client instead game code itself
* broken guns select
* no menu implementations
* no books implementations
* code is little bit mess
* game, client_effects hard linked
* only gl1 has full support render
* soft,gl3,vk render has incorrect angles and no particles

Minimal file set from Loki release:
```
7f705e54da770186abd84f1a904faa28 base/default.cfg
75cd6a0e878f24bb934e640b4d4dd18c base/htic2-0.pak
d771f54be69f8fb9054c5a84a2b61dff base/players/male/Corvus.m8
4633cc6801f36898f0d74ab136ce013e base/players/male/tris.fm
b90b7cc08ec002297a320e06bae6a5eb base/video/bumper.mpg
b92d295e769b2c5a94ee2dbf1bbe12e4 base/video/intro.mpg
9c201601fdd82754068d02a5474a4e60 base/video/outro.mpg
```

Code checked with:
```
map ssdocks
```

Goals:
* Implement minimal set required for single player,
* Multiplayer game or same protocol is not in priority,
* Raven code should be placed only in src/game or separate repository,
* All other code should be GPL or public domain,
* Minimal set of hacks over quake 2 engine.

======

# Yamagi Quake II

Yamagi Quake II is an enhanced client for id Software's Quake
II with focus on offline and coop gameplay. Both the gameplay and the graphics
are unchanged, but many bugs in the last official release were fixed and some
nice to have features like widescreen support and a modern OpenGL 3.2 renderer
were added. Unlike most other Quake II source ports Yamagi Quake II is fully 64-bit
clean. It works perfectly on modern processors and operating systems. Yamagi
Quake II runs on nearly all common platforms; including FreeBSD, Linux, NetBSD,
OpenBSD, Windows and macOS (experimental).

This code is built upon Icculus Quake II, which itself is based on Quake II
3.21. Yamagi Quake II is released under the terms of the GPL version 2. See the
LICENSE file for further information.

## Documentation

Before asking any question, read through the documentation! The current
version can be found here: [doc/010_index.md](doc/010_index.md)

## Releases

The official releases (including Windows binaries) can be found at our
homepage: https://www.yamagi.org/quake2
**Unsupported** preview builds for Windows can be found at
https://deponie.yamagi.org/quake2/misc/
