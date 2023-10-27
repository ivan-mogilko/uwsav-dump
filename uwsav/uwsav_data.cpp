#include <assert.h>
#include "uwsav_data.h"
#include "utils/memorystream.h"

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
    uint32_t Index = 0u;
    uint32_t Offset = 0u;
    bool     IsCompressed = false; // UW2
    bool     HasAvailSpace = false; // UW2
    uint32_t Size = 0u;
    uint32_t AvailSpace = 0u; // UW2
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

    /*
        If the "is_quant" field is 0 (unset), it contains the index of an associated
        object.
        If the "is_quant" flag is set, the field is a quantity or a special
        property. If the value is < 512 or 0x0200 it gives the number of stacked
        items present.
        If the value is > 512, the value minus 512 is a special property; the
        object type defines the further meaning of this value.
    */
    bool is_quant = (pobj.data1 & 0x8000) != 0;
    uint16_t special = (pobj.data4 >> 6) & 0x3FF;
    if (is_quant && special < 512)
        obj.Quantity = special;
    else if (is_quant && special > 512)
        obj.SpecialProperty = special - 512;
    else
        obj.SpecialLink = special;
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

void ReadLevelsUW1(Stream &in, std::vector<LevelData> &levels)
{
    levels.clear();

    uint16_t num_blocks = in.ReadInt16LE();
    std::vector<DataBlockInfo> blocks(num_blocks);
    for (uint16_t i = 0; i < num_blocks; ++i)
    {
        blocks[i].Index = i;
        blocks[i].Offset = in.ReadInt32LE();
    }
    for (uint16_t i = 0; i < num_blocks - 1; ++i)
    {
        blocks[i].Size = blocks[i + 1].Offset - blocks[i].Offset;
    }
    blocks[num_blocks - 1].Size = static_cast<uint32_t>(in.GetLength() - blocks[num_blocks - 1].Offset);

    uint8_t level_id = 1u;
    for (const auto &block : blocks)
    {
        // Block sizes are constant, we may use these to identify block type
        if (block.Size != LevelTilemapBlockSize)
            continue;

        LevelData level;
        level.LevelID = level_id++;
        in.Seek(block.Offset, kSeekBegin);
        ReadLevelTilemap(in, level);
        levels.push_back(std::move(level));
    }
}

bool UncompressUW2Block(const std::vector<uint8_t> &in_data, std::vector<uint8_t> &out_data)
{
    /*
       A compressed block always starts with an Int32 value that is to be ignored.
       If a block is actually compressed, it can be divided into subblocks.
       Each compressed subblock starts with an Int8 number; the bits from LSB to
       MSB describe if the following byte is just transferred to the target buffer
       (bit set) or if we have a copy record (bit cleared). After 8 bytes or copy
       record, the next subblock begins with an Int8 again.

       The copy record starts with two Int8's:
       0000   Int8   0..7: position, bits 0..7
       0001   Int8   0..3: copy count
                     4..7: position, bits 8..11

       The copy count is 4 bits long and an offset of 3 is added to it. The
       position has 12 bits (accessing the last 4k bytes) and an offset of 18 is
       added. The sign bit is bit 11 and should be treated appropriate. As the
       position field refers to a position in the current 4k segment, pointers
       have to be adjusted, too. Then "copy count" bytes are copied from the
       relative "position" to the current one.

       Also used this for a reference (could not understand "copy record part"):
       https://github.com/vividos/UnderworldAdventures/blob/main/uwadv/source/base/Uw2decode.cpp
    */

    const uint8_t *src = &in_data.front();
    const uint8_t *src_end = src + in_data.size();
    out_data.reserve(in_data.size());

    src += sizeof(int32_t); // unused?
    // The decompression loop
    while (src < src_end)
    {
        uint8_t buf_bits = *(src++);
        for (int b = 0; b < 8; ++b)
        {
            if (buf_bits & (1 << b))
            {
                // Direct copy byte
                out_data.push_back(*(src++));
            }
            else
            {
                // Copy "record": this means copy previously written *uncompressed* data
                // read 2 int32 with packed data and expand them into position and count
                int32_t i1 = *(src++);
                int32_t i2 = *(src++);
                int32_t position = i1 | ((i2 & 0xF0) << 4);
                // correct for sign bit
                if (position & 0x800)
                    position |= 0xFFFFF000;
                uint8_t count = (i2 & 0x0F);
                // add magic hardcoded offsets
                position += 18;
                count += 3;

                assert(position < 0 || static_cast<uint32_t>(position) < out_data.size());
                // adjust pos to current 4k segment
                while (position < 0 ||
                       (out_data.size() >= 4096) && (static_cast<uint32_t>(position) < (out_data.size() - 4096)))
                    position += 4096;
                // do the copying
                while (count--)
                    out_data.push_back(out_data[position++]);
            }
        }
    }
    return true;
}

void ReadLevelsUW2(Stream &in, std::vector<LevelData> &levels)
{
    levels.clear();

    uint16_t num_blocks = in.ReadInt16LE();
    in.ReadInt32LE(); // skip unknown
    std::vector<DataBlockInfo> blocks(num_blocks);
    for (uint16_t i = 0; i < num_blocks; ++i)
    {
        blocks[i].Index = i;
        blocks[i].Offset = in.ReadInt32LE();
    }
    for (uint16_t i = 0; i < num_blocks; ++i)
    {
        uint32_t flags = in.ReadInt32LE();
        blocks[i].IsCompressed = flags & 0x2;
        blocks[i].HasAvailSpace = flags & 0x4;
    }
    for (uint16_t i = 0; i < num_blocks; ++i)
    {
        blocks[i].Size = in.ReadInt32LE();
    }
    for (uint16_t i = 0; i < num_blocks; ++i)
    {
        blocks[i].AvailSpace = in.ReadInt32LE();
    }

    /*
        Ultima Underworld 2 has 320 (0x0140) entries (80 levels x 4 blocks). These
        can be split into 4 sets of 80 entries each:

           0.. 79  level maps
          80..159  texture mappings
         160..239  automap infos
         240..319  map notes
    */
    uint16_t blk_index = 0;
    for (uint16_t world_id = 0; world_id < 10; ++world_id)
    {
        for (uint16_t level_id = 0; level_id < 8; ++level_id)
        {
            const auto &block = blocks[blk_index++];
            if (block.Offset == 0 || block.Size == 0)
                continue; // unused

            LevelData level;
            level.LevelID = level_id + 1;
            level.WorldID = world_id + 1;

            in.Seek(block.Offset, kSeekBegin);
            if (block.IsCompressed)
            {
                std::vector<uint8_t> in_data(block.Size);
                in.Read(&in_data.front(), block.Size);
                std::vector<uint8_t> out_data;
                if (UncompressUW2Block(in_data, out_data))
                {
                    Stream mems(std::make_unique<VectorStream>(out_data, kStream_Read));
                    ReadLevelTilemap(mems, level);
                }
            }
            else
            {
                ReadLevelTilemap(in, level);
            }

            levels.push_back(std::move(level));
        }
    }
}
