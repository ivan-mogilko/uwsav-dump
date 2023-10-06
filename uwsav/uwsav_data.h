//=============================================================================
//
// UW save data structs.
//
// Format specs and thorough explanation may be found on the following pages:
// http://bootstrike.com/Ultima/Online/uwformat.php
// https://wiki.ultimacodex.com/wiki/Ultima_Underworld_internal_formats
//
//=============================================================================
#ifndef UWSAV__SAV_DATA_H__
#define UWSAV__SAV_DATA_H__

#include <stdint.h>
#include <vector>
#include "utils/stream.h"

// Level Tile data

struct TileData
{
    uint16_t FirstObjLink = 0u; // ref to obj list
};

struct ObjectData
{
    uint16_t ItemID = 0u; // type of item, defined by game
    uint16_t Flags = 0u;
    uint16_t NextObjLink = 0u; // ref to obj list
    uint16_t Special = 0u; // depends on item type
};

// General Level data
/*
    Each underworld level consists of a 64x64 tile map.
    The map's origin is at the lower left tile, going to the right,
    each line in turn.

    Then there's a master object list, which has a fixed limit of 1024 slots,
    (each of which may be filled or empty), for 256 mobile objects and
    768 static objects.
*/
struct LevelData
{
    std::vector<TileData> tiles;
    std::vector<ObjectData> objs;
};


// Reads LEVEL.ARK file, fills in LevelData array
void ReadLevels(Stream &in, std::vector<LevelData> &levels);

#endif // UWSAV__SAV_DATA_H__
