#include <memory>
#include <stdint.h>
#include <string>
#include <string.h>
#include <vector>
#include "uwsav/uwsav_data.h"
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

enum TileGlyphExtra
{
    kTileExtraDoor = kTileSlopeW + 1,
    kTileExtraUnknown
};

void print_tilemap(Stream &out, const LevelData &level)
{
    write_text_ln(out, "   0000000000111111111122222222223333333333444444444455555555556666");
    write_text_ln(out, "   0123456789012345678901234567890123456789012345678901234567890123");
    write_text_ln(out, "  -----------------------------------------------------------------");

    const char* tile_glyph[] = { "X", " ", "p", "q", "b", "d", " ", " ", " ", " ", "=", "?" };

    std::string line;
    for (uint16_t y = 0; y < level.Height; ++y)
    {
        uint16_t uw_y = level.Height - y - 1; // y axis is inverse
        write_text(out, StrPrint("%02d|", uw_y));
        line.clear();
        for (uint16_t x = 0; x < level.Width; ++x)
        {
            const TileData& tile = level.tiles[uw_y * level.Width + x];
            if (tile.IsDoor)
                line.append(tile_glyph[kTileExtraDoor]);
            else if (tile.Type >= kTileSolid && tile.Type <= kTileSlopeW)
                line.append(tile_glyph[tile.Type]);
            else
                line.append(tile_glyph[kTileExtraUnknown]);
        }
        line.append("|");
        write_text_ln(out, line);
    }

    write_text_ln(out, "  -----------------------------------------------------------------");
}

void print_objlist(Stream &out, const LevelData &level)
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
        uint16_t obj_index = level.tiles[tileid].FirstObjLink;
        if (obj_index == 0)
            continue;
        std::string s = StrPrint("    T [%02dx%02d]: ", tilex, tiley);
        write_text(out, s);
        while (obj_index > 0)
        {
            const ObjectData &obj = level.objs[obj_index];
            std::string s = StrPrint("0x%x (0x%x, %d)", obj.ItemID, obj.Flags, obj.Special);
            write_text(out, s);
            write_text(out, "    ");
            uint16_t next_index = obj.NextObjLink;
            // assertion, bug?
            if (next_index == obj_index)
                break;
            obj_index = next_index;
        }
        write_text(out, "\n");
    }
}

void print_levels(Stream &out, const std::vector<LevelData> &levels)
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
    std::vector<LevelData> levels;

    {
        Stream in(FileStream::TryOpen(in_filename, kFileMode_Open, kStream_Read));
        if (in)
        {
            ReadLevels(in, levels);
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
