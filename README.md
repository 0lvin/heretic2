Customized Quake2 engine with Heretic2 code parts.

Heretic 2 SDK code based on [Ht2Toolkit](https://www.quaddicted.com/files/idgames2/planetquake/hereticii/files/Ht2Toolkit_v1.06.exe)
Updated code based on [Heretic 2 Reconstruction Project](https://github.com/jmarshall23/Heretic2/tree/Engine-DLL)

# cleanup code
```shell
sed -i 's/[[:blank:]]*$//' */*.{c,h}
```

Comments:

It's on really initial steps, it could be complied ;-), it runs without
crashes and can open tutorial level. That's all what is good.

Drawbacks: code is not rebased over yquake2 code, no sounds, broken jumps,
no menu or books implementations.
Code can't open ssdocks level(first level of game).
Not all files has correct licence header and code is little bit mess.

======

This is the complete source code for Quake 2, version 3.21, buildable with
visual C++ 6.0.  The linux version should be buildable, but we haven't
tested it for the release.

The code is all licensed under the terms of the GPL (gnu public license).
You should read the entire license, but the gist of it is that you can do
anything you want with the code, including sell your new version.  The catch
is that if you distribute new binary versions, you are required to make the
entire source code available for free to everyone.

The primary intent of this release is for entertainment and educational
purposes, but the GPL does allow commercial exploitation if you obey the
full license.  If you want to do something commercial and you just can't bear
to have your source changes released, we could still negotiate a separate
license agreement (for $$$), but I would encourage you to just live with the
GPL.

All of the Q2 data files remain copyrighted and licensed under the
original terms, so you cannot redistribute data from the original game, but if
you do a true total conversion, you can create a standalone game based on
this code.

Thanks to Robert Duffy for doing the grunt work of building this release.

John Carmack
Id Software


