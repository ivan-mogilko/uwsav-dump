### uwsav-dump

This is a simple command-line tool meant for dumping "Ultima Underworld"'s lev.ark data to a text file.

Dumps following:
- Level map, in ASCII (marks openings, walls and doors only).
- Object list, printed per level tile, with nested NPC's inventories and container contents.

The original purpose of this tool is to search for objects of particular kind on the game levels.
Because UW player saves also contain full copy of level data (as lev.ark file) this lets dump both startup level data and the saved state.
May be useful if you've lost an important item and want to find it without searching whole game world in-game.
Also may be utilized for statistics and level research.

Usage:

    uwsav-dump.exe <input-lvl.ark> <output-text-file>

Building:

1. Windows: MSVS 2019 or higher, solution is available inside `msvc` dir.
2. Linux: use `make`, Makefile is available in the repo's root.
3. Other: potentially may build on FreeBSD and macOS using same Makefile, but did not test myself.

### License

[MIT License](LICENSE.md)

### Acknowledgements

Used thorough format specification by [vividos](https://github.com/vividos/), found at:
http://bootstrike.com/Ultima/Online/uwformat.php

Another instance of format explanation may be found in Ultima wiki:
https://wiki.ultimacodex.com/wiki/Ultima_Underworld_internal_formats
