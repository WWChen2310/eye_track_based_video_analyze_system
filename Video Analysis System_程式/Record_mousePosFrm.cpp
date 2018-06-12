///-----------------------------------------------------------------
///
/// @file      Record_mousePosFrm.cpp
/// @author    mic
/// Created:   2015/3/10 ¤W¤È 01:43:13
/// @section   DESCRIPTION
///            Record_mousePosFrm class implementation
///
///------------------------------------------------------------------

#include "Record_mousePosFrm.h"

//Do not add custom headers between
//Header Include Start and Header Include End
//wxDev-C++ designer will remove them
////Header Include Start
////Header Include End

#include <wx/utils.h>
#include <vector>
#include <fstream>

using namespace std;

class mousePos
{
private:
    int mouse_x;
    int mouse_y;
    wxLongLong videoTime_msec;
public:
    mousePos( int x, int y, wxLongLong msec )
    {
        mouse_x = x;
        mouse_y = y;
        videoTime_msec = msec;
    }
    int get_mouseX( )
    {
        return mouse_x;
    }
    int get_mouseY( )
    {
        return mouse_y;
    }
    wxLongLong get_videoTime( )
    {
        return videoTime_msec;
    }
};

wxMediaCtrl* WxMediaCtrl1;
vector<mousePos> All_mousePos;
//const wxPoint pt;
int mouseX, mouseY;
wxLongLong videoTime;
int Time = 15;
bool saveData = false;

extern wxString video_path;
extern wxString video_name;

//----------------------------------------------------------------------------
// Record_mousePosFrm
//----------------------------------------------------------------------------
//Add Custom Events only in the appropriate block.
//Code added in other places will be removed by wxDev-C++
////Event Table Start
BEGIN_EVENT_TABLE(Record_mousePosFrm,wxFrame)
	////Manual Code Start
	////Manual Code End
	
	EVT_CLOSE(Record_mousePosFrm::OnClose)
	EVT_KEY_DOWN(Record_mousePosFrm::Record_mousePosFrmKeyDown)
	EVT_TIMER(ID_WXTIMER1,Record_mousePosFrm::WxTimer1Timer)
END_EVENT_TABLE()
////Event Table End

Record_mousePosFrm::Record_mousePosFrm(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &position, const wxSize& size, long style)
: wxFrame(parent, id, title, position, size, style)
{
	CreateGUIControls();
}

Record_mousePosFrm::~Record_mousePosFrm()
{
}

void Record_mousePosFrm::CreateGUIControls()
{
	//Do not add custom code between
	//GUI Items Creation Start and GUI Items Creation End
	//wxDev-C++ designer will remove them.
	//Add the custom code before or after the blocks
	////GUI Items Creation Start

	WxTimer1 = new wxTimer();
	WxTimer1->SetOwner(this, ID_WXTIMER1);

	WxMediaCtrl1 = new wxMediaCtrl(this, ID_WXMEDIACTRL1, _(""), wxPoint(175, 133), wxSize(150, 150), 0, _(""), wxDefaultValidator, _("WxMediaCtrl1"));
	WxMediaCtrl1->Enable(false);
	WxMediaCtrl1->ShowPlayerControls(wxMEDIACTRLPLAYERCONTROLS_NONE);

	SetTitle(_("Record_mousePos"));
	SetIcon(wxNullIcon);
	SetSize(8,8,600,600);
	Center();
	
	////GUI Items Creation End
	wxString welcomeText = "Welcome to use the \"Eye Movement Recorder\"\n\n";
	welcomeText += "1.Load the video    ctrl+L\n";
	welcomeText += "2.Resize the window ctrl+R\n";
	welcomeText += "3.Play the video ctrl+P\n";
    welcomeText += "--when the end of the video--\n";
    welcomeText += "4.Output the eye movement data ctrl+O\n";
	welcomeText += "5.Exit ctrl+E\n";
	wxMessageBox(welcomeText);
}

void Record_mousePosFrm::OnClose(wxCloseEvent& event)
{
	Destroy();
}

/*
 * Record_mousePosFrmKeyDown
 */
void Record_mousePosFrm::Record_mousePosFrmKeyDown(wxKeyEvent& event)
{
	// insert your code here
	if( event.GetModifiers( )==wxMOD_CONTROL )
    {
        switch( event.GetKeyCode( ) )
        {
            // "Load" video
            case 'L':
                WxMediaCtrl1->Load( video_path );
                break;
                
            // "Play/pause" video
            case 'P':
            {
                if( WxMediaCtrl1->GetState( ) == wxMEDIASTATE_PLAYING )
                {
                    if( !WxMediaCtrl1->Pause( ) )
                        wxMessageBox( wxT( "Couldn't pause movie!" ) );
                    else
                        WxTimer1->Stop();
                }
                else
                {
                    if( !WxMediaCtrl1->Play( ) )
                        wxMessageBox( wxT( "Couldn't play movie!" ) );
                    else
                        WxTimer1->Start( Time );
                }
            }
                break;
            
            // "Resize" frame_size
            case 'R':
            {
                wxSize videoSize = WxMediaCtrl1->GetBestSize();
                int x = GetScreenPosition( ).x-( videoSize.x-GetSize( ).x )/2;
                int y = GetScreenPosition( ).y-( videoSize.y-GetSize( ).y )/2;
                SetSize( videoSize.x, videoSize.y );
                Centre( );
            }
                break;
            
            // "Output" mouse position info
            case 'O':
            {   
                wxString subjectName;
                wxTextEntryDialog *WxTextEntryDialog2;
                WxTextEntryDialog2 = new wxTextEntryDialog( NULL, _( "" ), _( "Please entry your name" ), _( "" ) );
                if( WxTextEntryDialog2->ShowModal( ) )
                {
                    subjectName = WxTextEntryDialog2->GetValue( );
                }
                
                string file_name;
                file_name += "eyeMovement_";
                file_name += video_name.mb_str( );
                file_name += "_";
                file_name += subjectName.mb_str( );
                file_name += ".txt";
                fstream file;
                file.open( file_name.c_str( ), ios::out | ios::trunc );
                
                for( int i=0 ; i<All_mousePos.size( ) ; i++ )
                {
                    file << All_mousePos[i].get_videoTime( ) << ',' << ';'
                         << All_mousePos[i].get_mouseX( ) << ','
                         << All_mousePos[i].get_mouseY( ) << ',' << ';' << '\n';
                }
                
                file.close( );
                All_mousePos.clear( );
                saveData = true;
            }
                break;
            
            // Exit
            case 'E':
            {
                if( saveData )
                {
                    Destroy( );
                }
                else
                {
                    wxMessageDialog *dial = new wxMessageDialog( NULL, 
                        wxT( "Are you sure to quit without saving?" ), wxT( "Exit" ), 
                        wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION );
                    if( dial->ShowModal( )==wxID_YES )
                        Destroy( );
                }
            }
                break;
        }
    }
}

/*
 * WxTimer1Timer
 */
void Record_mousePosFrm::WxTimer1Timer(wxTimerEvent& event)
{
	// insert your code here
	const wxPoint pt = wxGetMousePosition();
    mouseX = pt.x-GetScreenPosition().x;
    mouseY = pt.y-GetScreenPosition().y;
    videoTime = WxMediaCtrl1->Tell();
    
    if( videoTime==0 )
    {
        WxTimer1->Stop();
    }
    else
    {
        mousePos tempmousePos = mousePos( mouseX, mouseY, videoTime );
        All_mousePos.push_back( tempmousePos );
    }
}
