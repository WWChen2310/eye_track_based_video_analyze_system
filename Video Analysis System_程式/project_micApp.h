//---------------------------------------------------------------------------
//
// Name:        project_micApp.h
// Author:      mic
// Created:     2015/3/10 ¤W¤È 12:48:00
// Description: 
//
//---------------------------------------------------------------------------

#ifndef __PROJECT_MICFRMApp_h__
#define __PROJECT_MICFRMApp_h__

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#else
	#include <wx/wxprec.h>
#endif

class project_micFrmApp : public wxApp
{
	public:
		bool OnInit();
		int OnExit();
};

#endif
