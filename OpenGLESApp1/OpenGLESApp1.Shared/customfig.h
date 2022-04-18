#ifndef __CustomFigureNative__main__
#define __CustomFigureNative__main__

#include <stdio.h>
#include <cmath>
#include <time.h>
#ifdef __ANDROID__
#include <GLES/gl.h>
#elif __APPLE__
#include <OpenGLES/ES1/gl.h>
#endif

class GL {
public:
    static void setupGL(double _width, double _height)
    {
        // Initialize GL state.
        glDisable(GL_DITHER);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glEnable(GL_CULL_FACE);
        glShadeModel(GL_SMOOTH);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glViewport(0, 0, _width, _height);
        GLfloat ratio = (GLfloat)_width / _height;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustumf(-ratio, ratio, -1, 1, 1, 10);
    }
    static void clearGL() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
};

class Particle {
protected:
    GLint center[3] = { 0x00000,  0x00000,  0x00000 };
    enum Ftype { POINT = 1, LINE, TRIANGLE, SQUERE, ROUND };
public:
    float axisShift[3] = { 0.f, 0.f, 0.f };
    float axisAngle[3] = { 0.f, 0.f, 0.f };
    float axisScale[3] = { 1.f, 1.f, 1.f };
    float speed[3] = { 0, 0, 0 };
    float axel[3] = { 0, 0, 0 };
    uint32_t lifetime = 0;
    //по умолчанию создаетс€ фигура в центре экрана
    Particle() { }
    //есть смещение
    Particle(float _axisShift[3]) {
        move(_axisShift);
    }
    //смещение + поворот
    Particle(float _axisShift[3], float _axisAngle[3]) {
        move(_axisShift);
        rotate(_axisAngle);
    }
    //смещение + поворот + масштаб
    Particle(float _axisShift[3], float _axisAngle[3], float _axisScale[3]) {
        move(_axisShift);
        rotate(_axisAngle);
        resize(_axisScale);
    }
    ~Particle(){}
    void setCenter(GLint _center[3]) {
        for (int i = 0; i < 3; i++) center[i] = _center[i];
        return;
    }

    void move(float _dx, float _dy, float _dz) {
        axisShift[0] += _dx;
        axisShift[1] += _dy;
        axisShift[2] += _dz;
    }
    //example move((float[3] ){ 0.01f, 0, 0 });
    void move(float _axisShift[3]) {
        for (int i = 0; i < 3; i++) {
            axisShift[i] += _axisShift[i];
        }
    }
    void rotate(float _dx, float _dy, float _dz) {
        axisAngle[0] += _dx;
        axisAngle[1] += _dy;
        axisAngle[2] += _dz;
    }
    void rotate(float _axisAngle[3]) {
        for (int i = 0; i < 3; i++) {
            axisAngle[i] += _axisAngle[i];
        }
    }
    void setAngle(float _axisAngle[3]) {
        for (int i = 0; i < 3; i++) {
            axisAngle[i] = _axisAngle[i];
        }
    }
    void resize(float _dx, float _dy, float _dz) {
        axisScale[0] *= _dx;
        axisScale[1] *= _dy;
        axisScale[2] *= _dz;
    }
    void resize(float _axisScale[3]) {
        for (int i = 0; i < 3; i++) {
            axisScale[i] *= _axisScale[i];
        }
    }
    void gravity(float _gx, float _gy, float _gz) {
        axel[0] = _gx;
        axel[1]= _gy;
        axel[2] = _gz;
    }
    void applyShift() {
        for (int i = 0; i < 3; i++)
        {
            axisShift[i] += speed[i];
            speed[i] += axel[i];
            //а тут ли? надо добавить метод обнулени€ сил...
            axel[i] = 0;
        }
    }
    void applyRotation() {
        if (speed[0] == 0 && speed[1] == 0) return;
        float angle = atan2(speed[1],speed[0])/M_PI*180;
        axisAngle[2] = angle;
    }
    bool isVisible() {
        if ((axisShift[0] < -1.5f) || (axisShift[0] > 1.5f)) return 0;
        if ((axisShift[1] < - 2.5f) || (axisShift[1] > 2.5f)) return 0;
        if ((axisShift[2] < -2.f) || (axisShift[2] > 5.f)) return 0;
        return 1;
    }
    void axisTransform() {
        //начальное положение матрицы
        glLoadIdentity();
        //axis shift
        applyShift();
        applyRotation();
        glTranslatef
        (axisShift[0], axisShift[1], axisShift[2] - 1.9);
        //axis rotation
        glRotatef(axisAngle[0], 1, 0, 0);  // X
        glRotatef(axisAngle[1], 0, 1, 0);  // Y
        glRotatef(axisAngle[2], 0, 0, 1);  // Z
        //axis scaling
        glScalef(axisScale[0], axisScale[1], axisScale[2]);
    }
};

class Point : public Particle {
protected:
    Ftype ftype = POINT;
    GLint vertices[1][3];
    float vcshift[1][3] =
    {
        { 0, 0, 0 },   //0
    };
    //цвета пока фисированы!
    GLint colors[1][4] =
    {
        { 0x10000, 0x10000, 0x10000, 0x10000 },
    };
    GLubyte indices[1] =
    {
        0
    };
public:
    Point() {
        init();
    }
    Point(float _axisShift[3]) : Particle(_axisShift) {
        init();
    }
    Point(float _axisShift[3], float _axisAngle[3]) : Particle(_axisShift, _axisAngle) {
        init();
    }
    Point(float _axisShift[3], float _axisAngle[3], float _axisScale[3]) : Particle(_axisShift, _axisAngle, _axisScale) {
        init();
    }
    // заполн€ем массив вертикалей
    void init() {
        for (int _point = 0; _point < 1; _point++)
            for (int _coord = 0; _coord < 3; _coord++)
                vertices[_point][_coord] = vcshift[_point][_coord] /** size*/ * 0x10000 + center[_coord];
    };

