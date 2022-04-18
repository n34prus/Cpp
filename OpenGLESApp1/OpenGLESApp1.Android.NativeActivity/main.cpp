#include "customfig.h"
#include <malloc.h>
#include <GLES/gl.h>
#include <vector>
//#include "SOIL.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))

//этот кусок требует рефакторинга
int touched = 0;
float nx = 0;
float ny = 0;
float asx = 0;
float asy = 0;
//

/**
* Наши сохраненные данные состояния.
*/
struct saved_state {
	float axisAngle;
	int32_t x;
	int32_t y;
};
/**
* Общее состояние нашего приложения.
*/
struct engine {
	struct android_app* app;

	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	ASensorEventQueue* sensorEventQueue;

	int animating;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	int32_t width;
	int32_t height;
	struct saved_state state;
};

/**
* Инициализировать контекст EGL для текущего отображения.
*/
static int engine_init_display(struct engine* engine) {
	// инициализировать OpenGL ES и EGL

	/*
	* Здесь указываются атрибуты нужной конфигурации.
	* Ниже мы выбираем EGLConfig с не менее, чем 8 битами на компонент цвета,
	* совместимый с окном на экране
	*/
	const EGLint attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};
	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	/* Здесь приложение выбирает нужную ему конфигурацию. В данном
	* примере у нас очень упрощенный процесс выбора, когда выбирается
	* первый EGLConfig, который соответствуют нашим критериям */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID является атрибутом EGLConfig, который 
	* гарантированно принимается ANativeWindow_setBuffersGeometry().
	* Поскольку мы выбрали EGLConfig, можно безопасно изменить конфигурацию буферов
	* ANativeWindow, чтобы обеспечить соответствие с помощью EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
	context = eglCreateContext(display, config, NULL, NULL);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	engine->display = display;
	engine->context = context;
	engine->surface = surface;
	engine->width = w;
	engine->height = h;
	engine->state.axisAngle = 0;

	// Инициализировать состояние GL.
	GL::setupGL(w, h);

	return 0;
}

/**
* Просто текущий кадр в отображении.
*/

static void engine_draw_frame(struct engine* engine) {
	if (engine->display == NULL) {
		// Нет отображения.
		return;
	}

	GL::clearGL();

	eglSwapBuffers(engine->display, engine->surface);
}


/**
* Демонтировать контекст EGL, связанный в настоящее время с отображением.
*/
static void engine_term_display(struct engine* engine) {
	if (engine->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (engine->context != EGL_NO_CONTEXT) {
			eglDestroyContext(engine->display, engine->context);
		}
		if (engine->surface != EGL_NO_SURFACE) {
			eglDestroySurface(engine->display, engine->surface);
		}
		eglTerminate(engine->display);
	}
	engine->animating = 0;
	engine->display = EGL_NO_DISPLAY;
	engine->context = EGL_NO_CONTEXT;
	engine->surface = EGL_NO_SURFACE;

	//Cube_tearDownGL(); ???
}

/**
* Обработать следующее входное событие.
*/
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
	struct engine* engine = (struct engine*)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		engine->state.x = AMotionEvent_getX(event, 0);
		engine->state.y = AMotionEvent_getY(event, 0);
		//0.0 левый верхний угол
		if (!touched)
		{
			touched = 1;
			int _x = engine->state.x;
			int _y = engine->state.y;
			int _w = engine->width;
			int _h = engine->height;
			nx = (float(_x * 2 - _w))/_w;
			ny = -((float(_y * 2 - _h)) / _w);
		}
		return 1;
	}
	return 0;
}
/*
float* touchCoord(int _x, int _y)
{

}
*/

/**
* Обработать следующую основную команду.
*/
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
	struct engine* engine = (struct engine*)app->userData;
	switch (cmd) {
	case APP_CMD_SAVE_STATE:
		// Системы запросила сохранить текущее состояние. Сохраните его.
		engine->app->savedState = malloc(sizeof(struct saved_state));
		*((struct saved_state*)engine->app->savedState) = engine->state;
		engine->app->savedStateSize = sizeof(struct saved_state);
		break;
	case APP_CMD_INIT_WINDOW:
		// Окно показывается, подготовьте его.
		if (engine->app->window != NULL) {
			engine_init_display(engine);
			engine_draw_frame(engine);
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// Окно скрывается или закрывается, удалите его.
		engine_term_display(engine);
		break;
	case APP_CMD_GAINED_FOCUS:
		// При получении фокуса приложением мы начинаем мониторинг акселерометра.
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_enableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
			// Желательно получать 60 событий в секунду (в микросекундах).
			ASensorEventQueue_setEventRate(engine->sensorEventQueue,
				engine->accelerometerSensor, (1000L / 60) * 1000);
		}
		engine->animating = 1;
		break;
	case APP_CMD_LOST_FOCUS:
		// При потере фокуса приложением мы прекращаем мониторинг акселерометра.
		// Это помогает экономить заряд аккумулятора, когда устройство не используется.
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_disableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
		}
		// Также остановите анимацию.
		engine->animating = 0;
		engine_draw_frame(engine);
		break;
	}
}

