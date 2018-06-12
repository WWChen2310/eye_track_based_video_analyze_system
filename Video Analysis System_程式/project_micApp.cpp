//---------------------------------------------------------------------------
//
// Name:        project_micApp.cpp
// Author:      mic
// Created:     2015/3/10 ¤W¤È 12:48:00
// Description: 
//
//---------------------------------------------------------------------------

#include "project_micApp.h"
#include "project_micFrm.h"

IMPLEMENT_APP(project_micFrmApp)

bool project_micFrmApp::OnInit()
{
    project_micFrm* frame = new project_micFrm(NULL);
    SetTopWindow(frame);
    frame->Show();
    return true;
}
 
int project_micFrmApp::OnExit()
{
	return 0;
}