    void shiftClr(int vert, GLint _r, GLint _g, GLint _b, GLint _alpha) {
        colors[vert][0] += _r;
        colors[vert][1] += _g;
        colors[vert][2] += _b;
        colors[vert][3] += _alpha;
    }

    void show() {
        glMatrixMode(GL_MODELVIEW);

        axisTransform();

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        glPointSize(10.0);
        glFrontFace(GL_CW);
        glDisable(GL_CULL_FACE);
        glVertexPointer(3, GL_FIXED, 0, vertices);
        glColorPointer(4, GL_FIXED, 0, colors);
        glDrawElements(GL_POINTS, 1, GL_UNSIGNED_BYTE, indices);

        lifetime++;
    }
};

class Triangle : public Particle {
protected:
    Ftype ftype = TRIANGLE;
    GLint vertices[3][3];
    /*
    float vcshift[3][3] =
    {
        { -0.5, -0.28, 0 },   //0
        { 0.5, -0.28, 0 },    //1
        { 0, 0.57, 0 },    //2
    };
    */
    float vcshift[3][3] =
    {
        { -0.28, -0.5, 0 },   //0
        { -0.28, 0.5, 0 },    //1
        { 0.57, 0, 0 },    //2
    };
    //цвета пока фисированы!
    GLint colors[3][4] =
    {
        { 0x10000, 0x10000, 0x00000, 0x09000 },
        { 0x00000, 0x10000, 0x10000, 0x09000 },
        { 0x10000, 0x00000, 0x10000, 0x10000 },
    };
    GLubyte indices[3] =
    {
        2, 1, 0
    };
public:
    Triangle() {
        init();
    }
    Triangle(float _axisShift[3]) : Particle(_axisShift) {
        init();
    }
    Triangle(float _axisShift[3], float _axisAngle[3]) : Particle(_axisShift, _axisAngle) {
        init();
    }
    Triangle(float _axisShift[3], float _axisAngle[3], float _axisScale[3]) : Particle(_axisShift, _axisAngle, _axisScale) {
        init();
    }
    // заполн€ем массив вертикалей
    void init() {
        for (int _point = 0; _point < 3; _point++)
            for (int _coord = 0; _coord < 3; _coord++)
                vertices[_point][_coord] = vcshift[_point][_coord] /** size*/ * 0x10000 + center[_coord];
    };

    void shiftClr(int vert, GLint _r, GLint _g, GLint _b, GLint _alpha) {
        colors[vert][0] += _r;
        colors[vert][1] += _g;
        colors[vert][2] += _b;
        colors[vert][3] += _alpha;
    }

    void show() {
        glMatrixMode(GL_MODELVIEW);
        
        axisTransform();

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        glFrontFace(GL_CW);
        glDisable(GL_CULL_FACE);
        glVertexPointer(3, GL_FIXED, 0, vertices);
        glColorPointer(4, GL_FIXED, 0, colors);
        glDrawElements(GL_POINTS, 1, GL_UNSIGNED_BYTE, indices);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, indices);

        lifetime++;
    }
};

class Phys {
public:
    constexpr static float dempcoeff = 0.7;
    static bool areCollided(Triangle* _triangle, Point* _point) {
        float distsqr =
            std::pow(_triangle->axisShift[0] - _point->axisShift[0], 2) +
            std::pow(_triangle->axisShift[1] - _point->axisShift[1], 2);
        // если касание детектед
        if (distsqr < (float)(_triangle->axisScale[0] / 9) ||
            distsqr < (float)(_triangle->axisScale[1] / 9))
        {
            // закон сохранени€ импульса
            float absV = sqrt(pow(_triangle->speed[0], 2) + pow(_triangle->speed[1], 2))*dempcoeff;
            float tg = (_triangle->axisShift[1] - _point->axisShift[1]) / (_triangle->axisShift[0] - _point->axisShift[0]);
            float angle = atan(tg);
            //srand((unsigned int)_triangle->lifetime);
            //float randV = ((int)((int)rand() % 10) /1000);
            if ((_triangle->axisShift[0] - _point->axisShift[0]) < 0) angle += M_PI;
            _triangle->speed[0] = (absV /* + (_triangle->axisScale[0] - sqrt(distsqr)) * 0.1*/) * cos(angle);
            _triangle->speed[1] = (absV /* + (_triangle->axisScale[1] - sqrt(distsqr)) * 0.1*/) * sin(angle);
            // учитываем глубину, кхм, проникновени€.
            //_triangle->speed[0] += _triangle->axisScale[0] - distsqr;
            //_triangle->speed[1] += distsqr - _triangle->axisScale[1];
            return 1;
        }
        return 0;
    }

/*
    static void collisionReact(Triangle* _triangle, Point* _point) {
        float absV = sqrt(pow(_triangle->speed[0], 2) + pow(_triangle->speed[0], 2));
        float angle = atan(_triangle->axisShift[1] - _point->axisShift[1]) / (_triangle->axisShift[0] - _point->axisShift[0]);
        _triangle->speed[0] = absV * cos(angle);
        _triangle->speed[1] = absV * sin(angle);
    }
*/
};

#endif
