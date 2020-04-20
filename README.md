ofxArtool5
=====================================

OpenFrameworks addon wrapping the functionalities of ArtoolKit 5.
At the moment it provides :

- Marker based AR
- NFT based AR

License
-------
Artoolkit is [LGPLv3](https://github.com/artoolkit/artoolkit5/blob/master/LICENSE.txt), so you can consider my code LGPLv3 too.  
Copyright (c) 2015 Enrico<naus3a>Viola
naus3a@gmail.com

Installation
------------
Copy to your openFrameworks/addons folder.

Compatibility
------------
openFrameworks 009+ (tested on 011)


Known issues
------------
For some reason (I think it's a libjpg issue), it seems like you cannot load .iset image set files (used for NFT) from OF; since I could not find documentation about the .iset format, I simply made my own custom.dat file providing the same functionality.

You can use ofxArtool5::saveSurfaceSetData to make a your own custom file from an Artoolkit surface set.

Version history
------------



