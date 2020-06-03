#pragma once

#include "sf/Base.h"
#include "sf/String.h"

typedef bool (*ImguiCallback)(void *user, bool &changed, void *inst, sf::Type *type, const sf::CString &label);

bool handleFieldsImgui(void *inst, sf::Type *type, ImguiCallback callback, void *user);
bool handleInstImgui(void *inst, sf::Type *type, const sf::CString &label, ImguiCallback callback, void *user);

template <typename T>
bool handleImgui(T &t, sf::String label, ImguiCallback callback=NULL, void *user=NULL)
{
	sf::SmallStringBuf<128> localLabel;
	localLabel.append(label);
	return handleInstImgui(&t, sf::typeOf<T>(), localLabel, callback, user);
}
