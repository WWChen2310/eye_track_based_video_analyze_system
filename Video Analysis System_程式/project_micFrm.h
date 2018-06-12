///-----------------------------------------------------------------
///
/// @file      project_micFrm.h
/// @author    mic
/// Created:   2015/3/10 ¤W¤È 12:48:01
/// @section   DESCRIPTION
///            project_micFrm class declaration
///
///------------------------------------------------------------------

#ifndef __PROJECT_MICFRM_H__
#define __PROJECT_MICFRM_H__

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

//Do not add custom headers between 
//Header Include Start and Header Include End.
//wxDev-C++ designer will remove them. Add custom headers after the block.
////Header Include Start
#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/button.h>
#include <wx/panel.h>
////Header Include End

////Dialog Style Start
#undef project_micFrm_STYLE
#define project_micFrm_STYLE wxCAPTION | wxSYSTEM_MENU | wxMINIMIZE_BOX | wxCLOSE_BOX
////Dialog Style End

class project_micFrm : public wxFrame
{
	private:
		DECLARE_EVENT_TABLE();
		
	public:
		project_micFrm(wxWindow *parent, wxWindowID id = 1, const wxString &title = wxT("project_mic"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = project_micFrm_STYLE);
		virtual ~project_micFrm();
		void Mnuopen1006Click(wxCommandEvent& event);
		void Mnuexit1011Click(wxCommandEvent& event);
		void WxButton2Click(wxCommandEvent& event);
		void WxButton1Click(wxCommandEvent& event);
		void WxButton3Click(wxCommandEvent& event);
		
		
		
	private:
		//Do not add custom control declarations between
		//GUI Control Declaration Start and GUI Control Declaration End.
		//wxDev-C++ will remove them. Add custom code after the block.
		////GUI Control Declaration Start
		wxFileDialog *WxOpenFileDialog1;
		wxFileDialog *WxOpenFileDialog4;
		wxMenuBar *WxMenuBar1;
		wxButton *WxButton3;
		wxButton *WxButton2;
		wxButton *WxButton1;
		wxPanel *WxPanel1;
		////GUI Control Declaration End
		
	private:
		//Note: if you receive any error with these enum IDs, then you need to
		//change your old form code that are based on the #define control IDs.
		//#defines may replace a numeric value for the enum names.
		//Try copy and pasting the below block in your old form header files.
		enum
		{
			////GUI Enum Control ID Start
			ID_MNU_FILE_1005 = 1005,
			ID_MNU_OPEN_1006 = 1006,
			ID_MNU_EXIT_1011 = 1011,
			ID_MNU_HELP_1009 = 1009,
			
			ID_WXBUTTON3 = 1004,
			ID_WXBUTTON2 = 1003,
			ID_WXBUTTON1 = 1002,
			ID_WXPANEL1 = 1001,
			////GUI Enum Control ID End
			ID_DUMMY_VALUE_ //don't remove this value unless you have other enum values
		};
		
	private:
		void OnClose(wxCloseEvent& event);
		void CreateGUIControls();
};

#endif
