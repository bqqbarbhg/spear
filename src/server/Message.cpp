#include "Message.h"
#include "sf/Reflection.h"

#include "sp/Json.h"
#include "ext/json_input.h"
#include "ext/json_output.h"
#include "ext/sp_tools_common.h"

namespace sv {

sf::Box<Message> decodeMessage(sf::Slice<char> compressed)
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

	if (encoded[0] == '{') {
		jsi_args args = { };
		args.dialect.allow_comments = true;
		jsi_value *value = jsi_parse_memory(encoded.data, encoded.size, &args);
		if (!sp::readJson(value, msg)) {
			msg.reset();
		}
		jsi_free(value);
	} else if (encoded.size >= 8 && !memcmp(encoded.data, "sfbinv01", 8)) {
		sf::Slice<char> slice = encoded.drop(8);
		if (!sf::readBinary(slice, msg)) {
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

template<> void initType<sv::Message>(Type *t)
{
	static PolymorphType polys[] = {
		sf_poly(sv::Message, Join, sv::MessageJoin),
		sf_poly(sv::Message, Load, sv::MessageLoad),
		sf_poly(sv::Message, Update, sv::MessageUpdate),
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
	};
	sf_struct_base(t, sv::MessageJoin, sv::Message, fields);
}

template<> void initType<sv::MessageLoad>(Type *t)
{
	static Field fields[] = {
		sf_field(sv::MessageLoad, state),
		sf_field(sv::MessageLoad, sessionId),
		sf_field(sv::MessageLoad, sessionSecret),
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

}
