#pragma once

#include "client/ClientState.h"
#include "client/Client.h"
#include "server/ServerState.h"
#include "server/Message.h"
#include "sf/Array.h"

namespace cl {

struct EditorState;

struct EditorRequests
{
	sf::Array<sf::Array<sf::Box<sv::Edit>>> edits;
	bool undo = false;
	bool redo = false;
	sf::Array<sf::StringBuf> queryDirs;
};


EditorState *editorCreate(const sf::Box<sv::ServerState> &svState, const sf::Box<cl::ClientState> &clState);
void editorFree(EditorState *es);

bool editorPeekEventPre(EditorState *es, const sv::Event &event);
void editorAddQueryDir(EditorState *es, const sf::StringBuf &root, const sv::QueryDir &dir);

void editorUpdate(EditorState *es, const FrameArgs &frameArgs, const ClientInput &input);

EditorRequests &editorPendingRequests(EditorState *es);

}
