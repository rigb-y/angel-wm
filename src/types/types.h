/*
 * Copyright (c) 2026 rigb-y 
 * SPDX-License-Identifier: MIT
 */

#ifndef ANGEL_TYPE_H
#define ANGEL_TYPE_H

#include <X11/Xlib.h>

#define STATUS_FAIL 0

#define CLIENT_LIST_ORIGIN(X) \
    X(UNKNOWN) \
    X(CL) \
    X(FL) \
    X(UL) \
    X(ML)

#define MAKE_CLO_ENUM(Z) ORIGIN_##Z,
typedef enum ClientListOrigin {
    CLIENT_LIST_ORIGIN(MAKE_CLO_ENUM)
} ClientListOrigin;
#undef MAKE_CLO_ENUM

#define MINIMIZED_POSITION(X) \
    X(LEFT) \
    X(RIGHT) \
    X(TOP) \
    X(BOTTOM) \
    X(POSITION_UNKNOWN)

#define MAKE_MP_ENUM(Z) MINIMIZED_##Z,
typedef enum MinimizedPosition {
    MINIMIZED_POSITION(MAKE_MP_ENUM)
} MinimizedPosition;
#undef MAKE_MP_ENUM

#define STRUT(X) \
    X(TOP) \
    X(BOTTOM) \
    X(LEFT) \
    X(RIGHT)

#define MAKE_STRUT_SIDE_ENUM(Z) STRUT_SIDE_##Z,
typedef enum StrutSide {
    STRUT(MAKE_STRUT_SIDE_ENUM) 
} StrutSide;
#undef MAKE_STRUT_SIDE_ENUM

#define FOCUS_START(X) \
    X(ADJACENT) \
    X(END) \
    X(UNKNOWN)

#define MAKE_FOCUS_START_ENUM(Z) FOCUS_START_##Z,
typedef enum FocusStart {
    FOCUS_START(MAKE_FOCUS_START_ENUM) 
} FocusStart;
#undef MAKE_FOCUS_START_ENUM

#define FOCUS_END(X) \
    X(NEXT) \
    X(FOCUS_STACK) \
    X(UNKNOWN)

#define MAKE_FOCUS_END_ENUM(Z) FOCUS_END_##Z,
typedef enum FocusEnd {
    FOCUS_END(MAKE_FOCUS_END_ENUM) 
} FocusEnd;
#undef MAKE_FOCUS_END_ENUM

typedef struct Client Client;
typedef struct MTNode MTNode;
typedef struct MotionTree MotionTree;
typedef struct ConfigError ConfigError;
typedef struct Token Token;
typedef struct string string;
typedef struct OverrideParameters OverrideParameters;
typedef struct Monitor Monitor;
typedef struct Monitors Monitors;

typedef int(*Handler)(Display*, XErrorEvent*);
typedef int(*IOHandler)(Display*);

typedef MTNode*(*TreeMotionFn)(MotionTree*, MTNode*);

typedef void(*ResizeStepFn)(Client*);
typedef void(*ResizeSetStepFn)(Client*, int);
typedef int(*ResizeStepInfoFn)(const Client*);

typedef int(*GeometryInfoFn)(const Client*);

typedef void(*LayoutFn)(Monitor*);

typedef void(*EventHandleFn)(const XEvent*); 
typedef void(*KeybindActionFn)(const XKeyEvent*);
typedef int EventType;

typedef void MapClientFn(Client*, _Bool);

typedef ConfigError(*LexFn)(Token*, string*);

typedef ConfigError(*ParseFn)(void);

typedef _Bool(*OverrideDefaultFn)(const OverrideParameters*);

typedef _Bool(*IsValidFn)(const Token*);

typedef int(*GeometryScreenDimFn)(void);

typedef void(*SetMappedFromSwitchFn)(Client*, _Bool);

typedef void(*ReconcileMinimizedFn)(Monitor*, int);

typedef struct SubTileFn {
    void(*fn)(Monitor*, Client*, struct SubTileFn, _Bool, int, int, int, int);
    _Bool master;
    _Bool master_direction;
} SubTileFn;

typedef int(*MonitorGeometryInfoFn)(const Monitor*);

typedef int(*MinOrMaxFn)(int, int);
typedef int(*MonitorPositionInfoFn)(const Monitor*);
typedef MonitorPositionInfoFn MonitorGeometryInfoFn; 

typedef _Bool(*IntCompareFn)(int, int);

typedef Monitor*(*GetAdjacentMonitorFn)(const Monitors*, const Monitor*);

#endif
