///-----------------------------------------------------------------
///
/// @file      VisualizationFrm.h
/// @author    Michael
/// Created:   2016/7/4 ¤U¤È 10:02:57
/// @section   DESCRIPTION
///            Visualization class declaration
///
///------------------------------------------------------------------

#ifndef __VISUALIZATION_H__
#define __VISUALIZATION_H__

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
#include <wx/button.h>
#include <wx/panel.h>
////Header Include End

////Dialog Style Start
#undef Visualization_STYLE
#define Visualization_STYLE wxCAPTION | wxSYSTEM_MENU | wxMINIMIZE_BOX | wxCLOSE_BOX
////Dialog Style End

class Visualization : public wxFrame
{
	private:
		DECLARE_EVENT_TABLE();
		
	public:
		Visualization(wxWindow *parent, wxWindowID id = 1, const wxString &title = wxT("Choose of visualization type"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = Visualization_STYLE);
		virtual ~Visualization();
		
		void WxButton1Click(wxCommandEvent& event);
		void WxButton2Click(wxCommandEvent& event);
		void WxButton3Click(wxCommandEvent& event);
		
	private:
		//Do not add custom control declarations between
		//GUI Control Declaration Start and GUI Control Declaration End.
		//wxDev-C++ will remove them. Add custom code after the block.
		////GUI Control Declaration Start
		wxFileDialog *WxOpenFileDialog8;
		wxFileDialog *WxOpenFileDialog7;
		wxFileDialog *WxOpenFileDialog3;
		wxFileDialog *WxOpenFileDialog6;
		wxFileDialog *WxOpenFileDialog1;
		wxFileDialog *WxOpenFileDialog2;
		wxFileDialog *WxOpenFileDialog4;
		wxFileDialog *WxOpenFileDialog5;
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
