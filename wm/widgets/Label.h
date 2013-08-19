#pragma once

#include "Widget.h"

#include <string>

namespace blib
{
	namespace wm
	{
		namespace widgets
		{
			class Label : public Widget
			{
			public:
				std::string text;
				Label();

				virtual void draw(gl::SpriteBatch &spriteBatch, glm::mat4 matrix);
			};
		}
	}
}