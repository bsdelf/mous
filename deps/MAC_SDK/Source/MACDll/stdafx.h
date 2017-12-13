#pragma once

// environment
#include "WindowsEnvironment.h"

// defines for reducing size
#define VC_EXTRALEAN                            // Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS        // some CString constructors will be explicit

// MFC
#include <afxwin.h>                                // MFC core and standard components
#include <afxext.h>                                // MFC extensions

// Monkey's Audio
#include "All.h"
#include "MACLib.h"
using namespace APE;

// resources
#include "resource.h"
