#include "Message.h"
#include "sf/Reflection.h"

#include "sp/Json.h"
#include "ext/json_input.h"
#include "ext/json_output.h"
#include "ext/sp_tools_common.h"

namespace sv {

sf::Box<Message> decodeMessage(sf::Slice<char> compressed, const MessageDecodingLimits &limits)
{
	if (compressed.size == 0) return { };

	sf::Box<sv::Message> msg;

	sf::SmallArray<char, 4096> buffer;
	sf::Slice<char> encoded = compressed;
	if (compressed.size >= 8 && !memcmp(compressed.data, "zstd", 4)) {
		uint32_t msgSize = *(const uint32_t*)(compressed.data + 4);
		buffer.resizeUninit(msgSize);
		size_t size = sp_decompress_buffer(SP_COMPRESSION_ZSTD, buffer.data, buffer.size, compressed.data + 8, compressed.size - 8);
		if (size != msgSize) return { };
		encoded = buffer;
	}

	if (encoded.size < 2 || encoded.size > limits.maxDataSize) {
		return msg;
	}

	if (encoded[0] == '{') {
		jsi_args args = { };
		args.dialect.allow_comments = true;
		jsi_value *value = jsi_parse_memory(encoded.data, encoded.size, &args);
		if (!value || !sp::readJson(value, msg)) {
			msg.reset();
		}
		jsi_free(value);
	} else if (limits.allowBinary && encoded.size >= 8 && !memcmp(encoded.data, "sfbinv01", 8)) {
		sf::Slice<const char> slice = encoded.drop(8);
		if ( !sf::readBinary(slice, msg)) {
			msg.reset();
		}
	}

	return msg;
}

void encodeMessage(sf::Array<char> &data, const Message &message, const MessageEncoding &encoding)
{
	Message *msgPtr = (Message*)&message;
	sf::SmallArray<char, 4096> encoded;

	sf::Array<char> &dst = encoding.compressionLevel > 0 ? encoded : data;

	if (encoding.binary) {
		dst.push("sfbinv01", 8);
		sf::writeBinary(dst, msgPtr);
	} else {
		jso_stream s;
		sp::jsoInitArray(&s, dst);
		sp::writeJson(s, msgPtr);
		jso_close(&s);
	}

	if (encoding.compressionLevel > 0) {
		size_t bound = sp_get_compression_bound(SP_COMPRESSION_ZSTD, dst.size);
		uint32_t begin = data.size;
		data.push("zstd", 4);
		uint32_t uncompressedSize = (uint32_t)dst.size;
		data.push((char*)&uncompressedSize, 4);
		char *dst = data.pushUninit(bound);
		size_t size = sp_compress_buffer(SP_COMPRESSION_ZSTD, dst, bound, encoded.data, encoded.size, encoding.compressionLevel);
		data.resizeUninit(begin + 8 + size);
	}
}

}

namespace sf {

template<> void initType<sv::QueryFile>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::QueryFile, name),
	};
	sf_struct(t, sv::QueryFile, fields);
}

template<> void initType<sv::QueryDir>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::QueryDir, name),
		sf_field(sv::QueryDir, dirs),
		sf_field(sv::QueryDir, files),
	};
	sf_struct(t, sv::QueryDir, fields);
}

template<> void initType<sv::Message>(Type *t)
{
	static PolymorphType polys[] = {
		sf_poly(sv::Message, Join, sv::MessageJoin),
		sf_poly(sv::Message, Load, sv::MessageLoad),
		sf_poly(sv::Message, Update, sv::MessageUpdate),
		sf_poly(sv::Message, RequestEdit, sv::MessageRequestEdit),
		sf_poly(sv::Message, RequestEditUndo, sv::MessageRequestEditUndo),
		sf_poly(sv::Message, RequestEditRedo, sv::MessageRequestEditRedo),
		sf_poly(sv::Message, RequestReplayBegin, sv::MessageRequestReplayBegin),
		sf_poly(sv::Message, RequestReplayReplay, sv::MessageRequestReplayReplay),
		sf_poly(sv::Message, RequestAction, sv::MessageRequestAction),
		sf_poly(sv::Message, QueryFiles, sv::MessageQueryFiles),
		sf_poly(sv::Message, QueryFilesResult, sv::MessageQueryFilesResult),
		sf_poly(sv::Message, ErrorList, sv::MessageErrorList),
	};
	sf_struct_poly(t, sv::Message, type, { }, polys);
}

template<> void initType<sv::MessageJoin>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageJoin, sessionId),
		sf_field(sv::MessageJoin, sessionSecret),
		sf_field(sv::MessageJoin, playerId),
		sf_field(sv::MessageJoin, name),
		sf_field(sv::MessageJoin, editPath),
	};
	sf_struct_base(t, sv::MessageJoin, sv::Message, fields);
}

template<> void initType<sv::MessageLoad>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageLoad, state),
		sf_field(sv::MessageLoad, sessionId),
		sf_field(sv::MessageLoad, sessionSecret),
		sf_field(sv::MessageLoad, clientId),
		sf_field(sv::MessageLoad, editPath),
	};
	sf_struct_base(t, sv::MessageLoad, sv::Message, fields);
}

template<> void initType<sv::MessageUpdate>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageUpdate, events),
	};
	sf_struct_base(t, sv::MessageUpdate, sv::Message, fields);
}

template<> void initType<sv::MessageRequestEdit>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageRequestEdit, edits),
	};
	sf_struct_base(t, sv::MessageRequestEdit, sv::Message, fields);
}

template<> void initType<sv::MessageRequestEditUndo>(Type *t)
{
	sf_struct_base(t, sv::MessageRequestEditUndo, sv::Message, { });
}

template<> void initType<sv::MessageRequestEditRedo>(Type *t)
{
	sf_struct_base(t, sv::MessageRequestEditRedo, sv::Message, { });
}

template<> void initType<sv::MessageRequestReplayBegin>(Type *t)
{
	sf_struct_base(t, sv::MessageRequestReplayBegin, sv::Message, { });
}

template<> void initType<sv::MessageRequestReplayReplay>(Type *t)
{
	sf_struct_base(t, sv::MessageRequestReplayReplay, sv::Message, { });
}

template<> void initType<sv::MessageRequestAction>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageRequestAction, action),
	};
	sf_struct_base(t, sv::MessageRequestAction, sv::Message, fields);
}

template<> void initType<sv::MessageQueryFiles>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageQueryFiles, root),
	};
	sf_struct_base(t, sv::MessageQueryFiles, sv::Message, fields);
}

template<> void initType<sv::MessageQueryFilesResult>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageQueryFilesResult, root),
		sf_field(sv::MessageQueryFilesResult, dir),
	};
	sf_struct_base(t, sv::MessageQueryFilesResult, sv::Message, fields);
}

template<> void initType<sv::MessageErrorList>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageErrorList, errors),
	};
	sf_struct_base(t, sv::MessageErrorList, sv::Message, fields);
}

}
