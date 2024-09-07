-------------------------
SinView 1.0 - July 31, 1998
Copyright (c) Trey Harrison
trey@u.washington.edu
-------------------------

-----------
Instructions
-----------

The first time you run, you'll be promted for the path to your Sin
"base" directory. This is the directory containing the pak0.pak file.

It will then buzz and whirl for a few seconds reading the .pak file,
as well as gather information about the models contained in it.

You'll be presented with a tree view of the .pak on the left hand
side of the screen. You'll find .def files (containing animation names),
.swl files (textures) and .tga files (used for many different things).

To see any file, just click on it.

Some of the .def files reference invalid models or invalid animations.
Warning windows will come up. Just hit escape a few times.

---
3DFX
---

If you don't have a 3dfx card, unzip the included GLIDE2X.ZIP.
Put the glide2x.dll file into the directory from which you run
SinView.

------------
Viewing Models
------------

When you click on an animation in the .pak tree, you'll see a little
window off to the right with the animation. Click over there, and it
will jump you into fullscreen view mode where you can do fancy things:

The mouse and arrow keys move the camera.
The J and K keys rotate the model.
The L key changes the direction of lighting.
The R,G,B keys (hold shift, dont hold shift) adjust the color of the light.
The S key takes a screenshot.
The +/- keys increase / decrease animation speed.
The N and M keys switch between skins.
The Escape key exits the model viewer.

-----
Source
-----

Unzip the included svsrc.zip. A short explanation can be found in the
[src_doc.txt](src_doc.txt).

------
Updates
------

You'll be able to find the latest version of SinView (and other cool
programs) at [http://starbase.neosoft.com/~otaku/program.html](https://web.archive.org/web/20001212060900/http://starbase.neosoft.com:80/~otaku/program.html)

----------
Bugs / Misc
----------

If there is a file in the .pak file (say, pl_blade.def) that ALSO exists
on disk, then the one that is used by SinView is the one from disk.

Unfortunately, files that are on disk BUT ARENT in the .pak file will not
show up in the tree. This is a feature I'd like to add soon.

Thanks to Mark D. at Ritual for help on the file formats.

Sin and the Sin 'S' logo are trademarks
of Ritual Entertainment, Inc. (http://www.ritual.com)

Trey Harrison
trey@u.washington.edu
