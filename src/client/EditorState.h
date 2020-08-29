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

struct EditorInput
{
	uint32_t totalErrors;
	sf::Slice<const sf::StringBuf> errors;
};

EditorState *editorCreate(const sf::Box<sv::ServerState> &svState, const sf::Box<cl::ClientState> &clState);
void editorFree(EditorState *es);

void editorPeekSokolEvent(EditorState *es, const struct sapp_event *e);

bool editorPeekEventPre(EditorState *es, const sf::Box<sv::Event> &event);
void editorAddQueryDir(EditorState *es, const sf::StringBuf &root, const sv::QueryDir &dir);

void editorPreRefresh(EditorState *es);
void editorPostRefresh(EditorState *es, const sf::Box<cl::ClientState> &clState);

void editorUpdate(EditorState *es, const FrameArgs &frameArgs, const ClientInput &input, const EditorInput &editorInput);

void editorHandleGui(EditorState *es, const GuiArgs &guiArgs);

EditorRequests &editorPendingRequests(EditorState *es);

}
