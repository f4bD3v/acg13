//=============================================================================
//
//   Exercise code for the lecture
//   "Advanced Computer Graphics"
//
//   Adapted from Prof. Dr. Mario Botsch, Bielefeld University
//
//   Copyright (C) 2013 LGG, epfl
//
//   DO NOT REDISTRIBUTE
//=============================================================================


#ifndef GLUT_VIEWER_H
#define GLUT_VIEWER_H


//== INCLUDES =================================================================

#include <map>
#include <vector>
#include <string>


//== CLASS DEFINITION =========================================================



/** \class GLUT_viewer GLUT_viewer.h <utils/GLUT_viewer.h>
 Simple Glut viewer. 
 Based on C++ glut interface of George Stetten and Korin Crawford.
 **/

class GLUT_viewer
{
public:
    /// constructor
    GLUT_viewer(const char* _title, int _width, int _height);

    /// destructor
    virtual ~GLUT_viewer();

protected:
    /// draw the scene
    virtual void draw() {}

    /// turn the idle function on/off
    void toggle_idle(bool _b);

    /// display new frame. calls the draw() method
    virtual void display(void);

    /// handle keyboard events
    virtual void keyboard(int key, int x, int y);

    /// this function is called whenever there is time for it, i.e. whenever the application is idle.
    virtual void idle(void) {}

    /// handle keyboard events for special keys
    virtual void special(int key, int x, int y) {}

    /// handle mouse move events
    virtual void motion(int x, int y) {}

    /// handle mouse button events
    virtual void mouse(int button, int state, int x, int y) {}

    /// handle mouse move events without a button being pressed
    virtual void passivemotion(int x, int y) {}

    /// handle resize events
    virtual void reshape(int w, int h) {}

    /// handle change of visibility events
    virtual void visibility(int visible) {}

    /// handle popup-menu events
    virtual void processmenu(int i) {}

protected:
    /// width of the window
    int  width_;

    /// height of the window
    int height_;

private:
    static void display__(void);
    static void idle__(void);
    static void keyboard__(unsigned char key, int x, int y);
    static void motion__(int x, int y);
    static void mouse__(int button, int state, int x, int y);
    static void passivemotion__(int x, int y);
    static void reshape__(int w, int h);
    static void special__(int key, int x, int y);
    static void visibility__(int visible);
    static void processmenu__(int i);

    static std::map<int, GLUT_viewer*>  windows__;
    static GLUT_viewer* current_window();

private:
    int  windowID_;
    bool fullscreen_;
    int  bak_left_, bak_top_, bak_width_, bak_height_;
};


//=============================================================================
#endif
//=============================================================================

