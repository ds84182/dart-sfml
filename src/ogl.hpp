#pragma once

// Modified version of SFML/OpenGL.hpp to support versions later than 1.1

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include <SFML/Config.hpp>

#define GL_GLEXT_PROTOTYPES

////////////////////////////////////////////////////////////
/// This file just includes the OpenGL headers,
/// which have actually different paths on each system
////////////////////////////////////////////////////////////
#if defined(SFML_SYSTEM_WINDOWS)

    // The Visual C++ version of gl.h uses WINGDIAPI and APIENTRY but doesn't define them
    #ifdef _MSC_VER
        #include <windows.h>
    #endif

    #include <GL/gl.h>
    #include <GL/glext.h>

#elif defined(SFML_SYSTEM_LINUX) || defined(SFML_SYSTEM_FREEBSD)

    #if defined(SFML_OPENGL_ES)
        #include <GLES2/gl.h>
        #include <GLES2/glext.h>
    #else
        #include <GL/gl.h>
        #include <GL/glext.h>
    #endif

#elif defined(SFML_SYSTEM_MACOS)

    #include <OpenGL/gl.h>
    #include <OpenGL/glext.h>

#elif defined (SFML_SYSTEM_IOS)

    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>

#elif defined (SFML_SYSTEM_ANDROID)

    #include <GLES2/gl.h>
    #include <GLES2/glext.h>

#endif
