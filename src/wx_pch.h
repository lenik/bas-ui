#ifndef WX_PCH_H
#define WX_PCH_H

// 1. Include the PCH support header
#include <wx/wxprec.h>

// 2. Fallback for compilers that do not support PCH
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#else
    #include "bulk.h"
#endif

#endif // WX_PCH_H
