#include "WidgetCharacter.h"

#include "client/gui/GuiResources.h"

namespace cl { namespace gui {

void WidgetCharacter::layout(GuiLayout &layout, const sf::Vec2 &min, const sf::Vec2 &max)
{
	WidgetBase::layout(layout, min, max);

	clicked = false;

	if (currentHealth != prevCurrentHealth || maxHealth != prevMaxHealth) {
		if (prevMaxHealth < 0) {
			targetHealth = (float)currentHealth;
			smoothHealth = (float)currentHealth;
		}
		prevCurrentHealth = currentHealth;
		prevMaxHealth = maxHealth;
		healthStr.clear();
		healthStr.format("%d/%d", sf::max(currentHealth, 0), sf::max(maxHealth, 0));
		targetTimer = 0.0f;
	}

	targetTimer += layout.dt;
	if (targetTimer >= 2.0f) {
		targetHealth = (float)currentHealth;
	}

	lerpLinear(smoothHealth, targetHealth, (float)maxHealth, layout.dt);
}

void WidgetCharacter::paint(GuiPaint &paint)
{
	sf::Vec2 size = layoutSize;

	sf::Vec2 iconSize = sf::Vec2(size.y);
	paint.canvas->draw(paint.resources->statusIconOutline, layoutOffset, iconSize);
	paint.canvas->draw(icon, layoutOffset, iconSize);

	sf::Vec2 healthOffset = layoutOffset + sf::Vec2(iconSize.x * 1.1f, size.y * 0.15f);
	sf::Vec2 healthSize = sf::Vec2(size.x - healthOffset.x, size.y * 0.25f);

	{
		sp::SpriteDraw draw;
		draw.transform.m00 = healthSize.x;
		draw.transform.m11 = healthSize.y;
		draw.transform.m02 = healthOffset.x;
		draw.transform.m12 = healthOffset.y;

		if (maxHealth > 0) {
			float rcpMax = 1.0f / (float)maxHealth;
			float relHealth = smoothHealth * rcpMax;

			float curHealth = (float)currentHealth;
			float minHealth = sf::min(smoothHealth, curHealth);
			float maxHealth = sf::max(smoothHealth, curHealth);

			sp::SpriteDraw barDraw = draw;
			barDraw.cropMax.x = minHealth * rcpMax;
			barDraw.sprite = paint.resources->statusHealthFilling;
			paint.canvas->draw(barDraw);

			if (maxHealth > minHealth) {
				barDraw.cropMin.x = minHealth * rcpMax;
				barDraw.cropMax.x = maxHealth * rcpMax;
				barDraw.color = curHealth < smoothHealth ? sf::Vec4(1.0f, 0.0f, 0.0f, 1.0f) : sf::Vec4(0.0f, 0.0f, 1.0f, 1.0f);
				paint.canvas->draw(barDraw);
			}
		}

		draw.sprite = paint.resources->statusHealthOutline;
		paint.canvas->draw(draw);
	}

	{
		sp::TextDraw draw;
		draw.font = paint.resources->statusFont;
		draw.string = healthStr;
		draw.transform = sf::mat2D::translate(healthOffset + sf::Vec2(size.y * 0.05f, size.y * 0.55f));
		draw.color = sf::Vec4(1.0f);
		draw.height = size.y * 0.4f;
		paint.canvas->drawText(draw);
	}
}

bool WidgetCharacter::onPointer(GuiPointer &pointer)
{
	pointer.blocked = true;

	if (pointer.action == GuiPointer::Tap && (pointer.button == GuiPointer::MouseLeft || pointer.button == GuiPointer::Touch)) {
		clicked = true;
	}

	return false;
}

} }