/**
* Это главная точка входа приложения в машинном коде, которое использует
* android_native_app_glue. Оно выполняется в своем собственном потоке со своим собственным
* циклом обработки событий для получения входных событий и выполнения других действий.
*/
void android_main(struct android_app* state) {
	struct engine engine;

	memset(&engine, 0, sizeof(engine));
	state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	engine.app = state;

	// Подготовить мониторинг акселерометра
	engine.sensorManager = ASensorManager_getInstance();
	engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
		ASENSOR_TYPE_ACCELEROMETER);
	engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
		state->looper, LOOPER_ID_USER, NULL, NULL);

	if (state->savedState != NULL) {
		// Начинаем с предыдущего сохраненного состояния; восстанавливаем из него.
		engine.state = *(struct saved_state*)state->savedState;
	}

	engine.animating = 1;

	// цикл ожидает выполнения операций.

	//работает: список указателей на треугольники
	std::vector<Triangle*> triangles;
	//triangles.push_back(new Triangle((float[3]) { 0.f, 0.f, 0 }));
	//// position, angle, size
	Point* point1 = new Point((float[3]) { 0.f, -0.5f, 0 });
	//Triangle* triangle1 = new Triangle((float[3]) { 0.f, -0.f, -1.f });

	int width, height;
	//unsigned char* image = SOIL_load_image("container.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	
	while (1) {
		// Чтение всех отложенных событий.
		int ident;
		int events;
		struct android_poll_source* source;

		// Без анимации мы заблокируем навсегда ожидание событий.
		// При анимации мы создаем цикл до считывания всех событий, а затем продолжаем
		// рисовать следующий кадр анимации.
		while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
			(void**)&source)) >= 0) {

			// Обработать это событие.
			if (source != NULL) {
				source->process(state, source);
			}

			// Если у датчика есть данные, нужно обработать их сейчас.
			if (ident == LOOPER_ID_USER) {
				if (engine.accelerometerSensor != NULL) {
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
						&event, 1) > 0) {
						LOGI("accelerometer: x=%f y=%f z=%f",
							event.acceleration.x, event.acceleration.y,
							event.acceleration.z);
						asx = event.acceleration.x;
						asy = event.acceleration.y;
					}
				}
			}

			// Убедитесь, что выполняется выход.
			if (state->destroyRequested != 0) {
				engine_term_display(&engine);
				return;
			}
		}

		if (engine.animating && (engine.display != NULL)) {
			// Закончили с событиями. Можно рисовать следующий кадр анимации.
			if (touched)
			{
				triangles.push_back(new Triangle((float[3]) { nx, ny, 0 }, (float[3]) {0,0,0}, (float[3]) { 0.2, 0.2, 1 }));
				touched = 0;
			}
			if (!triangles.empty())
				for (int i = 0; i < triangles.size(); i++) {
					//triangles[i]->rotate(0,0,3);
					triangles[i]->gravity(-0.0001*asx, -0.0001*asy, -0.00001);
				}
			
			//triangle1->move(0.01f, 0, 0);
			//triangle1->move((float[3] ){ 0.01f, 0, 0 });
			//triangle2->rotate(0, 0,-3);
			//triangle2->shiftClr(1,0,-100,-100,0);

			//if (engine.state.angle > 1) {
			//	engine.state.angle = 0;
			//}

			// Рисование регулируется скоростью обновления экрана, поэтому
			// здесь не нужно выполнять синхронизацию.
			GL::clearGL();
			point1->show();
			//triangle1->show();
			if (!triangles.empty())
				//for (int i = triangles.size()-1; i >= 0; i--) {
				for (int i = 0; i < triangles.size(); i++) {
					Phys::areCollided(triangles[i], point1);
					triangles[i]->show();


					//удоляем потеряшек
					if (!(triangles[i]->isVisible()) ||
						(triangles[i]->lifetime > 500) ) {
						delete triangles[i];
						triangles.erase(triangles.begin() + i);
						//vector<int>(vec).swap(vec);
					}
				}
			eglSwapBuffers(engine.display, engine.surface);
			//engine_draw_frame(&engine);
			
		}
	}
}
