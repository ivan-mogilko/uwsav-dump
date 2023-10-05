#include <memory>
#include <stdint.h>
#include <string>
#include <vector>
#include "utils/filestream.h"
#include "utils/stream.h"
#include "utils/str_utils.h"

void write_text(Stream &out, const std::string &s)
{
    out.Write(s.c_str(), s.size());
}

void write_text(Stream &out, const char *cstr)
{
    size_t len = strlen(cstr);
    out.Write(cstr, len);
}

void write_text_ln(Stream &out, const std::string &s)
{
    out.Write(s.c_str(), s.size());
    out.Write("\n", 1);
}

void write_text_ln(Stream &out, const char *cstr)
{
    size_t len = strlen(cstr);
    out.Write(cstr, len);
    out.Write("\n", 1);
}

/*

http://bootstrike.com/Ultima/Online/uwformat.php
https://wiki.ultimacodex.com/wiki/Ultima_Underworld_internal_formats

*/

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

struct TileInfo
{
    uint16_t data1 = 0u;
    uint16_t data2 = 0u;

    uint16_t GetFirstObjectIndex() const
    {
        return (data2 >> 6) & 0x3FF;
    }
};

struct ObjectInfo
{
    // basic info
    uint16_t data1 = 0u;
    uint16_t data2 = 0u;
    uint16_t data3 = 0u;
    uint16_t data4 = 0u;
    // mobile info ...

    uint16_t GetItemID() const
    {
        return (data1 & 0x1FF);
    }

    uint16_t GetFlags() const
    {
        return (data1 >> 9) & 0xF;
    }

    uint16_t GetNextObjectIndex() const
    {
        return (data3 >> 6) & 0x3FF;
    }

    uint16_t GetSpecial() const
    {
        return (data4 >> 6) & 0x3FF;
    }
};

struct LevelInfo
{
    std::vector<TileInfo> tiles;
    std::vector<ObjectInfo> objs;
};

void read_tilemap_objlist(Stream &in, LevelInfo &levelinfo)
{
    /*
       Each underworld level consists of a 64x64 tile map (just like on a chess
       board). A tile can be of different types and can have various floor
       heights. The ceiling height is fixed. A tile can have an index
       into the master object list that is the start of an object chain with
       objects in this tile.

       The first 0x4000 bytes of each "level tilemap/master object list" contain
       the tilemap info bytes. For each tile there are two Int16 that describe a
       tile's properties. The map's origin is at the lower left tile, going to the
       right, each line in turn.

       The two Int16 values can be split into bits:

       0000 tile properties / flags:

          bits     len  description
           0- 3    4    tile type (0-9, see below)
           4- 7    4    floor height
           8       1    unknown (?? special light feature ??) always 0 in uw1
           9       1    0, never used in uw1
          10-13    4    floor texture index (into texture mapping)
          14       1    when set, no magic is allowed to cast/to be casted upon
          15       1    door bit (when 1, a door is present)

       0002 tile properties 2 / object list link

         bits     len  description
          0- 5    6    wall texture index (into texture mapping)
          6-15    10   first object in tile (index into master object list) 
    */
    const uint16_t tile_num = 64 * 64;
    std::vector<TileInfo> tiles(tile_num);
    for (uint16_t i = 0; i < tile_num; ++i)
    {
        tiles[i].data1 = in.ReadInt16LE();
        tiles[i].data2 = in.ReadInt16LE();
    }
    
    // mobile object information (objects 0000-00ff, 256 x 27 bytes)
    // static object information (objects 0100-03ff, 768 x 8 bytes)
    /*
        The "general object info" block looks as following:

            bits  size  field      description

       0000 objid / flags
            0- 8   9   "item_id"   Object ID (see below)
            9-12   4   "flags"     Flags
              12   1   "enchant"   Enchantment flag (enchantable objects only)
              13   1   "doordir"   Direction flag (doors)
              14   1   "invis"     Invisible flag (don't draw this object)
              15   1   "is_quant"  Quantity flag (link field is quantity/special)

       0002 position
            0- 6   7   "zpos"      Object Z position (0-127)
            7- 9   3   "heading"   Heading (*45 deg)
           10-12   3   "ypos"      Object Y position (0-7)
           13-15   3   "xpos"      Object X position (0-7)

       0004 quality / chain
            0- 5   6   "quality"   Quality
            6-15   10  "next"      Index of next object in chain

       0006 link / special
            0- 5   6   "owner"     Owner / special
            6-15   10  (*)         Quantity / special link / special property
    */
    const uint16_t mob_obj_num = 256;
    const uint16_t sta_obj_num = 768;
    std::vector<ObjectInfo> objs(mob_obj_num + sta_obj_num);
    for (uint16_t i = 0; i < mob_obj_num; ++i)
    {
        objs[i].data1 = in.ReadInt16LE();
        objs[i].data2 = in.ReadInt16LE();
        objs[i].data3 = in.ReadInt16LE();
        objs[i].data4 = in.ReadInt16LE();
        // mobile info
        in.Seek(19, kSeekCurrent);
    }
    for (uint16_t i = mob_obj_num; i < mob_obj_num + sta_obj_num; ++i)
    {
        objs[i].data1 = in.ReadInt16LE();
        objs[i].data2 = in.ReadInt16LE();
        objs[i].data3 = in.ReadInt16LE();
        objs[i].data4 = in.ReadInt16LE();
    }

    levelinfo.tiles = std::move(tiles);
    levelinfo.objs = std::move(objs);
}

