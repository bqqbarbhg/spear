#pragma once

#include "client/ClientState.h"
#include "client/Client.h"
#include "server/ServerState.h"
#include "sf/Array.h"

namespace cl {

struct EditorState;

EditorState *editorCreate(const sf::Box<sv::ServerState> &svState, const sf::Box<cl::ClientState> &clState);
void editorFree(EditorState *es);

void editorUpdate(EditorState *es, const FrameArgs &frameArgs, const ClientInput &input);

sf::Array<sf::Array<sf::Box<sv::Edit>>> &editorPendingEdits(EditorState *es);

}
