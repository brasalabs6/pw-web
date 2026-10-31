/*
 * FILE: EXDISPID.H (platform seam)
 *
 * DESCRIPTION: The WebBrowser dispatch IDs the browser-dialog wrapper's
 * event handlers bind to. The documented Internet Explorer dispatch IDs.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_EXDISPID_H
#define PW153_PLATFORM_EXDISPID_H

#include "exdisp.h"	//	IDispatch/IWebBrowser2 come through the original ATL chain.

//\tThe documented WebBrowser dispatch IDs.
#define DISPID_NAVIGATECOMPLETE2 0x000000fc
#define DISPID_NAVIGATEERROR 0x000001fe
#define DISPID_NEWWINDOW2 0x000000fb
#define DISPID_NEWWINDOW3 0x00000107
#define DISPID_BEFORENAVIGATE2 0x000000fa
#define DISPID_DOCUMENTCOMPLETE 0x00000103
#define DISPID_DOWNLOADBEGIN 0x0000006a
#define DISPID_DOWNLOADCOMPLETE 0x00000068
#define DISPID_COMMANDSTATECHANGE 0x00000069
#define DISPID_STATUSTEXTCHANGE 0x00000066
#define DISPID_TITLECHANGE 0x00000071
#define DISPID_PROGRESSCHANGE 0x0000006c
#define DISPID_PROPERTYCHANGE 0x00000070
#define DISPID_QUIT 0x00000003

#endif // PW153_PLATFORM_EXDISPID_H
