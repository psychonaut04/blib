#include "App.h"
#include <glm/gtc/matrix_transform.hpp>
#include <blib/gl/Window.h>
#include <blib/gl/GlInitRegister.h>
#include <blib/gl/GlResizeRegister.h>
#include <blib/gl/Renderer.h>
#include <blib/util/Profiler.h>
#include <blib/RenderState.h>
#include <blib/MouseListener.h>
#include <blib/SpriteBatch.h>
#include <blib/util/Signal.h>
#include <blib/util/Mutex.h>
#include <blib/util/Log.h>
#include <blib/util/Semaphore.h>
#include <blib/util/Profiler.h>
#include <blib/SpriteBatch.h>
#include <blib/LineBatch.h>
#include <blib/Font.h>
#include <blib/util.h>
using blib::util::Log;

namespace blib
{

	void App::start(bool looping)
	{
		util::Profiler();
		semaphore = new util::Semaphore(0,2);
		updateThread = new UpdateThread(this);	//will create the window in the right thread
		updateThread->start();
		updateThread->semaphore->wait(); //wait until it is initialized
		renderThread = new RenderThread(this);
		renderThread->start();

		if(looping)
			run();
	}

	void App::run()
	{
		while(running)
		{
			step();
		}
	}


	void App::addKeyListener(KeyListener* keyListener)
	{
		window->addListener(keyListener);
	}
	void App::addMouseListener(MouseListener* mouseListener)
	{
		window->addListener(mouseListener);
	}


	void App::createWindow()
	{
		window = new blib::gl::Window();
		window->setSize(appSetup.width, appSetup.height);
		window->setBorder(appSetup.border);
		window->create();

		{
			for(int i = 0; i < 255; i++)
				keyState.pressedKeys[i] = false;
			class AppKeyListener : public KeyListener {
				App* app;
			public:
				AppKeyListener(App* app)			{			this->app = app;							}
				void onKeyDown( blib::Key key )		{			app->keyState.pressedKeys[key] = true;		}
				void onKeyUp( blib::Key key )		{			app->keyState.pressedKeys[key] = false;		}
			};
			addKeyListener(new AppKeyListener(this));
		}

		{
			class AppMouseListener : public MouseListener {
				App* app;
			public:
				AppMouseListener(App* app)						{	this->app = app;								}
				void onMouseDown(int x, int y, Button button)	{	app->mouseState.buttons[button] = true;	};
				void onMouseUp(int x, int y, Button button)		{	app->mouseState.buttons[button] = true;	};
				void onMouseMove(int x, int y, Buttons button)	{	app->mouseState.x = x; app->mouseState.y = y;};
			};
			addMouseListener(new AppMouseListener(this));
		}

		mouseState.leftButton = false;
		mouseState.rightButton = false;
		mouseState.middleButton = false;
		mouseState.x = 0;
		mouseState.y = 0;

		renderer = new gl::Renderer();
		renderState = RenderState::activeRenderState;
		spriteBatch = new SpriteBatch(renderer);
		lineBatch = new LineBatch(renderer);

		blib::gl::GlInitRegister::initRegisteredObjects();

		init();

		blib::gl::GlResizeRegister::ResizeRegisteredObjects(window->getWidth(), window->getHeight());
		wglMakeCurrent(NULL, NULL);
		frameTimeIndex = 0;
		running = true;
	}


	App::RenderThread::RenderThread( App* app ) : Thread("RenderThread")
	{
		this->app = app;
		semaphore = new blib::util::Semaphore(0,1);
	}
	App::UpdateThread::UpdateThread( App* app ) : Thread("UpdateThread")
	{
		this->app = app;
		semaphore = new blib::util::Semaphore(0,1);
	}


	void App::step()
	{
		renderer->swap();
		renderThread->semaphore->signal();
		updateThread->semaphore->signal();
		semaphore->wait();
		semaphore->wait();

		frameTimes[frameTimeIndex].drawTime = renderThread->frameTime;
		frameTimes[frameTimeIndex].updateTime = updateThread->frameTime;
		frameTimes[frameTimeIndex].fps = util::Profiler::fps;
		frameTimeIndex = (frameTimeIndex+1)%1000;
	}



