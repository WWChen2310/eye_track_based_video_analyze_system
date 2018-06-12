///-----------------------------------------------------------------
///
/// @file      TestFrm.cpp
/// @author    Michael
/// Created:   2016/7/4 ¤U¤È 08:14:03
/// @section   DESCRIPTION
///            TestFrm class implementation
///
///------------------------------------------------------------------

#include "TestFrm.h"

//Do not add custom headers between
//Header Include Start and Header Include End
//wxDev-C++ designer will remove them
////Header Include Start
////Header Include End

//----------------------------------------------------------------------------
// TestFrm
//----------------------------------------------------------------------------
//Add Custom Events only in the appropriate block.
//Code added in other places will be removed by wxDev-C++
////Event Table Start
BEGIN_EVENT_TABLE(TestFrm,wxFrame)
	////Manual Code Start
	////Manual Code End
	EVT_BUTTON(ID_WXBUTTON1,TestFrm::WxButton1Click)
	EVT_BUTTON(ID_WXBUTTON2,TestFrm::WxButton2Click)
	
	EVT_CLOSE(TestFrm::OnClose)
END_EVENT_TABLE()
////Event Table End

TestFrm::TestFrm(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &position, const wxSize& size, long style)
: wxFrame(parent, id, title, position, size, style)
{
	CreateGUIControls();
}

TestFrm::~TestFrm()
{
}

void TestFrm::CreateGUIControls()
{
	//Do not add custom code between
	//GUI Items Creation Start and GUI Items Creation End
	//wxDev-C++ designer will remove them.
	//Add the custom code before or after the blocks
	////GUI Items Creation Start

	WxPanel1 = new wxPanel(this, ID_WXPANEL1, wxPoint(1, 2), wxSize(373, 143));

	//WxButton1 = new wxButton(WxPanel1, ID_WXBUTTON1, _("WxButton1"), wxPoint(60, 57), wxSize(2, 3), 0, wxDefaultValidator, _("WxButton1"));

	WxButton1 = new wxButton(WxPanel1, ID_WXBUTTON1, _("WxButton1"), wxPoint(59, 43), wxSize(72, 54), 0, wxDefaultValidator, _("WxButton1"));

	WxButton2 = new wxButton(WxPanel1, ID_WXBUTTON2, _("WxButton2"), wxPoint(206, 45), wxSize(80, 52), 0, wxDefaultValidator, _("WxButton2"));

	SetTitle(_("TestFrm"));
	SetIcon(wxNullIcon);
	SetSize(8,8,389,183);
	Center();
	
	////GUI Items Creation End
}

void TestFrm::OnClose(wxCloseEvent& event)
{
	Destroy();
}

void TestFrm::WxButton1Click(wxCommandEvent& event)
{
	// insert your code here
	// Create a new frame
	extern wxString video_path;//work
    wxMessageBox(video_path);
}

void TestFrm::WxButton2Click(wxCommandEvent& event)
{
	// insert your code here
	// Create a new frame
    
}

