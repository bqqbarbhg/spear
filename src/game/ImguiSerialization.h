#pragma once

#include "sf/Base.h"
#include "sf/String.h"

struct ImguiStatus
{
	bool modified = false;
	bool changed = false;
};

typedef bool (*ImguiCallback)(void *user, ImguiStatus &status, void *inst, sf::Type *type, const sf::CString &label, sf::Type *parentType);

void handleInstCopyPasteImgui(ImguiStatus &status, void *inst, sf::Type *type, const char *id=nullptr);
void handleFieldsImgui(ImguiStatus &status, void *inst, sf::Type *type, ImguiCallback callback, void *user);
void handleInstImgui(ImguiStatus &status, void *inst, sf::Type *type, const sf::CString &label, sf::Type *parentType, ImguiCallback callback, void *user);

template <typename T>
void handleImgui(ImguiStatus &status, T &t, sf::String label, ImguiCallback callback=NULL, void *user=NULL)
{
	sf::SmallStringBuf<128> localLabel;
	localLabel.append(label);
	return handleInstImgui(status, &t, sf::typeOf<T>(), localLabel, callback, user);
}

template <typename T>
ImguiStatus handleImgui(T &t, sf::String label, ImguiCallback callback=NULL, void *user=NULL)
{
	ImguiStatus s;
	handleImgui(s, t, label, callback, user);
	return s;
}
