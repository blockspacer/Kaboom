#pragma once

#include <osgGA/GUIEventHandler>

class InputEventHandler;

class MouseEventHandler : public osgGA::GUIEventHandler {
public:
    typedef void(InputEventHandler::*Function) ();

    enum KeyState {
        KEY_UP,
        KEY_DOWN
    };

    struct FunctionState {
        FunctionState() {
            keyState = KEY_UP;
            keyFunction = NULL;
        }

        Function keyFunction;
        KeyState keyState;
    };

    typedef std::map<int, FunctionState> KeyFunctionMap;

    MouseEventHandler(InputEventHandler &inputEventHandler);
    ~MouseEventHandler();

    bool bindKey(int key, Function func);
    bool bindKey(int key, KeyState state, Function func);

    virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
    virtual void accept(osgGA::GUIEventHandlerVisitor &v);

private:
    InputEventHandler &inputEventHandler;

    KeyFunctionMap keyDownFuncMap;
    KeyFunctionMap keyUpFuncMap;
};