	int App::RenderThread::run()
	{
		if(!wglMakeCurrent(app->window->hdc, ((blib::gl::Window*)app->window)->hrc))
		{
			Log::out<<"ERROR MAKING CURRENT"<<Log::newline;
			char* lpMsgBuf;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS, NULL,	GetLastError(),	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL );
			Log::out<<"Error: "<<lpMsgBuf<<Log::newline;
		}
		while(app->running)
		{
			semaphore->wait();
			if(!app->running)
				break;
			double frameStart = util::Profiler::getAppTime();
			app->renderer->flush();
			app->window->swapBuffers();
			frameTime = util::Profiler::getAppTime() - frameStart;
			app->semaphore->signal();
		}
		app->semaphore->signal();
		return 0;
	}

	int App::UpdateThread::run()
	{
		app->createWindow();

		Font* font = Font::getFontInstance("tahoma");

		semaphore->signal();
		while(app->running)
		{
			semaphore->wait();
			if(!app->running)
				break;
			double frameStart = util::Profiler::getAppTime();
			app->window->tick();
			double elapsedTime = blib::util::Profiler::getTime();
			blib::util::Profiler::startFrame();
			app->time += elapsedTime;
			app->update(elapsedTime);
			if(!app->running)
				break;
			app->draw();

			app->spriteBatch->begin();
			app->spriteBatch->draw(font, "FPS: " + util::toString(util::Profiler::fps), glm::mat4());
			app->spriteBatch->draw(font, "draw time",	glm::translate(glm::mat4(), glm::vec3(220, 20,0)), glm::vec4(1,0,0,1));
			app->spriteBatch->draw(font, "update time", glm::translate(glm::mat4(), glm::vec3(220, 40,0)), glm::vec4(0,1,0,1));
			app->spriteBatch->end();
			app->lineBatch->begin();
			app->lineBatch->draw(math::Rectangle(glm::vec2(20,20), 200,100), glm::vec4(1,1,1,1));

			PerformanceInfo minValues = { 99999999999, 99999999999, 99999999999 };
			PerformanceInfo maxValues = { -99999999999, -99999999999, -99999999999 };
			for(int i = 0; i < 1000; i++)
			{
				for(int ii = 0; ii < 3; ii++)
				{
					minValues.data[ii] = glm::min(minValues.data[ii], app->frameTimes[i].data[ii]);
					maxValues.data[ii] = glm::max(maxValues.data[ii], app->frameTimes[i].data[ii]);
				}
			}

			float timeFactor = 100 / glm::max(maxValues.updateTime, maxValues.drawTime);

			PerformanceInfo prevAccum = { 0, 0, 0 };
			PerformanceInfo accum = { 0, 0, 0 };
			for(int i = 0; i < 1000; i++)
			{
				for(int ii = 0; ii < 3; ii++)
					accum.data[ii] += app->frameTimes[i].data[ii];
				if(i%5 == 0 && i > 0)
				{
					for(int ii = 0; ii < 3; ii++)
						accum.data[ii] /= 5.0;

					if(i != 5)
					{
						app->lineBatch->draw(glm::vec2(19 + i*.2f, 120 - timeFactor*prevAccum.drawTime), glm::vec2(20 + i*.2f, 120 - timeFactor*accum.drawTime), glm::vec4(1,0,0,1));
						app->lineBatch->draw(glm::vec2(19 + i*.2f, 120 - timeFactor*prevAccum.updateTime), glm::vec2(20 + i*.2f, 120 - timeFactor*accum.updateTime), glm::vec4(0,1,0,1));
					}
					prevAccum = accum;
					ZeroMemory(&accum, sizeof(PerformanceInfo));
				}
			}

			app->lineBatch->end();



			frameTime = util::Profiler::getAppTime() - frameStart;
			app->semaphore->signal();
		}
		app->semaphore->signal();
		return 0;
	}


}