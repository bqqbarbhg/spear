#include "Command.h"
#include "sf/Reflection.h"

namespace sf {

template<> void initType<sv::RawTileInfo>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::RawTileInfo, position),
		sf_field(sv::RawTileInfo, tileId),
	};
	sf_struct_base(t, sv::RawTileInfo, sv::Event, fields);
}

template<> void initType<sv::Command>(Type *t)
{
	static PolymorphType polys[] = {
		sf_poly(sv::Command, SetTiles, sv::CommandSetTiles),
		sf_poly(sv::Command, SetTilesRaw, sv::CommandSetTilesRaw),
		sf_poly(sv::Command, Undo, sv::CommandUndo),
		sf_poly(sv::Command, Redo, sv::CommandRedo),
	};
	sf_struct_poly(t, sv::Command, type, { }, polys);
}

template<> void initType<sv::CommandSetTiles>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CommandSetTiles, tileType),
		sf_field(sv::CommandSetTiles, tiles),
	};
	sf_struct_base(t, sv::CommandSetTiles, sv::Event, fields);
}

template<> void initType<sv::CommandSetTilesRaw>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::CommandSetTilesRaw, tiles),
	};
	sf_struct_base(t, sv::CommandSetTilesRaw, sv::Event, fields);
}

template<> void initType<sv::CommandUndo>(Type *t)
{
	sf_struct_base(t, sv::CommandUndo, sv::Event, { });
}

template<> void initType<sv::CommandRedo>(Type *t)
{
	sf_struct_base(t, sv::CommandRedo, sv::Event, { });
}

}