void read_uwsav(Stream &in, std::vector<LevelInfo> &levels)
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
            

    // TODO: an option telling what we read, uw1 or uw2 etc
    // UW-1: 135 blocks, 9 lvls x 15, where
    /*
        <9 blocks level tilemap/master object list>
        <9 blocks object animation overlay info>
        <9 blocks texture mapping>
        <9 blocks automap infos>
        <9 blocks map notes>

        The remaining 9 x 10 blocks are unused.
    */
    //const uint16_t blk_per_level = 15;
    //for (uint8_t level = 0; level < 9; ++level)
    uint8_t level_id = 0;
    for (uint16_t blk_index = 0; blk_index < blocks.size(); ++blk_index)
    {
        if (blocks[blk_index].size != 31752) // tiles + objlists
            continue;

        in.Seek(blocks[blk_index].offset, kSeekBegin);
        LevelInfo levelinfo;
        read_tilemap_objlist(in, levelinfo);
        levels.push_back(std::move(levelinfo));
    }
}

void print_tilemap(Stream &out, const LevelInfo &level)
{

}

void print_objlist(Stream &out, const LevelInfo &level)
{
    // print object lists per tile
    // 64x64
    const uint16_t levelwidth = 64;
    const uint16_t levelheight = 64;
    for (uint16_t tileid = 0; tileid < level.tiles.size(); ++tileid)
    {
        // ???
        uint16_t tiley = tileid / levelheight;
        uint16_t tilex = tileid - tiley * levelheight;
        uint16_t obj_index = level.tiles[tileid].GetFirstObjectIndex();
        if (obj_index == 0)
            continue;
        std::string s = StrPrint("    T [%02dx%02d]: ", tilex, tiley);
        write_text(out, s);
        while (obj_index > 0)
        {
            const ObjectInfo &obj = level.objs[obj_index];
            uint16_t item_id = obj.GetItemID();
            std::string s = StrPrint("0x%x (0x%x, %d)", item_id, obj.GetFlags(), obj.GetSpecial());
            write_text(out, s);
            write_text(out, "    ");
            uint16_t next_index = obj.GetNextObjectIndex();
            // assertion, bug?
            if (next_index == obj_index)
                break;
            obj_index = next_index;
        }
        write_text(out, "\n");
    }
}

void print_levels(Stream &out, const std::vector<LevelInfo> &levels)
{
    for (size_t i = 0; i < levels.size(); ++i)
    {
        write_text_ln(out, "==========================================");

        std::string s = StrPrint(" Level %d", i + 1);
        write_text_ln(out, s);

        print_tilemap(out, levels[i]);
        print_objlist(out, levels[i]);

        write_text_ln(out, "==========================================");
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return -1;

    const char *in_filename = argv[1];
    const char *out_filename = (argc > 2) ? argv[2] : nullptr;
    std::vector<LevelInfo> levels;

    {
        Stream in(FileStream::TryOpen(in_filename, kFileMode_Open, kStream_Read));
        if (in)
        {
            read_uwsav(in, levels);
        }
    }

    if (out_filename)
    {
        Stream out(FileStream::TryOpen(out_filename, kFileMode_CreateAlways, kStream_Write));
        if (out)
        {
            print_levels(out, levels);
        }
    }
    return 0;
}
