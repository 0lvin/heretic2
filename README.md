# Heretic2 rebased over Yamagi Quake II Remaster

Customized Quake2 engine with Heretic2 code parts.

Heretic 2 SDK code based on [Ht2Toolkit](https://www.quaddicted.com/files/idgames2/planetquake/hereticii/files/Ht2Toolkit_v1.06.exe)

Updated code based on [Heretic 2 Reconstruction Project](https://github.com/jmarshall23/Heretic2/tree/Engine-DLL)

Tested with [Heretic 2 Loki](https://archive.org/details/heretic-2-linux) release.

### cleanup code
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
* no menu implementations
* no books implementations
* code is little bit mess
* only gl1 has full support render
* soft,gl3,vk render has incorrect angles and no particles

To check:
* color type/paticles,
* ANGLE_1 usage in client code,
* different move code and noclip
* Incorrect 16 bit frame fkag unset,
* MAX_MODELS/MAX_SOUND define hardcode,
* no action on button press.

Minimal file set from Loki release:
```
7f705e54da770186abd84f1a904faa28 baseq2/default.cfg
75cd6a0e878f24bb934e640b4d4dd18c baseq2/htic2-0.pak
d771f54be69f8fb9054c5a84a2b61dff baseq2/players/male/Corvus.m8
4633cc6801f36898f0d74ab136ce013e baseq2/players/male/tris.fm
b90b7cc08ec002297a320e06bae6a5eb baseq2/video/bumper.mpg
b92d295e769b2c5a94ee2dbf1bbe12e4 baseq2/video/intro.mpg
9c201601fdd82754068d02a5474a4e60 baseq2/video/outro.mpg
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

# Yamagi Quake II Remaster

This is an experimental fork of Yamagi Quake II with ongoing work to add
support for Quake II Enhanced aka Q2 Remaster(ed). This enhanced version
has a lot non trivial changes, adding support isn't easy and takes time.
Feel free to try this code but you mileage may vary.

Have a look at the yquake2 repository for the "normal" Yamagi Quake II:
https://github.com/yquake2/yquake2

Alpha windows 64 bit [binaries](https://github.com/yquake2/yquake2remaster/releases).

State:
 * GL1/GL3/GLES3/VK:
   * base1: no known issies
   * base2: no known issies
   * q64/outpost: no known issies
   * mguhub: loaded, sometimes broken logic for surface fall in next maps
 * GL4:
   * base1: unchecked
   * base2: unchecked
   * q64/outpost: unchecked
   * mguhub: unchecked
 * SOFT:
   * base1: broken wall light
   * base2: broken wall light
   * q64/outpost: no known issies
   * mguhub: broken wall light, sometimes broken logic for surface fall in next maps

Monsters:
  * incorrect dead animation for Arachnid,
  * broken fire effect for Guardian.

Models support:

| Format | Original Game   | Frame vertex | Meshes | Comments                                |
| ------ | --------------- | ------------ | ------ | --------------------------------------- |
| mdl    | Quake 1         | 8 bit        | Single | Unsupported grouped textures            |
| md2    | Quake 2         | 8 bit        | Single |                                         |
| md2    | Anachronox      | 8/10/16 bit  | Single | No tagged surfaces, unchecked with game |
| mdx    | Kingpin         | 8 bit        | Many   | No sfx support, unchecked with game     |
| fm     | Heretic 2       | 8 bit        | Many   |                                         |
| dkm    | Daikatana DKM1  | 8 bit        | Many   | Unchecked with game                     |
| dkm    | Daikatana DKM2  | 10 bit       | Many   | Unchecked with game                     |
| md3    | Quake 3         | 16 bit       | Many   | No tags support                         |
| md5    | Doom 3/Quake 4  | float        | Many   | Requires md2 for skins                  |

All models support only single texture for all meshes and only up to 255 frames.

Texture support:

| Format | Original Game  | Comments |
| ------ | -------------- | -------- |
| wal    | Quake 2        | 8 bit    |
| wal    | Daikatana      | 8 bit    |
| m8     | Heretic 2      | 8 bit    |
| m32    | Heretic 2      | 24 bit   |
| pcx    | Quake2         | 24 bit   |
| tga    | Quake2         | 24 bit   |
| png    | retexturing    | 24 bit   |
| jpg    | retexturing    | 24 bit   |
| bmp    | Daikatana      | 24 bit   |

Maps support:

| Format | Version | Game                                       |
| ------ | ------- | ------------------------------------------ |
| IBSP   | 39      | Quake 2 / Anachronox / Kingpin / Heretic 2 |
| IBSP   | 41      | Daikatana                                  |
| QBSP   | 39      | Quake 2 ReRelease                          |
| BSPX   | 39      | Quake 2 ReRelease (Extension to IBSP)      |

Note:
 * SiN Gold has IBSP/41 format but has different size of lump and is
   unsupported.
 * Non Quake 2 maps are limmited mostly view only, and could have issues
   with tranparency or some animations flags and properties.

Goals (finished):
  * BSPX DECOUPLEDLM light map support (base1),
  * QBSP map format support (mguhub),
  * Use ffmpeg for load any video,
  * RoQ and Theora cinematic videos support.
  * Cinematic videos support in smk, mpeg, ogv format,
  * Daikatana/Heretic 2 map partial format support,
  * md5 improve load speed,
  * support Anachronox .dat format,
  * add debug progress loading code for maps.


Goals (none of it finished):

  * Single player support,
  * support surface flags for Daikatana, Heretic 2, Anachronox,
  * modified ReRelease game code support with removed KEX only related code.

Bonus goals:
  * Use shared model cache in client code insted reimplemnet in each render,
  * Check load soft colormap as 24bit color,
  * Use separete texture hi-color buffer for ui in soft render,
  * Convert map surface flag by game type,
  * Cleanup function declarations in game save code,
  * Use 3 bytes vertex normal,
  * Support scalled textures for models and walls in soft render and fix
    lighting with remastered maps.

Not a goal:
  * multiplayer protocol support with KEX engine,
  * support KEX engine features (inventary, compass and so on),
  * [KEX game library support](https://github.com/id-Software/quake2-rerelease-dll).

Code tested with such [maps](doc/100_tested_maps.md).

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
