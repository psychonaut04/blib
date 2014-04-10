#pragma once

#include <stdlib.h>

namespace blib
{
	class Shader;
	class Texture;
	class FBO;
	class VBO;
	class VIO;

	class RenderState
	{
	public:
		Shader* activeShader;
		Texture* activeTexture[10];
		FBO* activeFbo;
		VBO* activeVbo;
		VIO* activeVio;

		//blending
		bool blendEnabled;
		enum BlendOptions
		{
			ZERO,
			ONE,
			SRC_ALPHA,
			ONE_MINUS_SRC_ALPHA,
		};
		BlendOptions srcBlendColor;
		BlendOptions dstBlendColor;
		BlendOptions srcBlendAlpha;
		BlendOptions dstBlendAlpha;

		enum RenderStyle
		{
			WIREFRAME,
			FILLED,
		};
		RenderStyle renderStyle;

		bool stencilTestEnabled;
		bool stencilWrite;

		bool scissor;
		int scissorArea[4];


		bool depthTest;

		RenderState()
		{
			activeShader = NULL;
			for(int i = 0; i < 10; i++)
				activeTexture[i] = NULL;
			activeVbo = NULL;
			activeVio = NULL;
			activeFbo = NULL;

			blendEnabled = false;
			srcBlendColor = SRC_ALPHA;
			dstBlendColor = ONE_MINUS_SRC_ALPHA;
			srcBlendAlpha = SRC_ALPHA;
			dstBlendAlpha = ONE_MINUS_SRC_ALPHA;

			stencilTestEnabled = false;

			renderStyle = FILLED;
			depthTest = false;
			scissor = false;
		}
	};


}