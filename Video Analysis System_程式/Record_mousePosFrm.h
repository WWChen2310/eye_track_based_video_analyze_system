///-----------------------------------------------------------------
///
/// @file      Record_mousePosFrm.h
/// @author    mic
/// Created:   2015/3/10 ¤W¤È 01:43:13
/// @section   DESCRIPTION
///            Record_mousePosFrm class declaration
///
///------------------------------------------------------------------

#ifndef __RECORD_MOUSEPOSFRM_H__
#define __RECORD_MOUSEPOSFRM_H__

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
#include <wx/timer.h>
#include <wx/mediactrl.h>
////Header Include End

////Dialog Style Start
#undef Record_mousePosFrm_STYLE
#define Record_mousePosFrm_STYLE wxNO_BORDER
////Dialog Style End

class Record_mousePosFrm : public wxFrame
{
	private:
		DECLARE_EVENT_TABLE();
		
	public:
		Record_mousePosFrm(wxWindow *parent, wxWindowID id = 1, const wxString &title = wxT("Record_mousePos"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = Record_mousePosFrm_STYLE);
		virtual ~Record_mousePosFrm();
		void Record_mousePosFrmKeyDown(wxKeyEvent& event);
		void WxTimer1Timer(wxTimerEvent& event);
		
	private:
		//Do not add custom control declarations between
		//GUI Control Declaration Start and GUI Control Declaration End.
		//wxDev-C++ will remove them. Add custom code after the block.
		////GUI Control Declaration Start
		wxTimer *WxTimer1;
		wxMediaCtrl *WxMediaCtrl1;
		////GUI Control Declaration End
		
	private:
		//Note: if you receive any error with these enum IDs, then you need to
		//change your old form code that are based on the #define control IDs.
		//#defines may replace a numeric value for the enum names.
		//Try copy and pasting the below block in your old form header files.
		enum
		{
			////GUI Enum Control ID Start
			ID_WXTIMER1 = 1003,
			ID_WXMEDIACTRL1 = 1002,
			////GUI Enum Control ID End
			ID_DUMMY_VALUE_ //don't remove this value unless you have other enum values
		};
		
	private:
		void OnClose(wxCloseEvent& event);
		void CreateGUIControls();
};

#endif
