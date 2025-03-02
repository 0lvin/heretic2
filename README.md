# Yamagi Quake II Remaster

This is an experimental fork of Yamagi Quake II with ongoing work to add
support for Quake II Enhanced aka Q2 Remaster(ed). This enhanced version
has a lot non trivial changes, adding support isn't easy and takes time.
Feel free to try this code but you mileage may vary.

Have a look at the yquake2 repository for the "normal" Yamagi Quake II:
<https://github.com/yquake2/yquake2>

Alpha windows 64 bit [binaries](https://github.com/yquake2/yquake2remaster/releases).
Saves format is unstabled and could change between alpha releases.

Models support:

| Format | Original Game   | Frame vertex | Meshes   | Comments                                |
| ------ | --------------- | ------------ | -------- | --------------------------------------- |
| mdl    | Quake 1         | 8 bit        | Single   | Unsupported grouped textures            |
| md2    | Quake 2         | 8 bit        | Single   |                                         |
| mda    | Anachronox      | Part of md2  | Single   | Unsupported skin pass combine           |
| md2    | Anachronox      | 8/10/16 bit  | Single   | Unchecked with game                     |
| mdx    | Kingpin         | 8 bit        | Multiple | No sfx support, unchecked with game     |
| fm     | Heretic 2       | 8 bit        | Multiple | Without skeletal animation              |
| def    | SiN             | Part of sam  | Multiple | Unchecked with game                     |
| dkm    | Daikatana DKM1  | 8 bit        | Multiple | Unchecked with game                     |
| dkm    | Daikatana DKM2  | 10 bit       | Multiple | Unchecked with game                     |
| md3    | Quake 3         | 16 bit       | Multiple | No tags support                         |
| mdr    | EliteForce      | float        | Multiple | No tags support. Uses first LOD only    |
| md5    | Doom 3/Quake 4  | float        | Multiple | Requires md2 for skins                  |
| sbm    | SiN             | Part of sam  | Multiple | Unchecked with game                     |
| sam    | SiN             | 8 bit        | Multiple | Unchecked with game                     |

All models support only single texture for all meshes and frames limit based on game protocol.

Texture support:

| Format | Original Game  | Comments |
| ------ | -------------- | -------- |
| wal    | Quake 2        | 8 bit    |
| wal    | Daikatana      | 8 bit    |
| swl    | SiN            | 8 bit    |
| m8     | Heretic 2      | 8 bit    |
| m32    | Heretic 2      | 24 bit   |
| pcx    | Quake2         | 24 bit   |
| tga    | Quake2         | 24 bit   |
| png    | retexturing    | 24 bit   |
| jpg    | retexturing    | 24 bit   |
| bmp    | Daikatana      | 24 bit   |

Sprites support:

| Format | Original Game  | Comments                      |
| ------ | -------------- | ----------------------------- |
| sp2    | Quake 2        |                               |
| atd    | Anachronox     | Show first frame of animation |

Maps support:

| Format | Version | Game                                       |
| ------ | ------- | ------------------------------------------ |
| IBSP   | 39      | Quake 2 / Anachronox / Kingpin / Heretic 2 |
| IBSP   | 41      | Daikatana / SIN                            |
| RBSP   | 1       | SIN                                        |
| QBSP   | 39      | Quake 2 ReRelease                          |
| BSPX   | 39      | Quake 2 ReRelease (Extension to IBSP)      |

Note:

* Non Quake 2 maps are limited mostly view only, and could have issues
   with tranparency or some animations flags and properties.
* If you like support some other maps type, create pull request for Mod_Load2QBSP
   function and provide a link to demo maps.
* Use `maptype 1` before load any Heretic 2 maps, or place game data to `heretic2` directory.
   Look to [maptype_t](src/common/header/cmodel.h#L42) for more info.

Games:

* Quake 2 ReRelease:
  * SDK: <https://github.com/id-Software/quake2-rerelease-dll>
  * Tech info: <https://bethesda.net/en/article/6NIyBxapXOurTKtF4aPiF4/enhancing-quake-ii>
  * PSX source: <https://www.moddb.com/mods/quake-ii-psx/downloads/quake-ii-psx-10-sources>
  * PSX Mod: <https://www.moddb.com/mods/quake-ii-psx>
* Anachronox:
  * SDK: <https://github.com/hogsy/chronon>
  * SDK: <https://code.idtech.space/ion-storm/anachronox-sdk>
  * Tech info: <https://anachrodox.talonbrave.info/>
* Kingpin:
  * SDK: <https://github.com/QuakeTools/Kingpin-SDK-v1.21>
  * SDK: <https://code.idtech.space/xatrix/kingpin-sdk>
  * Tech info: <https://www.kingpin.info/>
* Daikatana:
  * Info: <http://daikatananews.net/>
* Heretic 2:
  * SDK: <https://www.quaddicted.com/files/idgames2/planetquake/hereticii/files/Ht2Toolkit_v1.06.exe>
  * SDK: <https://code.idtech.space/raven/heretic2-sdk>
  * Tech info: <http://h2vault.infinityfreeapp.com/index.html>
* SiN:
  * Tools: [SiNview](https://web.archive.org/web/20001212060900/http://starbase.neosoft.com:80/~otaku/program.html)
  * Tools: <https://www.moddb.com/games/sin/downloads/sin-modding-tools-and-other-stuff>
  * SDK: <https://github.com/NightDive-Studio/sin-ex-game>
  * SDK: <https://github.com/jimdose/SiN_110_Source>
  * SDK: <https://code.idtech.space/ritual/sin-sdk>
* Dawn of Darkness:
  * Docs: <https://www.moddb.com/mods/dawn-of-darkness1/downloads/dod-mood-scripts-gsm-tutorials-fgd-and-def-file>
  * Demo: [Episode 1](https://www.moddb.com/mods/dawn-of-darkness1/downloads/dawn-of-darkness-episode-1)
* Additional maps used for check maps support:
  * PSX: <https://www.moddb.com/mods/quake-ii-psx/downloads/quake-ii-psx-10>
  * ReRelease N64 Jam: <https://www.moddb.com/games/quake-2/addons/quake-2-re-release-n64-sp-map-jam>
  * ReRelease Basic Jam: <https://www.moddb.com/games/quake-2/addons/quake-2-re-release-back-to-baseq2ics-jam-1>
  * ReRelease PSX Jam: <https://www.moddb.com/mods/psx-jam-1/downloads/quake-2-re-release-psx-jam-1>

Games check videos:

* 8.42RR11:

[![First episode](https://img.youtube.com/vi/Ha1FuVXaQSE/0.jpg)](https://www.youtube.com/watch?v=Ha1FuVXaQSE)
[![Q2DQ2](https://img.youtube.com/vi/6P3wJojExyI/0.jpg)](https://www.youtube.com/watch?v=6P3wJojExyI)
[![8.42RR11](https://img.youtube.com/vi/ukqBrx80ESM/0.jpg)](https://www.youtube.com/watch?v=ukqBrx80ESM)

* 8.42RR10:

[![8.42RR10](https://img.youtube.com/vi/obIrzYsNxBY/0.jpg)](https://www.youtube.com/watch?v=obIrzYsNxBY)

* 8.42RR9:

[![8.42RR9](https://img.youtube.com/vi/N0iHhEDkZFg/0.jpg)](https://www.youtube.com/watch?v=N0iHhEDkZFg)

* 8.42RR8:

[![8.42RR8](https://img.youtube.com/vi/NJ7T0cdyqk8/0.jpg)](https://www.youtube.com/watch?v=NJ7T0cdyqk8)

* 8.31RR7:

[![8.31RR7](https://img.youtube.com/vi/VAFs1HtQU_0/0.jpg)](https://www.youtube.com/watch?v=VAFs1HtQU_0)


Goals, fully finished goals could be checked in [here](CHANGELOG):

* [ ] soft: support custom ttf fonts,
* [ ] skins load broken in Anachronox maps with mingw win64 build,
* [ ] Anachronox rowdys: models disappear on dance space,
* [ ] soft: q64/outpost scale textures unsupported,
* [ ] soft: broken wall light and wall glitch,
* [ ] soft: build with SDL3 has glitch in menu,
* [ ] mguhub: sometimes broken logic for surface fall in next maps,
* [ ] incorrect light apply to models (wall looks fine),
* [ ] incorrect dead animation for Arachnid,
* [ ] broken fire effect for Guardian.
* [ ] CTC entity format from Anachronox,
* [ ] SDEF dynamicaly allocate list of skins,
* [ ] Support material load textures/textureinfo.dat from Anachronox,
* [ ] Fix invisiable entities in basicsjam1_ziutek,
* [ ] Make lightmap textures dynamic n64jam_palmlix,
* [x] Support textures/*/*.mat load from ReRelease (footstep),
* [ ] Support textures/*/*.mat load from ReRelease texture effects,
* [ ] Support textures/*/*_glow.png load from ReRelease,
* [ ] Support tactile/*/*.bnvib/.wav feedback load from ReRelease,
* [ ] Fix physics with incorrect floor height in psx/base0.bsp,
* [ ] Make pmove_state_t.origin 29.3 (PS_M_ORIGIN) >4k coord values support,
* [ ] Fix statusbar for DoD `roarke`,
* [ ] Group `it_pic` images in vulkan render,
* [ ] Rearange surfaces in vulkan render before render,
* [ ] Fully implement `misc_flare`,
* [ ] Single player ReRelease support,
* [ ] Support effects and additional flags for ReRelease when possible.
* [ ] Use shared model cache in client code insted reimplemnet in each render,
* [ ] Fix transparent textures in Daikatana/SiN maps,
* [ ] Use separete texture hi-color buffer for ui in soft render,
* [ ] Cleanup function declarations in game save code,
* [ ] Fix broken base3 with sorted fields names,
* [ ] Support scalled textures for models and walls in soft render and fix
    lighting with remastered maps,
* [ ] Modified ReRelease game code support with removed KEX only related code.
* [ ] Add support for [JaBot](https://www.moddb.com/mods/jabotq2/downloads/jabot-q2-v09x-win32-and-linux)

Not a goal:

* [ ] Multiplayer protocol support with KEX engine,
* [ ] Support KEX engine features (inventary, compass and so on),
* [ ] [KEX game library support](https://github.com/id-Software/quake2-rerelease-dll).


# Additional requirements:

Localization requires `Q2Game.kpf` file in root directory of game. If you
like to support your language put it to localization/loc_<your language>.txt
and extend MAX_FONTCODE to your [max symbol code](https://en.wikipedia.org/wiki/List_of_Unicode_characters).
Used font and language file are defined by `g_language` and `r_ttffont`, as
an example could be used fonts like [unifont](https://unifoundry.com/pub/unifont/unifont-15.0.06/font-builds/unifont-15.0.06.ttf).

Quake 1 models usage requires such models:

 | source Quake 1 Path        | destination Quake 2 Path          | md5 hash                         |
 | -------------------------- | --------------------------------- | -------------------------------- |
 | progs/soldier.mdl          | models/monsters/army/tris.mdl     | 5b6c30a984872b4273dd5861412d35c5 |
 | progs/demon.mdl            | models/monsters/demon/tris.mdl    | 4c73786e7cfb2083ca38cbc983cd6c4b |
 | progs/dog.mdl              | models/monsters/dog/tris.mdl      | e727fbc39acc652f812972612ce37565 |
 | progs/enforcer.mdl         | models/monsters/enforcer/tris.mdl | 136c265f96d6077ee3312c52e134529f |
 | progs/fish.mdl             | models/monsters/rotfish/tris.mdl  | d770d6ef92ae8b372926e6c3d49e8716 |
 | progs/hknight.mdl          | models/monsters/hknight/tris.mdl  | ed20e30be6fdb83efbaa6d0b23671a49 |
 | progs/knight.mdl           | models/monsters/knight/tris.mdl   | 5328915db5c53e85cf75d46e7b747fb9 |
 | progs/ogre.mdl             | models/monsters/ogre/tris.mdl     | fbb592ca3788a576dd2f31fcf8c80fab |
 | progs/shalrath.mdl         | models/monsters/shalrath/tris.mdl | dac8f6077b0d2bf970573d2d2c22529f |
 | progs/shambler.mdl         | models/monsters/shambler/tris.mdl | 9b09375a0f614dc4081e363cc34f1185 |
 | progs/tarbaby.mdl          | models/monsters/tarbaby/tris.mdl  | 2bfc45593a43a0191982e3cfdc112dc5 |
 | progs/wizard.mdl           | models/monsters/wizard/tris.mdl   | bf60986594f045e60e8aa6339d553ee7 |
 | progs/zombie.mdl           | models/monsters/zombie/tris.mdl   | a923b1d03b9d237dd5ead9bd56acc900 |
 | progs/k_spike.mdl          | models/proj/fireball/tris.mdl     | da95b49a695b34c2c3516a8d789c6b20 |
 | progs/v_spike.mdl          | models/proj/pod/tris.mdl          | 2bdc0b264b9fd3443c8eee816305728f |
 | progs/w_spike.mdl          | models/proj/spit/tris.mdl         | 8cf9245df2164e8062b01a220b508d6b |
 | sound/army/death1.wav      | sound/army/death1.wav             | fe25952af40b536c3bb67bfca58062eb |
 | sound/army/idle.wav        | sound/army/idle.wav               | 4cdfbdda8fb5f40b125c7bcddb6e8914 |
 | sound/army/pain1.wav       | sound/army/pain1.wav              | 54d3b05fdcecf7480034858f2b22b06c |
 | sound/army/pain2.wav       | sound/army/pain2.wav              | e2bbc9d3405b9b9fdae76d2fc298fc2f |
 | sound/army/sattck1.wav     | sound/army/sattck1.wav            | b76ac35f900f280a7a7996da7e404362 |
 | sound/army/sight1.wav      | sound/army/sight1.wav             | e2a39533250ddd1b08053d5c4ebf7fd0 |
 | sound/demon/ddeath.wav     | sound/demon/ddeath.wav            | 62a48dcd405ca55c9ce18497e36faa0e |
 | sound/demon/dhit2.wav      | sound/demon/dhit2.wav             | 9e741696e4d0d66e30e718fc70ec186e |
 | sound/demon/djump.wav      | sound/demon/djump.wav             | 791ac9d6b6593808316145443cb7e7a0 |
 | sound/demon/dland2.wav     | sound/demon/dland2.wav            | db14c7b01838f826b7a712075f9026fd |
 | sound/demon/dpain1.wav     | sound/demon/dpain1.wav            | 5e6c485878393c9bebb0d0e6659bc479 |
 | sound/demon/idle1.wav      | sound/demon/idle1.wav             | 7a83f8caf3d5b1172a7dc5afdc942988 |
 | sound/demon/sight2.wav     | sound/demon/sight2.wav            | 65171c355c22a5069520b0881677ae3c |
 | sound/dog/dattack1.wav     | sound/dog/dattack1.wav            | 0c47c9d4aaae0bf3159a288b674f4a35 |
 | sound/dog/ddeath.wav       | sound/dog/ddeath.wav              | b0e7760b1d7286a028ff9773f6802958 |
 | sound/dog/dpain1.wav       | sound/dog/dpain1.wav              | 28e5ab93b41e13e2916433283a5d1a46 |
 | sound/dog/dsight.wav       | sound/dog/dsight.wav              | 0edc290765bac1fb4fe03ca55bc7c225 |
 | sound/dog/idle.wav         | sound/dog/idle.wav                | 42bd6026ff32bd94523ad9f8fe8362cc |
 | sound/enforcer/death1.wav  | sound/enforcer/death1.wav         | f38d7a235e13ef8918aac0c70437320d |
 | sound/enforcer/enfire.wav  | sound/enforcer/enfire.wav         | b080a6a53f138bef9fc4403fb9373a24 |
 | sound/enforcer/enfstop.wav | sound/enforcer/enfstop.wav        | 0bc39c3c88187fb37d20ef21300e7b22 |
 | sound/enforcer/idle1.wav   | sound/enforcer/idle1.wav          | d0435763860fe54e938ad25bb1780f9e |
 | sound/enforcer/pain1.wav   | sound/enforcer/pain1.wav          | d52bc02afe624924684b25295edaa040 |
 | sound/enforcer/pain2.wav   | sound/enforcer/pain2.wav          | f6b0ad6485d93f5c6d7d61b847d06877 |
 | sound/enforcer/sight1.wav  | sound/enforcer/sight1.wav         | 9e9876444ba92ed18bfce4cec3a2ff12 |
 | sound/enforcer/sight2.wav  | sound/enforcer/sight2.wav         | 815dd4f6a890b0a35dc466005df74850 |
 | sound/enforcer/sight3.wav  | sound/enforcer/sight3.wav         | f8c8d19905b24d4adb6760a1b3762f3b |
 | sound/enforcer/sight4.wav  | sound/enforcer/sight4.wav         | 04e40eb925290bc56520de326df6b220 |
 | sound/fish/bite.wav        | sound/fish/bite.wav               | 7c56712615e2a2c3de4fe0e7e35cba05 |
 | sound/fish/death.wav       | sound/fish/death.wav              | 8d6152ee55507430a8c2eb276c4aa8ac |
 | sound/fish/idle.wav        | sound/fish/idle.wav               | 54a02731b85eb6f8b1a8ecce09771d20 |
 | sound/hknight/attack1.wav  | sound/hknight/attack1.wav         | 695e9d890a085addf1a607aa79137e8e |
 | sound/hknight/death1.wav   | sound/hknight/death1.wav          | 2d68c3c86632027676b823fcff3af08c |
 | sound/hknight/grunt.wav    | sound/hknight/grunt.wav           | 122f3d4623e0146732aa0e4deb0fe326 |
 | sound/hknight/hit.wav      | sound/hknight/hit.wav             | fece24e2548f1d769e8fce39a5c87d01 |
 | sound/hknight/idle.wav     | sound/hknight/idle.wav            | e9ca23c4853472750d1f56e52a3d227b |
 | sound/hknight/pain1.wav    | sound/hknight/pain1.wav           | 52c20eb6f5b5146ece6b0ac02f423341 |
 | sound/hknight/sight1.wav   | sound/hknight/sight1.wav          | 786d28ec653a0d512d078b32eb8cb58b |
 | sound/hknight/slash1.wav   | sound/hknight/slash1.wav          | 83e64bb90cff2a4f9764dd53489b9d7d |
 | sound/knight/idle.wav      | sound/knight/idle.wav             | 1b5c3981bee078318af346d8fe8090b9 |
 | sound/knight/kdeath.wav    | sound/knight/kdeath.wav           | 11a96f48a8f2433c23beae299c8fb96e |
 | sound/knight/khurt.wav     | sound/knight/khurt.wav            | 32c9f48dd4a6a87752aedf56fe8de668 |
 | sound/knight/ksight.wav    | sound/knight/ksight.wav           | eb75b4af348c88e84d8c87c19d5976fa |
 | sound/knight/sword1.wav    | sound/knight/sword1.wav           | 93045b5f3178413dc6b01ec2869d1f73 |
 | sound/knight/sword2.wav    | sound/knight/sword2.wav           | 82d1a7c263cd3df7b060dee2f7f8b1fd |
 | sound/ogre/ogdrag.wav      | sound/ogre/ogdrag.wav             | 658587dc89aaefae5ca3f058f7ff4227 |
 | sound/ogre/ogdth.wav       | sound/ogre/ogdth.wav              | de98e7bb9e70586edddd3308cfded3db |
 | sound/ogre/ogidle2.wav     | sound/ogre/ogidle2.wav            | 196ae9c33a4311160e1d5f3da878ab09 |
 | sound/ogre/ogidle.wav      | sound/ogre/ogidle.wav             | 177c829840a87c885bbc8951e7ce962e |
 | sound/ogre/ogpain1.wav     | sound/ogre/ogpain1.wav            | ac6d133d7720c12df49813fe10e683b5 |
 | sound/ogre/ogsawatk.wav    | sound/ogre/ogsawatk.wav           | 7257bdfd014bb61d778b8f88cb65d560 |
 | sound/ogre/ogwake.wav      | sound/ogre/ogwake.wav             | 3296459e495c4c12b31179d4eeffac19 |
 | sound/shalrath/attack2.wav | sound/shalrath/attack2.wav        | 777fbda2f81350e45920ea1c36d318e1 |
 | sound/shalrath/attack.wav  | sound/shalrath/attack.wav         | 8814a72b399be619d14223235f78f294 |
 | sound/shalrath/death.wav   | sound/shalrath/death.wav          | 771844c5cfb5a42e82e221a63f101ac7 |
 | sound/shalrath/idle.wav    | sound/shalrath/idle.wav           | 1a45f12579f9cdd27152332d5f816048 |
 | sound/shalrath/pain.wav    | sound/shalrath/pain.wav           | 6ba007b79c50d7d5347f1a29fb315422 |
 | sound/shalrath/sight.wav   | sound/shalrath/sight.wav          | 3d3c2001cc976efc67af1552427eedd3 |
 | sound/shambler/melee1.wav  | sound/shambler/melee1.wav         | 77fe6240973a204b757a427ebf66bcfd |
 | sound/shambler/melee2.wav  | sound/shambler/melee2.wav         | 3be31638d769e3d690c92311dfafdeac |
 | sound/shambler/sattck1.wav | sound/shambler/sattck1.wav        | ec8c3f79ea42d09154cb8975ada1b21c |
 | sound/shambler/sboom.wav   | sound/shambler/sboom.wav          | 9834601a645d7d1f62001b2a314ec18f |
 | sound/shambler/sdeath.wav  | sound/shambler/sdeath.wav         | e0e62d229a985a6e39218c6488599059 |
 | sound/shambler/shurt2.wav  | sound/shambler/shurt2.wav         | 1e87631ede95e061a75f9c31629ac0b8 |
 | sound/shambler/sidle.wav   | sound/shambler/sidle.wav          | 368a240e6c3e877793352a79c1828386 |
 | sound/shambler/smack.wav   | sound/shambler/smack.wav          | bd8c634c9894348360fec7b7419d1649 |
 | sound/shambler/ssight.wav  | sound/shambler/ssight.wav         | 657107577460d5acf3157dec97e07a89 |
 | sound/tarbaby/death1.wav   | sound/tarbaby/death1.wav          | 53f8c73de11c018086ba19ed036833d3 |
 | sound/tarbaby/hit1.wav     | sound/tarbaby/hit1.wav            | 42c5a1edd02421bf6df7ddc3e395c4f8 |
 | sound/tarbaby/land1.wav    | sound/tarbaby/land1.wav           | 5a1585ab22368a48827db9b7d3763b58 |
 | sound/tarbaby/sight1.wav   | sound/tarbaby/sight1.wav          | df4dd7ec91e094b5c2235be3f5353e4a |
 | sound/wizard/hit.wav       | sound/wizard/hit.wav              | 6598d669334df977f1f97b95288edf11 |
 | sound/wizard/wattack.wav   | sound/wizard/wattack.wav          | b3568a47439f81779d40335e76f5bc2d |
 | sound/wizard/wdeath.wav    | sound/wizard/wdeath.wav           | 99d9c71343bc23222abb92119d404400 |
 | sound/wizard/widle1.wav    | sound/wizard/widle1.wav           | b22b1dd9f66879b3447d36ab584316c7 |
 | sound/wizard/widle2.wav    | sound/wizard/widle2.wav           | 7178fd83df227360564adef01bed92c6 |
 | sound/wizard/wpain.wav     | sound/wizard/wpain.wav            | bd7e59ab7d7b835f9ed8ab51950914de |
 | sound/wizard/wsight.wav    | sound/wizard/wsight.wav           | df5949057c750528917c6cb32b7fd9fd |
 | sound/zombie/idle_w2.wav   | sound/zombie/idle_w2.wav          | d2bb59b9d7ac71097c07123a23525641 |
 | sound/zombie/z_fall.wav    | sound/zombie/z_fall.wav           | 6968318b2d196c6d54bc707d8a390547 |
 | sound/zombie/z_gib.wav     | sound/zombie/z_gib.wav            | 199dda66ade8d6f0ef4897e4d7608c0d |
 | sound/zombie/z_hit.wav     | sound/zombie/z_hit.wav            | 6c66ab3e6b149bf35956a4bbc0057acb |
 | sound/zombie/z_idle1.wav   | sound/zombie/z_idle1.wav          | 21088a7b38b1124d7ec72e58d5ab93a6 |
 | sound/zombie/z_idle.wav    | sound/zombie/z_idle.wav           | e5e570e38c4405111004190a79f0e925 |
 | sound/zombie/z_miss.wav    | sound/zombie/z_miss.wav           | 96fad96a8c3bc6f7bcc5da6c82678da2 |
 | sound/zombie/z_pain1.wav   | sound/zombie/z_pain1.wav          | fcd83b89a94b0cadbe1361a4df080ec0 |
 | sound/zombie/z_pain.wav    | sound/zombie/z_pain.wav           | 56637968b8309bd2f825153330bbdfc0 |
 | sound/zombie/z_shot1.wav   | sound/zombie/z_shot1.wav          | 561afb69b7851923a8fb3b54e3e13c96 |

Look to [infighter](https://github.com/decino/q2-infighter) for more info.

# Yamagi Quake II

Yamagi Quake II is an enhanced client for id Software's Quake II with
focus on offline and coop gameplay. Both the gameplay and the graphics
are unchanged, but many bugs in the last official release were fixed and
some nice to have features like widescreen support, reliable support for
high framerates, a modern sound backend based upon OpenAL, support for
modern game controllers and a modern OpenGL 3.2 renderer were added.
Unlike most other Quake II source ports Yamagi Quake II is fully 64-bit
clean. It works perfectly on modern processors and operating systems.

This code is built upon Icculus Quake II, which itself is based on Quake
II 3.21. Yamagi Quake II is released under the terms of the GPL version
2. See LICENSE for further information:

* [LICENSE](https://github.com/yquake2/yquake2/blob/master/LICENSE)

Officially supported operating systems are:

* FreeBSD
* Linux
* Windows

Beside theses Yamagi Quake II has community support for MacOS and most
other unixoid operating systems, including NetBSD, OpenBSD and Solaris.


## Addons and partner projects

This repository contains Yamagi Quake II itself. The official addons
have their own repositories:

* [The Reckoning](https://github.com/yquake2/xatrix)
* [Ground Zero](https://github.com/yquake2/rogue)
* [Three Waves Capture The Flag](https://github.com/yquake2/ctf)

Yamagi Quake II Remaster is a project providing optional support for the
assets of Quake II Remaster by Nightdive Studios and has a less
conservative approach in regards to new features. It also lives in it's
own repository:

* [Yamagi Quake II Remaster](https://github.com/yquake2/yquake2remaster)


## Development

Yamagi Quake II is a community driven project and lives from community
involvement. Please report bugs in our issue tracker:

* [Issue Tracker](https://github.com/yquake2/yquake2/issues)

We are always open to code contributions, no matter if they are small
bugfixes or bigger features. However, Yamagi Quake II is a conservative
project with big focus on stability and backward compatibility. We don't
accept breaking changes. When in doubt please open an issue and ask if a
contribution in welcome before putting too much work into it. Open a
pull request to submit code:

* [Pull Requests](https://github.com/yquake2/yquake2/pulls)

Also have a look at our contributors guide:

* [Contributors Guide](https://github.com/yquake2/yquake2/blob/master/doc/080_contributing.md)


## Documentation

Yamagi Quake II has rather extensive documentation covering all relevant
areas from installation and configuration to package building. Have a
look at the documentation index:

* [Documentation Index](https://github.com/yquake2/yquake2/blob/master/doc/010_index.md)


## Releases

Yamagi Quake II releases at an irregular schedule. The official releases
with source code tarballs and prebuild Windows binaries can be found at
the homepage:

* [Homepage](https://www.yamagi.org/quake2/)

Our CI builds **unsupported** Linux, MacOS and Windows binaries at every
commit. The artifacts can be found here:

* [Github Actions](https://github.com/yquake2/yquake2/actions)
