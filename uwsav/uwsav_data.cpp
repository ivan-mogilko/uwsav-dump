#include "uwsav_data.h"

// Various constants; UW format has many things fixed in size and number.
const uint32_t LevelTilemapBlockSize = 31752;
const uint16_t MobileObjectsLimit    = 256;
const uint16_t StaticObjectsLimit    = 768;
const uint16_t TotalObjectsLimit     = (MobileObjectsLimit + StaticObjectsLimit);

// General file block info
/*
    The file is a container for several differently-sized blocks that contain
    different infos of the level maps. Some blocks may be unused, e.g. automap
    blocks.

    The file header looks like this:

    0000   Int16   number of blocks in file
    0002   Int32   file offset to block 0
    0006   Int32   file offset to block 1
    ...            etc.
*/
struct DataBlockInfo
{
    uint32_t index = 0u;
    uint32_t offset = 0u;
    uint32_t size = 0u;
};

// Packed Tile data
/*
    For each tile there are two Int16 that describe a tile's properties.

        bits     len  description

    0000 tile properties / flags:
        0- 3     4    tile type (0-9, see below)
        4- 7     4    floor height
        8        1    unknown (?? special light feature ??) always 0 in uw1
        9        1    0, never used in uw1
        10-13    4    floor texture index (into texture mapping)
        14       1    when set, no magic is allowed to cast/to be casted upon
        15       1    door bit (when 1, a door is present)

    0002 tile properties 2 / object list link
        0- 5     6    wall texture index (into texture mapping)
        6-15     10   first object in tile (index into master object list)
*/
struct TileDataPacked
{
    uint16_t data1 = 0u;
    uint16_t data2 = 0u;
};

// Packed Object data
/*
    The "general object info" block looks as following:

        bits  size  field      description

    0000 objid / flags
        0- 8   9   "item_id"   Object ID (see below)
        9-12   4   "flags"     Flags
        12     1   "enchant"   Enchantment flag (enchantable objects only)
        13     1   "doordir"   Direction flag (doors)
        14     1   "invis"     Invisible flag (don't draw this object)
        15     1   "is_quant"  Quantity flag (link field is quantity/special)

    0002 position
        0- 6   7   "zpos"      Object Z position (0-127)
        7- 9   3   "heading"   Heading (*45 deg)
        10-12  3   "ypos"      Object Y position (0-7)
        13-15  3   "xpos"      Object X position (0-7)

    0004 quality / chain
        0- 5   6   "quality"   Quality
        6-15   10  "next"      Index of next object in chain

    0006 link / special
        0- 5   6   "owner"     Owner / special
        6-15   10  (*)         Quantity / special link / special property
*/
struct ObjectDataPacked
{
    // basic info
    uint16_t data1 = 0u;
    uint16_t data2 = 0u;
    uint16_t data3 = 0u;
    uint16_t data4 = 0u;
    // mobile info ... todo
};


// Unpacks packed tile data into the TileData struct
static TileData UnpackTileData(const TileDataPacked& ptile)
{
    TileData tile;
    tile.Type = static_cast<TileType>(ptile.data1 & 0x7);
    tile.IsDoor = (ptile.data1 & 0x8000) != 0;
    tile.FirstObjLink = (ptile.data2 >> 6) & 0x3FF;
    return tile;
}

// Unpacks packed object data into the ObjectData struct
static ObjectData UnpackObjectData(const ObjectDataPacked& pobj)
{
    ObjectData obj;
    obj.ItemID = pobj.data1 & 0x1FF;
    obj.Flags = (pobj.data1 >> 9) & 0xF;
    obj.NextObjLink = (pobj.data3 >> 6) & 0x3FF;
    obj.Special = (pobj.data4 >> 6) & 0x3FF;
    return obj;
}

// Reads tilemap + master object list of a single level
static void ReadLevelTilemap(Stream &in, LevelData &levelinfo)
{
/*
    The first 0x4000 bytes of each "level tilemap/master object list" contain
    the tilemap info bytes.

    This is followed by the master object list:
    there are 1024 slots, which may be filled or empty, for 256 mobile objects
    (27 bytes each) and 768 static objects (8 bytes each).

    mobile object information (objects 0000-00ff, 256 x 27 bytes)
    static object information (objects 0100-03ff, 768 x 8 bytes)
*/
    const uint16_t tile_num = 64 * 64;
    std::vector<TileDataPacked> tiles(tile_num);
    for (uint16_t i = 0; i < tile_num; ++i)
    {
        tiles[i].data1 = in.ReadInt16LE();
        tiles[i].data2 = in.ReadInt16LE();
    }

    std::vector<ObjectDataPacked> objs(TotalObjectsLimit);
    // Mobile objects: have general obj data + mobile data
    for (uint16_t i = 0; i < MobileObjectsLimit; ++i)
    {
        objs[i].data1 = in.ReadInt16LE();
        objs[i].data2 = in.ReadInt16LE();
        objs[i].data3 = in.ReadInt16LE();
        objs[i].data4 = in.ReadInt16LE();
        // mobile info, skip for now
        in.Seek(19, kSeekCurrent);
    }
    // Static objects: have general obj data only
    for (uint16_t i = MobileObjectsLimit; i < TotalObjectsLimit; ++i)
    {
        objs[i].data1 = in.ReadInt16LE();
        objs[i].data2 = in.ReadInt16LE();
        objs[i].data3 = in.ReadInt16LE();
        objs[i].data4 = in.ReadInt16LE();
    }

    for (const auto ptile : tiles)
        levelinfo.tiles.push_back(UnpackTileData(ptile));
    for (const auto pobj: objs)
        levelinfo.objs.push_back(UnpackObjectData(pobj));
}

void ReadLevels(Stream &in, std::vector<LevelData> &levels)
{
    levels.clear();

    uint16_t num_blocks = in.ReadInt16LE();
    std::vector<DataBlockInfo> blocks(num_blocks);
    for (uint16_t i = 0; i < num_blocks; ++i)
    {
        blocks[i].index = i;
        blocks[i].offset = in.ReadInt32LE();
    }
    for (uint16_t i = 0; i < num_blocks - 1; ++i)
    {
        blocks[i].size = blocks[i + 1].offset - blocks[i].offset;
    }
    blocks[num_blocks - 1].size = static_cast<uint32_t>(in.GetLength() - blocks[num_blocks - 1].offset);

    // TODO: an option telling what we read, uw1 or uw2 etc;
    // different games may have different block types and their sizes
    for (uint16_t blk_index = 0; blk_index < blocks.size(); ++blk_index)
    {
        // Block sizes are constant, we may use these to identify block type
        if (blocks[blk_index].size != LevelTilemapBlockSize)
            continue;

        in.Seek(blocks[blk_index].offset, kSeekBegin);
        LevelData level;
        ReadLevelTilemap(in, level);
        levels.push_back(std::move(level));
    }
}
