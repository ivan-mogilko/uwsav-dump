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

// Prints tilemap in ASCII
void print_tilemap(Stream &out, const LevelData &level)
{
    write_text_ln(out, "--------------------------------------------------------------------");
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

void print_objlinkedlist(Stream &out, const LevelData &level,
    uint16_t obj_index, const std::string &indent)
{
    std::string line;
    std::vector<uint16_t> containers;
    while (obj_index > 0)
    {
        if (line.size() >= 80)
        {
            write_text_ln(out, line);
            line = indent + ">>  ";
        }

        const ObjectData& obj = level.objs[obj_index];
        // NPCs or containers: save for later
        if ((obj.ItemID >= 0x0040 && obj.ItemID <= 0x007f) ||
            (obj.ItemID >= 0x0080 && obj.ItemID <= 0x008f))
        {
            containers.push_back(obj_index);
        }
        else
        {
            std::string s = StrPrint(" 0x%03x", obj.ItemID);
            if (obj.Quantity > 1)
                s.append(StrPrint(" (*%03u) |", obj.Quantity));
            else
                s.append("        |");
            line.append(s);
        }

        uint16_t next_index = obj.NextObjLink;
        if (next_index == obj_index)
            break; // safety skip, prevent endless loop
        obj_index = next_index;
    }
    write_text_ln(out, line);

    for (auto cont_index : containers)
    {
        const ObjectData &obj = level.objs[cont_index];
        const char *tag = (obj.ItemID >= 0x0040 && obj.ItemID <= 0x007f) ? "npc" : "inv";
        const char *has_inv = (obj.SpecialLink > 0) ? "+" : "-";
        const char *has_inv2 = (obj.SpecialLink > 0) ? ":" : " ";
        write_text(out, indent + ">>  " + StrPrint(" 0x%03x (%s%s)%s ", obj.ItemID, has_inv, tag, has_inv2));
        if (obj.SpecialLink > 0)
            print_objlinkedlist(out, level, obj.SpecialLink, indent + "               ");
        else
            write_text_ln(out, "");
    }
}

// Prints master objects list
void print_objlist(Stream &out, const LevelData &level)
{
    write_text_ln(out, "--------------------------------------------------------------------");
    write_text_ln(out, "  Objects in Tiles");

    for (uint16_t y = 0; y < level.Height; ++y)
    {
        for (uint16_t x = 0; x < level.Width; ++x)
        {
            const TileData& tile = level.tiles[y * level.Width + x];
            uint16_t obj_index = tile.FirstObjLink;
            if (obj_index == 0)
                continue;

            write_text(out, StrPrint(" T [%02dx%02d]: ", x, y));
            print_objlinkedlist(out, level, obj_index, "        ");
        }
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
