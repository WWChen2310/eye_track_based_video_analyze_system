///-----------------------------------------------------------------
///
/// @file      project_micFrm.cpp
/// @author    mic
/// Created:   2015/3/10 上午 12:48:01
/// @section   DESCRIPTION
///            project_micFrm class implementation
///
///------------------------------------------------------------------

#include "project_micFrm.h"

//Do not add custom headers between
//Header Include Start and Header Include End
//wxDev-C++ designer will remove them
////Header Include Start
////Header Include End

#include "Record_mousePosFrm.h"
#include "ManualAutoFrm.h"
#include "VisualizationFrm.h"
#include "heatmap.h"
#include "RdYlGn.h"
#include "Spectral.h"




#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <cv.h>
#include <highgui.h>
#include <fstream>
#include <vector>

#define RED_REC 0
#define PURPLE_REC 1

#define AUTO 0
#define MAMU 1

using namespace cv;
using namespace std;


void initialVideo( );
void on_track_bar_slide( int pos );
void my_mouse_callback( int event, int x, int y, int flags, void* param );
int interpolation( int x, int y, int timeX, int timeY );
void draw_box( IplImage* img, int pos, int type );
void Caculate_offset_distance(int standard_x, int standard_y, int N_x, int N_y);
void draw_HeatMap( IplImage* img, int pos );
int mouse_location( int x, int y, int pos );
int nearby_corner( int x, int y, int pos );
int change_name( int x, int y, int pos );
void outputAOI( );

//用於 Analysis
void inputAOI( wxString dynamic_AOI );
void input_other_AOI( wxArrayString other_AOI_path );//用於碩論實驗
void inputEyePosition( wxString eye_Position );
void input_otherEyePosition( wxArrayString other_eyePosition );
void outputAnalysis(int type);

//Auto Mode object tracking related
void AOI_tracking_Initial(Mat frame, int AOIth, int AOIindex);
void Tracker_space_release(int AOIth, Mat frame);


// frames,fps,mspf are video data for drawing mouse position
int frames;
float fps;
float mspf; // ms per frame
int frame_width;//用於HeatMap大小設定 
int frame_height;//用於HeatMap大小設定 

// AOI class : store "a" AOI information
class AOI
{
    // AOI_frame_vector is a vector for "frame position of video"
    // AOI_position is a vector for "AOI state(x,y,width,height)" corresponding to AOI_frame_vector
    // fixationTime records each length of the time that subject watchs on this object(AOI)
    private:
        vector<int> AOI_frame_vector;
        vector<CvRect> AOI_position;
		
		//Auto mode
		vector<bool> AOI_AutoMode_isAdjust;
		int AOI_Type;
    public:
        AOI( int x, int y, int pos )
        {
            CvRect tempbox = cvRect( x, y, 0, 0 );
            AOI_position.push_back( tempbox );
            // pos means AOI start at which frame of video
            AOI_frame_vector.push_back( pos );
            // -1 means AOI still alive until the end of video
            AOI_frame_vector.push_back( -1 );
			
			//Auto Mode
			AOI_AutoMode_isAdjust.push_back(false);
			//All_AOI[box_number].set_AOI_Type( AUTO );
        }
        
        string AOI_name;
        void input_txt( string tempname, vector<int> tempAOI_frame_vector, vector<CvRect> tempAOI_position )
        {
            AOI_name = tempname;
            for( int i=0 ; i<tempAOI_frame_vector.size( ) ; i++ )
            {
                AOI_frame_vector.push_back( tempAOI_frame_vector[i] );
            }
            for( int i=0 ; i<tempAOI_position.size( ) ; i++ )
            {
                AOI_position.push_back( tempAOI_position[i] );
            }
        }
        // output the length of time to this AOI 
        int output_AOItime( )
        {
            if( AOI_frame_vector.back( )!=-1 )
            {
                return AOI_frame_vector.back( )-AOI_frame_vector[0]+1;
            }
            else
            {
                return frames-AOI_frame_vector[0]+1;
            }
        }
        void All_clear( )
        {
            AOI_frame_vector.clear( );
            AOI_position.clear( );
        }
    
        int AOI_frame_size( )
        {
            return AOI_frame_vector.size( );
        }
        int AOI_size( )
        {
            return AOI_position.size( );
        }
        
        int AOI_frame( int i )
        {
            return AOI_frame_vector[i];
        }
        int get_x( int i )
        {
            return AOI_position[i].x;
        }
        int get_y( int i )
        {
            return AOI_position[i].y;
        }
        int get_width( int i )
        {
            return AOI_position[i].width;
        }
        int get_height( int i )
        {
            return AOI_position[i].height;
        }
		bool get_isAdjust(int i)
		{
			return AOI_AutoMode_isAdjust[i];
		}
		int get_isAdjust_size()
		{
			return AOI_AutoMode_isAdjust.size();
		}
		int get_AOI_Type()
		{
			return AOI_Type;
		}
    
        void set_x( int x, int i )
        {
            AOI_position[i].x=x;
        }
        void set_y( int y, int i )
        {
            AOI_position[i].y=y;
        }
        void set_width( int width, int i )
        {
            AOI_position[i].width=width;
        }
        void set_height( int height, int i )
        {
            AOI_position[i].height=height;
        }
		void set_isAdjust(bool s, int i)
		{
			AOI_AutoMode_isAdjust[i] = s;
		}
		void set_AOI_Type(int type)
		{
			AOI_Type = type;
		}
    
        void AOI_insert( CvRect box, int index, int pos )
        {
            AOI_position.insert( AOI_position.begin( )+index, box );
            AOI_frame_vector.insert( AOI_frame_vector.begin( )+index, pos );
			
			//Auto Mode
			AOI_AutoMode_isAdjust.insert(AOI_AutoMode_isAdjust.begin( )+index, false);
        }
        void AOI_pushback( CvRect box, int pos )
        {
            AOI_position.push_back( box );
            AOI_frame_vector.push_back( pos );
			
			//Auto Mode
			AOI_AutoMode_isAdjust.push_back(false);
        }
        void AOI_erase( int index )
        {
            AOI_position.erase( AOI_position.begin( )+index );
            AOI_frame_vector.erase( AOI_frame_vector.begin( )+index );
    
        }
        void AOI_delete( int index )
        {
            AOI_position.erase( AOI_position.begin( )+index, AOI_position.end( ) );
            AOI_frame_vector.erase( AOI_frame_vector.begin( )+index, AOI_frame_vector.end( ) );
        }
};

// Analysis_data used to store fixation data for each AOIs
class Analysis_data
{
    public:
        vector<int> fixationTime;
		int fix_times;//fix 超過 350ms 的次數
        Analysis_data( )
        {
            ;
        }
};

// store eye_Position info from inputEyePosition()
vector<int> eye_time;
vector< vector<int> > other_eye_time;

//基於時間點的特定個人EyePosition
vector<int> eye_PositionX;
vector<int> eye_PositionY;
//基於時間點的複數EyePosition
vector< vector<int> > other_eye_PositionX;
vector< vector<int> > other_eye_PositionY;

// eye_PositionX, eye_PositionY for each frame
//基於Frame的特定個人EyePosition(於inputEyePosition()從時間點轉換至對應的frame )
vector<int> eye_PositionX_frame;
vector<int> eye_PositionY_frame;
//基於Frame的複數EyePosition(於input_otherEyePosition()從時間點轉換至對應的frame )
vector< vector<int> > other_eye_PositionX_frame;
vector< vector<int> > other_eye_PositionY_frame;

vector<AOI> All_AOI;
vector< vector<AOI> > other_All_AOI;
bool drawAOI, moveAOI, adjustAOI, insertAOI, draw_eyePosition;
int oldboxx, oldboxy, oldx, oldy, box_number, video_position, pre_video_position;

// the index of AOI_frame_vector to match AOI
int AOIindex;

char type;
IplImage* origin_image;
IplImage* temp;
IplImage* pre_image;
//tracking
Mat M_frame;
Mat M_preframe;

CvCapture* capture=NULL;
CvCapture* pre_capture=NULL;
CvRect insertbox=cvRect( 0, 0, 0, 0 );

wxString video_path;//載入video的路徑 
wxString video_name;//載入video的名稱 
wxString dynamicAOI;//載入AOI.txt的路徑 
wxString eyePosition;//載入特定單一眼動資料的路徑 
wxArrayString other_eyePosition;//載入複數眼動資料的路徑 
int other_eyePosition_num = 0;////載入複數眼動資料的個數(default=0) 
wxString eyemovement_name;
wxArrayString group_eyemovement_name;

//碩論實驗
wxArrayString other_AOI_path;//載入複數眼動資料的路徑 
wxArrayString other_AOI_name;
int other_AOI_path_num = 0;////載入複數眼動資料的個數(default=0) 

//
bool videoanalyzer_mode = false;
//Heat Map Radius
int heatmap_Radius = 32;

vector<bool> isIniAOItracker; 
//Auto AOI Setting mode flags
bool isAuto_mode = false;

//test write
char filename_test[]="DCLButton_test_write.txt";
fstream fpp;


//碩論實驗: AOISetting 手動自動比較
int mouse_operation_count;


//碩論實驗相關-比較不同間隔N與N=1(standard)的offset
char filename_standard_N[]="Standard_N_offset_distance.csv";
fstream fp_standard_N;
int standard_N_frame_pos=-1;


//----------------------------------------------------------------------------
// project_micFrm
//----------------------------------------------------------------------------
//Add Custom Events only in the appropriate block.
//Code added in other places will be removed by wxDev-C++
////Event Table Start
BEGIN_EVENT_TABLE(project_micFrm,wxFrame)
	////Manual Code Start
	////Manual Code End
	
	EVT_CLOSE(project_micFrm::OnClose)
	EVT_MENU(ID_MNU_OPEN_1006, project_micFrm::Mnuopen1006Click)
	EVT_MENU(ID_MNU_EXIT_1011, project_micFrm::Mnuexit1011Click)
	EVT_BUTTON(ID_WXBUTTON3,project_micFrm::WxButton3Click)
	EVT_BUTTON(ID_WXBUTTON2,project_micFrm::WxButton2Click)
	EVT_BUTTON(ID_WXBUTTON1,project_micFrm::WxButton1Click)
END_EVENT_TABLE()
////Event Table End

project_micFrm::project_micFrm(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &position, const wxSize& size, long style)
: wxFrame(parent, id, title, position, size, style)
{
	CreateGUIControls();
}

project_micFrm::~project_micFrm()
{
}
/*
WxButton1 = new wxButton(WxPanel1, ID_WXBUTTON1, _("Eye Movement\nRecorder"), wxPoint(21, 30), wxSize(100, 100), 0, wxDefaultValidator, _("WxButton1"));

	WxButton2 = new wxButton(WxPanel1, ID_WXBUTTON2, _("Dynamic AOI\nModule"), wxPoint(142, 30), wxSize(100, 100), 0, wxDefaultValidator, _("WxButton2"));

	WxButton3 = new wxButton(WxPanel1, ID_WXBUTTON3, _("Video Analyzer"), wxPoint(263, 30), wxSize(100, 100), 0, wxDefaultValidator, _("WxButton3"));

	WxOpenFileDialog2 =  new wxFileDialog(this, _("Choose a \"dynamic AOI of video\" file"), _(""), _(""), _("Text files (*.txt)|*.txt"), wxFD_OPEN);

	WxOpenFileDialog1 =  new wxFileDialog(this, _("Choose a file"), _(""), _(""), _("video files (*.mp4;*.avi)|*.mp4;*.avi"), wxFD_OPEN);

	WxOpenFileDialog3 =  new wxFileDialog(this, _("Choose a \"eye movement data\" file"), _(""), _(""), _("Text files (*.txt)|*.txt"), wxFD_OPEN);

	WxMenuBar1 = new wxMenuBar();
	wxMenu *ID_MNU_FILE_1005_Mnu_Obj = new wxMenu();
	ID_MNU_FILE_1005_Mnu_Obj->Append(ID_MNU_OPEN_1006, _("&Open Video\tCtrl+V"), _(""), wxITEM_NORMAL);
	ID_MNU_FILE_1005_Mnu_Obj->AppendSeparator();
	ID_MNU_FILE_1005_Mnu_Obj->Append(ID_MNU_EXIT_1011, _("E&xit"), _(""), wxITEM_NORMAL);
	WxMenuBar1->Append(ID_MNU_FILE_1005_Mnu_Obj, _("&File"));
	
	wxMenu *ID_MNU_HELP_1009_Mnu_Obj = new wxMenu();
	WxMenuBar1->Append(ID_MNU_HELP_1009_Mnu_Obj, _("&Help"));
	SetMenuBar(WxMenuBar1);

	WxOpenFileDialog4 =  new wxFileDialog(this, _("Choose a \"dynamic AOI of video\" file"), _(""), _(""), _("Text files (*.txt)|*.txt"), wxFD_OPEN);

	SetTitle(_("Advertisement Video Analysis System"));
	SetIcon(wxNullIcon);
	SetSize(8,8,397,199);
	Center();
*/

void project_micFrm::CreateGUIControls()
{
	//Do not add custom code between
	//GUI Items Creation Start and GUI Items Creation End
	//wxDev-C++ designer will remove them.
	//Add the custom code before or after the blocks
	////GUI Items Creation Start

	WxPanel1 = new wxPanel(this, ID_WXPANEL1, wxPoint(0, 0), wxSize(385, 163));

	//mouse position Record
	WxButton1 = new wxButton(WxPanel1, ID_WXBUTTON1, _("Eye Movement\nRecorder"), wxPoint(21, 30), wxSize(100, 100), 0, wxDefaultValidator, _("WxButton1"));
    
	WxButton2 = new wxButton(WxPanel1, ID_WXBUTTON2, _("Dynamic AOI\nModule"), wxPoint(142, 30), wxSize(100, 100), 0, wxDefaultValidator, _("WxButton2"));

	WxButton3 = new wxButton(WxPanel1, ID_WXBUTTON3, _("Video Analyzer"), wxPoint(263, 30), wxSize(100, 100), 0, wxDefaultValidator, _("WxButton3"));
	
	WxOpenFileDialog4 =  new wxFileDialog(this, _("Choose a \"dynamic AOI of video\" file"), _(""), _(""), _("Text files (*.txt)|*.txt"), wxFD_OPEN);
    //File -> open video
	WxOpenFileDialog1 =  new wxFileDialog(this, _("Choose a Video"), _(""), _(""), _("video files (*.mp4;*.avi)|*.mp4;*.avi"), wxFD_OPEN);

	WxMenuBar1 = new wxMenuBar();
	wxMenu *ID_MNU_FILE_1005_Mnu_Obj = new wxMenu();
	ID_MNU_FILE_1005_Mnu_Obj->Append(ID_MNU_OPEN_1006, _("&Open Video\tCtrl+V"), _(""), wxITEM_NORMAL);
	ID_MNU_FILE_1005_Mnu_Obj->AppendSeparator();
	ID_MNU_FILE_1005_Mnu_Obj->Append(ID_MNU_EXIT_1011, _("E&xit"), _(""), wxITEM_NORMAL);
	WxMenuBar1->Append(ID_MNU_FILE_1005_Mnu_Obj, _("&File"));
	
	wxMenu *ID_MNU_HELP_1009_Mnu_Obj = new wxMenu();
	WxMenuBar1->Append(ID_MNU_HELP_1009_Mnu_Obj, _("&Help"));
	SetMenuBar(WxMenuBar1);

	SetTitle(_("Video Analysis System"));
	SetIcon(wxNullIcon);
	SetSize(8,8,397,199);
	Center();
	
	////GUI Items Creation End
}

void project_micFrm::OnClose(wxCloseEvent& event)
{
	Destroy();
}

/*
 * Mnuopen1006Click
 */
void project_micFrm::Mnuopen1006Click(wxCommandEvent& event)
{
	// insert your code here
	if( WxOpenFileDialog1->ShowModal( ) )
    {
        video_path = WxOpenFileDialog1->GetPath( );
        video_name = WxOpenFileDialog1->GetFilename( );
        wxStripExtension( video_name );
    }
}

/*
 * Mnuexit1011Click
 */
void project_micFrm::Mnuexit1011Click(wxCommandEvent& event)
{
	// insert your code here
	Destroy();
}


/*
 * WxButton1Click, Eye Movement Recorder
 */
void project_micFrm::WxButton1Click(wxCommandEvent& event)
{
	// insert your code here
	// Create a new frame
    Record_mousePosFrm * TempFrame = new Record_mousePosFrm(this);
    TempFrame->Show();
}

/*
 * WxButton2Click, Dynamic AOI module
 */
void project_micFrm::WxButton2Click(wxCommandEvent& event)
{
	//fpp.open(filename_test, ios::out);
	
    //叫出 ManualAuto Form 
    ManualAuto * TempFrame = new ManualAuto(this);
    TempFrame->Show();
}

/*
 * WxButton3Click, Video Analyzer
 */
void project_micFrm::WxButton3Click(wxCommandEvent& event)
{
    //叫出 Visualization Form    
    Visualization * TempFrame = new Visualization(this);
    TempFrame->Show();   
}


void initialVideo( )
{
    
    //wxMessageBox(video_path);
    
    drawAOI = false;
    moveAOI = false;
    adjustAOI = false;
    insertAOI = false;
    video_position = 0;
	pre_video_position = 0;
    AOIindex = 0;
    insertbox=cvRect( 0, 0, 0, 0 );
    dynamicAOI = "";
    eyePosition = "";
    draw_eyePosition = false;
    
    for( int i=0 ; i<All_AOI.size( ) ; i++ )
    {
        All_AOI[i].All_clear( );
    }
    All_AOI.clear();
	
}

void on_track_bar_slide( int pos )
{
	
    //cvSetCaptureProperty( capture, CV_CAP_PROP_POS_FRAMES, pos+50 );
	//pre_image=cvQueryFrame( capture );
	
	cvSetCaptureProperty( capture, CV_CAP_PROP_POS_FRAMES, pos );
}

// Implement mouse callback
void my_mouse_callback( int event, int x, int y, int flags, void* param )
{
    IplImage* frame = ( IplImage* ) param;

    switch( event )
    {
		case CV_EVENT_MOUSEMOVE://滑動
		{
			if( drawAOI || adjustAOI )
			{
				All_AOI[box_number].set_width( x-All_AOI[box_number].get_x( AOIindex ), AOIindex );
				All_AOI[box_number].set_height( y-All_AOI[box_number].get_y( AOIindex ), AOIindex );											
			}
			else if( moveAOI )
			{
				All_AOI[box_number].set_x( oldboxx+(x-oldx), AOIindex );
				All_AOI[box_number].set_y( oldboxy+(y-oldy), AOIindex );
				
				for(int i=AOIindex+1; i<All_AOI[box_number].get_isAdjust_size(); i++)
				{
					All_AOI[box_number].set_isAdjust(true, i);
				}
			}		
		}
		break;

		case CV_EVENT_LBUTTONDOWN://左鍵點擊
		{
			//cvCopyImage( origin_image, frame );
			cvCopy( origin_image, frame );
			
			//關鍵.....................................................
			box_number = mouse_location( x, y, video_position );//mouse_location()會決定AOIindex
			int corner = nearby_corner( x, y, video_position );

			// mouse cursor is not in any AOI
			if( box_number==-1 )//創新的AOI
			{
				drawAOI=true;
				AOI tempbox=AOI( x, y, video_position );
				All_AOI.push_back( tempbox );
				box_number=All_AOI.size( )-1;
				AOIindex=0;
				
				//Auto_Mode
				All_AOI[box_number].set_isAdjust( false, 0 );
				if(isAuto_mode)
				{
					All_AOI[box_number].set_AOI_Type( AUTO );
				}
				else
				{
					All_AOI[box_number].set_AOI_Type( MAMU );
				}
				
			}
			// mouse cursor is in a AOI but not nearby corner
			else if( box_number!=-1 && corner==-1 )//移動AOI
			{
				moveAOI = true;
				if( !insertAOI )
				{
					oldboxx = All_AOI[box_number].get_x( AOIindex );
					oldboxy = All_AOI[box_number].get_y( AOIindex );
				}
				else
				{
					All_AOI[box_number].AOI_insert( insertbox, AOIindex, video_position );
					oldboxx = insertbox.x;
					oldboxy = insertbox.y;
				}
				oldx = x;
				oldy = y;
			}
			// mouse cursor is nearby a corner of a AOI(調整位置大小 etc)
			else//調整AOI
			{
				adjustAOI = true;
				if( !insertAOI )
				{
					// left-top
					if( corner==1 )
					{
						All_AOI[box_number].set_x( All_AOI[box_number].get_x( AOIindex )+All_AOI[box_number].get_width( AOIindex ), AOIindex );
						All_AOI[box_number].set_y( All_AOI[box_number].get_y( AOIindex )+All_AOI[box_number].get_height( AOIindex ), AOIindex );
						All_AOI[box_number].set_width( All_AOI[box_number].get_width( AOIindex )*( -1 ), AOIindex );
						All_AOI[box_number].set_height( All_AOI[box_number].get_height( AOIindex )*( -1 ), AOIindex );
					}
					// right-top
					else if( corner==2 )
					{
						All_AOI[box_number].set_x( All_AOI[box_number].get_x( AOIindex )+All_AOI[box_number].get_width( AOIindex ), AOIindex );
						All_AOI[box_number].set_width( All_AOI[box_number].get_width( AOIindex )*( -1 ), AOIindex );
					}
					// left-bottom
					else if( corner==3 )
					{
						All_AOI[box_number].set_y( All_AOI[box_number].get_y( AOIindex )+All_AOI[box_number].get_height( AOIindex ), AOIindex );
						All_AOI[box_number].set_height( All_AOI[box_number].get_height( AOIindex )*( -1 ), AOIindex );
					}
				}
				else
				{
					if( corner==1 )
					{
						insertbox.x += insertbox.width;
						insertbox.y += insertbox.height;
						insertbox.width *= ( -1 );
						insertbox.height *= ( -1 );
					}
					else if( corner==2 )
					{
						insertbox.x += insertbox.width;
						insertbox.width *= ( -1 );
					}
					else if( corner==3 )
					{
						insertbox.y += insertbox.height;
						insertbox.height *= ( -1 );
					}
					All_AOI[box_number].AOI_insert( insertbox, AOIindex, video_position );
				}
			}
			//printf("down ");
		}
		break;

		case CV_EVENT_LBUTTONUP://左鍵放開
		{
			if( drawAOI || adjustAOI )
			{
				if( All_AOI[box_number].get_width( AOIindex )<0 )//當調整是往左邊拉且拉過原x點(寬變負的)，重新調整x與width
				{
					All_AOI[box_number].set_x( All_AOI[box_number].get_x( AOIindex )+All_AOI[box_number].get_width( AOIindex ), AOIindex );
					All_AOI[box_number].set_width( All_AOI[box_number].get_width( AOIindex )*( -1 ), AOIindex );
				}

				if( All_AOI[box_number].get_height( AOIindex )<0 )//當調整是往上邊拉且拉過原y點(高變負的)，重新調整y與height
				{
					All_AOI[box_number].set_y( All_AOI[box_number].get_y( AOIindex )+All_AOI[box_number].get_height( AOIindex ), AOIindex );
					All_AOI[box_number].set_height( All_AOI[box_number].get_height( AOIindex )*( -1 ), AOIindex );
				}
				
				if( All_AOI[box_number].get_width( AOIindex )<10 && All_AOI[box_number].get_height( AOIindex )<10 )
				{
					All_AOI[box_number].AOI_erase( AOIindex );
					if( All_AOI[box_number].AOI_size( )==0 ) All_AOI.erase( All_AOI.begin( )+box_number );
				}
								
			}
			
			if(drawAOI || adjustAOI || moveAOI)
			{
				//Auto AOI setting Mode
				//AOI中第i個框被調整，之後的框也設定成需要調正
				for(int i=AOIindex+1; i<All_AOI[box_number].get_isAdjust_size(); i++)
				{
					All_AOI[box_number].set_isAdjust(true, i);
				}
				
				//新創的AOI在 Auto mode 需 initail tracking
				if(isAuto_mode)
				{
					if(drawAOI)//新加入的AOI
					{
						isIniAOItracker.push_back(false);
					}
					AOI_tracking_Initial(M_frame, box_number, AOIindex);
				}
			}
			
			//此AOI物件設定完畢，設初值
			drawAOI=false;
			moveAOI=false;
			adjustAOI=false;
			insertAOI=false;

			//cvCopyImage( origin_image, frame );//frame指向傳進來的圖
			cvCopy( origin_image, frame );
			draw_box( frame, video_position, RED_REC );
			
			mouse_operation_count++;
		}
		break;

		case CV_EVENT_LBUTTONDBLCLK://左鍵雙擊
		{
			box_number = change_name( x, y, video_position );
			if( box_number!=-1 )//點擊在AOI上方，取名
			{
				wxTextEntryDialog *WxTextEntryDialog1;
				WxTextEntryDialog1 = new wxTextEntryDialog(NULL, _(""), _("Please entry the name of the AOI"), _(""));
				//wxMessageBox( wxString::Format( wxT( "%i" ), box_number+1 ) );
				wxString tempname;
				if( All_AOI[box_number].AOI_name.size( )==0 )
					tempname = wxString::Format( wxT( "object%i" ), box_number+1 );
				else
					tempname = wxString( All_AOI[box_number].AOI_name.c_str(), wxConvUTF8);
				WxTextEntryDialog1->SetValue( tempname );
				if( WxTextEntryDialog1->ShowModal( ) )
				{
					tempname = WxTextEntryDialog1->GetValue( );
					All_AOI[box_number].AOI_name = tempname.mb_str();
					//wxMessageBox( tempname );
				}
				//cvCopyImage( origin_image, frame );
				cvCopy( origin_image, frame );
				draw_box( frame, video_position, RED_REC  );
			}
			else//點擊在AOI裡面，刪除
			{
				box_number=mouse_location( x, y, video_position );
				if( box_number!=-1 )
				{
					/*if( All_AOI[box_number].AOI_size( )<=2 )
					{
						All_AOI.erase( All_AOI.begin( )+box_number );
					}
					else
					{
						All_AOI[box_number].AOI_delete( AOIindex );
						if( All_AOI[box_number].AOI_frame( AOIindex-1 )!=( video_position-1 ) )
						{
							All_AOI[box_number].AOI_pushback( insertbox, video_position-1 );
							//printf( "video_position-1:%d\n",video_position-1 );
						}
					}*/
					//將此index 後的AOI index 都刪除
					All_AOI[box_number].AOI_delete( AOIindex );
					//將AOI最後一個frame的位置存入
					if( All_AOI[box_number].AOI_frame( AOIindex-1 )!=( video_position-1 ) /*&& video_position != 0 && AOIindex-1 != -1*/)
					{
						All_AOI[box_number].AOI_pushback( insertbox, video_position-1 );
						
						//fpp << "AOIindex " <<  AOIindex << "  video_position " << video_position << endl;//test write
					}
					
					//fpp << "AOIindex " <<  AOIindex << "  video_position " << video_position << endl;
					
					//tracker clear (不釋放最後用太多程式會爆掉，但用clear()似乎記憶體也沒有釋放，所以tracker重新initail()一個小區塊降低記憶體)
					
					if(isAuto_mode)
					{
						Tracker_space_release(box_number, M_frame);
					}
					
				}
			}
		}
		break;

		case CV_EVENT_RBUTTONDBLCLK://右鍵雙擊
		{
			box_number=mouse_location( x, y, video_position );
			if( box_number!=-1 )
			{
				All_AOI.erase( All_AOI.begin( )+box_number );
			}
		}
		break;
		
		case CV_EVENT_RBUTTONDOWN://右鍵點擊
		{
			;
		}
		break;
		
		case CV_EVENT_MBUTTONDOWN://中鍵點擊
		{
			if(isAuto_mode)
			{
				box_number = mouse_location( x, y, video_position );
				if(All_AOI[box_number].get_AOI_Type() == AUTO)
				{
					All_AOI[box_number].set_AOI_Type( MAMU );
					//wxMessageBox("AOI type set to MAMU type. ");
				}
				else
				{
					All_AOI[box_number].set_AOI_Type( AUTO );
					//wxMessageBox("AOI type set to AUTO type. ");
				}
				cvCopy( origin_image, frame );
				draw_box( frame, video_position, RED_REC );
			}
		}
		break;
		
		case CV_EVENT_MBUTTONDBLCLK://中鍵雙擊
		{
			;
		}
		break;

		default:
			break;
	}
}

//關鍵
int interpolation( int x, int y, int timeX, int timeY, int pos )
{
    return x+( pos-timeX )*( ( double )( y-x )/( timeY-timeX ) );
}

void draw_box( IplImage* img, int pos, int type )
{
    int tempX, tempY;
	int other_tempX, other_tempY;
    int Radius = 10;
    int other_Radius = 5;
	
	//先畫EyePosition
	//之後改這裡... (新增 past_eye_PositionX_frame[], past_eye_PositionY_frame[])
    if( draw_eyePosition )
    {
        tempX = eye_PositionX_frame[pos];
        tempY = eye_PositionY_frame[pos];
		//畫一個人的眼動位置
        if( tempX>=0 && tempY>=0 )
            cvCircle( img, cvPoint( tempX, tempY ), Radius, cvScalar( 0, 255, 0 ), -1);
        
		//畫group的眼動位置
        for(int i=0; i<other_eyePosition_num; i++)
        {
            other_tempX = other_eye_PositionX_frame[i][pos];
            other_tempY = other_eye_PositionY_frame[i][pos];
            if( other_tempX>=0 && other_tempY>=0 )
                cvCircle( img, cvPoint( other_tempX, other_tempY ), other_Radius, cvScalar( 0, 0, 255 ), -1);
        }
    }
    
	//字型定義
    double Scale;
    int Thickness;
    Scale=1;
    Thickness=1;
    CvFont Font1=cvFont(Scale,Thickness);//字型
	
	//碩論實驗相關-比較不同間隔N與N=1(standard)的offset
	int standard_x, standard_y, N_x, N_y;
		
    
	//畫AOI
    //CvRect drawbox=cvRect( 0, 0, 0, 0 );
    int AOIx, AOIy, AOIwidth, AOIheight, framePos, AOIx_l, AOIy_l, AOIwidth_l, AOIheight_l, framePos_l, lastframePos;
    for( int i=0 ; i<All_AOI.size( ) ; i++ )
    {
        CvRect drawbox=cvRect( 0, 0, 0, 0 );
		lastframePos=All_AOI[i].AOI_frame( All_AOI[i].AOI_frame_size( )-1 );
        for( int j=0 ; j<All_AOI[i].AOI_frame_size( ) ; j++ )
        {
            AOIx=All_AOI[i].get_x( j );
            AOIy=All_AOI[i].get_y( j );
            AOIwidth=All_AOI[i].get_width( j );
            AOIheight=All_AOI[i].get_height( j );
            framePos=All_AOI[i].AOI_frame( j );
            //lastframePos=All_AOI[i].AOI_frame( All_AOI[i].AOI_frame_size( )-1 );
            if( j>=1 )
            {
                // _l means last
                AOIx_l=All_AOI[i].get_x( j-1 );
                AOIy_l=All_AOI[i].get_y( j-1 );
                AOIwidth_l=All_AOI[i].get_width( j-1 );
                AOIheight_l=All_AOI[i].get_height( j-1 );
                framePos_l=All_AOI[i].AOI_frame( j-1 );
            }

            if( pos<framePos && j==0 ) break;
            else if( pos>lastframePos && lastframePos!=-1 ) break;
            else if( pos<framePos && j!=0 )
            {
                drawbox.x=interpolation( AOIx_l, AOIx, framePos_l, framePos, pos );
                drawbox.y=interpolation( AOIy_l, AOIy, framePos_l, framePos, pos );
                drawbox.width=interpolation( AOIwidth_l, AOIwidth, framePos_l, framePos, pos );
                drawbox.height=interpolation( AOIheight_l, AOIheight, framePos_l, framePos, pos );
                break;
            }
            else if( pos==framePos )
            {
                //printf("pos:%d\n",pos);
                drawbox.x=AOIx;
                drawbox.y=AOIy;
                drawbox.width=AOIwidth;
                drawbox.height=AOIheight;
                break;
            }
            else if( pos>framePos && framePos!=-1 ) continue;
            else if( pos>framePos && framePos==-1 )
            {
                drawbox.x=AOIx_l;
                drawbox.y=AOIy_l;
                drawbox.width=AOIwidth_l;
                drawbox.height=AOIheight_l;
                break;
            }
        }
		
		//碩論實驗相關-比較不同間隔N與N=1(standard)的offset
		standard_x = drawbox.x; 
		standard_y = drawbox.y;
		
		
		//再原frame上畫AOI
        if( ( drawbox.width>0 && drawbox.height>0 ) || adjustAOI || drawAOI )
        {
            char str_i[11];
            sprintf(str_i, "object%d", i+1);
            
            if( draw_eyePosition )
            {
                CvScalar Color;
                if( tempX>=drawbox.x && tempX<=drawbox.x+drawbox.width && tempY>=drawbox.y && tempY<=drawbox.y+drawbox.height )
                    Color = cvScalar( 0, 255, 0 );
                else
                    Color = cvScalar( 0, 0, 255 );
                cvRectangle( img, cvPoint( drawbox.x, drawbox.y ), cvPoint( drawbox.x+drawbox.width,drawbox.y+drawbox.height ), Color ,2 );
                
                if( !( drawAOI || adjustAOI || moveAOI ) )
                {
                    if( All_AOI[i].AOI_name.size()==0 )
                        cvPutText( img, str_i, cvPoint( drawbox.x, drawbox.y-5 ), &Font1, Color );
                    else
                        cvPutText( img, All_AOI[i].AOI_name.c_str( ), cvPoint( drawbox.x, drawbox.y-5 ), &Font1, Color );
                }
            }
            else
            {
				if(All_AOI[i].get_AOI_Type() == AUTO && !videoanalyzer_mode)
				{
					if(type == PURPLE_REC)
					{
						cvRectangle(img, cvPoint( drawbox.x, drawbox.y ), cvPoint( drawbox.x+drawbox.width,drawbox.y+drawbox.height ), cvScalar( 255, 0, 255 ) ,2 );
					}
					else
					{
						cvRectangle(img, cvPoint( drawbox.x, drawbox.y ), cvPoint( drawbox.x+drawbox.width,drawbox.y+drawbox.height ), cvScalar( 255, 0, 125 ) ,2 );
					}
				}
				else
				{
					cvRectangle(img, cvPoint( drawbox.x, drawbox.y ), cvPoint( drawbox.x+drawbox.width,drawbox.y+drawbox.height ), cvScalar( 0, 0, 255 ) ,2 );
					
					//碩論實驗相關-比較不同間隔N與N=1(standard)的offset
					standard_N_frame_pos++;
				}
                //cvRectangle(img, cvPoint( drawbox.x, drawbox.y ), cvPoint( drawbox.x+drawbox.width,drawbox.y+drawbox.height ), cvScalar( 0, 0, 255 ) ,2 );
				
                if( !( drawAOI || adjustAOI || moveAOI ) )
                {
                    if( All_AOI[i].AOI_name.size()==0 )
                        cvPutText( img, str_i, cvPoint( drawbox.x, drawbox.y-5 ), &Font1, Scalar( 0 , 0, 255 ) );
                    else
                        cvPutText( img, All_AOI[i].AOI_name.c_str( ), cvPoint( drawbox.x, drawbox.y-5 ), &Font1, Scalar( 0, 0, 255 ) );
                }
            }
        }
    }
	
	//畫 other AOI
	for(int k=0; k<other_AOI_path_num; k++)
	{
		for( int i=0 ; i<other_All_AOI[k].size( ) ; i++ )
		{
			CvRect drawbox=cvRect( 0, 0, 0, 0 );
			lastframePos=other_All_AOI[k][i].AOI_frame( other_All_AOI[k][i].AOI_frame_size( )-1 );
			for( int j=0 ; j<other_All_AOI[k][i].AOI_frame_size( ) ; j++ )
			{
				AOIx=other_All_AOI[k][i].get_x( j );
				AOIy=other_All_AOI[k][i].get_y( j );
				AOIwidth=other_All_AOI[k][i].get_width( j );
				AOIheight=other_All_AOI[k][i].get_height( j );
				framePos=other_All_AOI[k][i].AOI_frame( j );
				//lastframePos=All_AOI[i].AOI_frame( All_AOI[i].AOI_frame_size( )-1 );
				if( j>=1 )
				{
					// _l means last
					AOIx_l=other_All_AOI[k][i].get_x( j-1 );
					AOIy_l=other_All_AOI[k][i].get_y( j-1 );
					AOIwidth_l=other_All_AOI[k][i].get_width( j-1 );
					AOIheight_l=other_All_AOI[k][i].get_height( j-1 );
					framePos_l=other_All_AOI[k][i].AOI_frame( j-1 );
				}

				if( pos<framePos && j==0 ) break;
				else if( pos>lastframePos && lastframePos!=-1 ) break;
				else if( pos<framePos && j!=0 )
				{
					drawbox.x=interpolation( AOIx_l, AOIx, framePos_l, framePos, pos );
					drawbox.y=interpolation( AOIy_l, AOIy, framePos_l, framePos, pos );
					drawbox.width=interpolation( AOIwidth_l, AOIwidth, framePos_l, framePos, pos );
					drawbox.height=interpolation( AOIheight_l, AOIheight, framePos_l, framePos, pos );
					break;
				}
				else if( pos==framePos )
				{
					//printf("pos:%d\n",pos);
					drawbox.x=AOIx;
					drawbox.y=AOIy;
					drawbox.width=AOIwidth;
					drawbox.height=AOIheight;
					break;
				}
				else if( pos>framePos && framePos!=-1 ) continue;
				else if( pos>framePos && framePos==-1 )
				{
					drawbox.x=AOIx_l;
					drawbox.y=AOIy_l;
					drawbox.width=AOIwidth_l;
					drawbox.height=AOIheight_l;
					break;
				}
			}
			
			//碩論實驗相關-比較不同間隔N與N=1(standard)的offset
			N_x = drawbox.x;
			N_y = drawbox.y;
						
			//再原frame上畫AOI
			int color_arr[5][3] = {{255,0,0},{0,255,0},{255,0,255},{255,255,0},{0,255,255}};//blue, green, purple, cyan, yellow
			if( ( drawbox.width>0 && drawbox.height>0 ) || adjustAOI || drawAOI )
			{
				char str_i[11];
				sprintf(str_i, "object%d", i+1);
				
				if( draw_eyePosition )
				{
					CvScalar Color;
					if( tempX>=drawbox.x && tempX<=drawbox.x+drawbox.width && tempY>=drawbox.y && tempY<=drawbox.y+drawbox.height )
						Color = cvScalar( 0, 255, 0 );
					else
						Color = cvScalar( 0, 0, 255 );
					cvRectangle( img, cvPoint( drawbox.x, drawbox.y ), cvPoint( drawbox.x+drawbox.width,drawbox.y+drawbox.height ), Color ,2 );
					
					if( !( drawAOI || adjustAOI || moveAOI ) )
					{
						if( other_All_AOI[k][i].AOI_name.size()==0 )
							cvPutText( img, str_i, cvPoint( drawbox.x, drawbox.y-5 ), &Font1, Color );
						else
							cvPutText( img, other_All_AOI[k][i].AOI_name.c_str( ), cvPoint( drawbox.x, drawbox.y-5 ), &Font1, Color );
					}
				}
				else
				{
					if(other_All_AOI[k][i].get_AOI_Type() == AUTO && !videoanalyzer_mode)
					{
						if(type == PURPLE_REC)
						{
							cvRectangle(img, cvPoint( drawbox.x, drawbox.y ), cvPoint( drawbox.x+drawbox.width,drawbox.y+drawbox.height ), cvScalar( 255, 0, 255 ) ,2 );
						}
						else
						{
							cvRectangle(img, cvPoint( drawbox.x, drawbox.y ), cvPoint( drawbox.x+drawbox.width,drawbox.y+drawbox.height ), cvScalar( 255, 0, 125 ) ,2 );
						}
					}
					else
					{
						cvRectangle(img, cvPoint( drawbox.x, drawbox.y ), cvPoint( drawbox.x+drawbox.width,drawbox.y+drawbox.height ), cvScalar( color_arr[k][0], color_arr[k][1], color_arr[k][2] ) ,2 );
						
						//碩論實驗相關-比較不同間隔N與N=1(standard)的offset
						//fp_standard_N << ", offset_x, offset_y, distance"
						Caculate_offset_distance(standard_x, standard_y, N_x, N_y);
						
						if(k==other_AOI_path_num-1)
						{
							fp_standard_N << "\n";
						}
					}
					//cvRectangle(img, cvPoint( drawbox.x, drawbox.y ), cvPoint( drawbox.x+drawbox.width,drawbox.y+drawbox.height ), cvScalar( 0, 0, 255 ) ,2 );
					
					if( !( drawAOI || adjustAOI || moveAOI ) )
					{
						if( other_All_AOI[k][i].AOI_name.size()==0 )
							cvPutText( img, str_i, cvPoint( drawbox.x, drawbox.y-5 ), &Font1, Scalar( 0 , 0, 255 ) );
						else
							cvPutText( img, other_All_AOI[k][i].AOI_name.c_str( ), cvPoint( drawbox.x, drawbox.y-5 ), &Font1, Scalar( 0, 0, 255 ) );
					}
				}
			}
		}
	}
		
}

//碩論實驗相關-比較不同間隔N與N=1(standard)的offset
void Caculate_offset_distance(int standard_x, int standard_y, int N_x, int N_y)
{
	int offset_x = N_x - standard_x; 
	int offset_y = N_y - standard_y;
	float distance = (float) sqrt(pow(abs(offset_x), 2) + pow(abs(offset_y), 2));
	
	fp_standard_N << standard_N_frame_pos << "," << offset_x << "," << offset_y << "," << distance << ",,";

	return;
}

void draw_HeatMap( IplImage* img, int pos )
{
	//字型定義
    double Scale;
    int Thickness;
    Scale=1;
    Thickness=1;
    CvFont Font1=cvFont(Scale,Thickness);//字型
    
	//畫AOI
    //CvRect drawbox=cvRect( 0, 0, 0, 0 );
    int AOIx, AOIy, AOIwidth, AOIheight, framePos, AOIx_l, AOIy_l, AOIwidth_l, AOIheight_l, framePos_l, lastframePos;
    for( int i=0 ; i<All_AOI.size( ) ; i++ )
    {
        CvRect drawbox=cvRect( 0, 0, 0, 0 );
		lastframePos=All_AOI[i].AOI_frame( All_AOI[i].AOI_frame_size( )-1 );
        for( int j=0 ; j<All_AOI[i].AOI_frame_size( ) ; j++ )
        {
            AOIx=All_AOI[i].get_x( j );
            AOIy=All_AOI[i].get_y( j );
            AOIwidth=All_AOI[i].get_width( j );
            AOIheight=All_AOI[i].get_height( j );
            framePos=All_AOI[i].AOI_frame( j );
            //lastframePos=All_AOI[i].AOI_frame( All_AOI[i].AOI_frame_size( )-1 );
            if( j>=1 )
            {
                // _l means last
                AOIx_l=All_AOI[i].get_x( j-1 );
                AOIy_l=All_AOI[i].get_y( j-1 );
                AOIwidth_l=All_AOI[i].get_width( j-1 );
                AOIheight_l=All_AOI[i].get_height( j-1 );
                framePos_l=All_AOI[i].AOI_frame( j-1 );
            }

            if( pos<framePos && j==0 ) break;
            else if( pos>lastframePos && lastframePos!=-1 ) break;
            else if( pos<framePos && j!=0 )
            {
                drawbox.x=interpolation( AOIx_l, AOIx, framePos_l, framePos, pos );
                drawbox.y=interpolation( AOIy_l, AOIy, framePos_l, framePos, pos );
                drawbox.width=interpolation( AOIwidth_l, AOIwidth, framePos_l, framePos, pos );
                drawbox.height=interpolation( AOIheight_l, AOIheight, framePos_l, framePos, pos );
                break;
            }
            else if( pos==framePos )
            {
                //printf("pos:%d\n",pos);
                drawbox.x=AOIx;
                drawbox.y=AOIy;
                drawbox.width=AOIwidth;
                drawbox.height=AOIheight;
                break;
            }
            else if( pos>framePos && framePos!=-1 ) continue;
            else if( pos>framePos && framePos==-1 )
            {
                drawbox.x=AOIx_l;
                drawbox.y=AOIy_l;
                drawbox.width=AOIwidth_l;
                drawbox.height=AOIheight_l;
                break;
            }
        }
		
		//再原frame上畫AOI
        if( ( drawbox.width>0 && drawbox.height>0 ) || adjustAOI || drawAOI )
        {
            char str_i[11];
            sprintf(str_i, "object%d", i+1);
			
			cvRectangle(img, cvPoint( drawbox.x, drawbox.y ), cvPoint( drawbox.x+drawbox.width,drawbox.y+drawbox.height ), cvScalar( 0, 0, 255 ) ,2 );
				
             //cvRectangle(img, cvPoint( drawbox.x, drawbox.y ), cvPoint( drawbox.x+drawbox.width,drawbox.y+drawbox.height ), cvScalar( 0, 0, 255 ) ,2 );
				
            if( !( drawAOI || adjustAOI || moveAOI ) )
            {
                if( All_AOI[i].AOI_name.size()==0 )
                    cvPutText( img, str_i, cvPoint( drawbox.x, drawbox.y-5 ), &Font1, Scalar( 0 , 0, 255 ) );
                else
                    cvPutText( img, All_AOI[i].AOI_name.c_str( ), cvPoint( drawbox.x, drawbox.y-5 ), &Font1, Scalar( 0, 0, 255 ) );
            }
        }
    }
	
	

	
    // Create the heatmap object with the given dimensions (in pixel).
    heatmap_t* hm = heatmap_new(frame_width, frame_height);   
    
    // Add a bunch of random points to the heatmap now.
    heatmap_stamp_t* stamp = heatmap_stamp_gen(heatmap_Radius);
    //srand(time(NULL));
    for(int i = 0 ; i < other_eyePosition_num ; ++i) 
    {
        //int a=rand()%width;
        //int b=rand()%height;
        heatmap_add_point_with_stamp(hm, (unsigned int)other_eye_PositionX_frame[i][pos], (unsigned int) other_eye_PositionY_frame[i][pos], stamp);
    }
    
    
    // This creates an image out of the heatmap.
    // `image` now contains the image data in 32-bit RGBA.
    vector<unsigned char> frame_vector(frame_width*frame_height*4);
    heatmap_render_to(hm, heatmap_cs_Spectral_soft, &frame_vector[0]);
    
    // Now that we've got a finished heatmap picture, we don't need the map anymore.
    heatmap_free(hm);
    
    int Alpha=255;
    for(int i=0;i<img->height;i++)
    {
        for(int j=0, k=0;j<img->widthStep;j=j+3, k++)
        {
            if(frame_vector[i*frame_width*4 +k*4]>0 || frame_vector[i*frame_width*4 +k*4+1]>0 || frame_vector[i*frame_width*4 +k*4+2]>0)//若 frame_vector中有值(即熱點的範圍)才進行運算 
            {
                
                //半透明 
                int R = (int)((img->imageData[i*img->widthStep + j+2]*(255-Alpha)) + (((int)frame_vector[i*frame_width*4 +k*4])*(Alpha)))/255;
                int G = (int)((img->imageData[i*img->widthStep + j+1]*(255-Alpha)) + (((int)frame_vector[i*frame_width*4 +k*4+1])*(Alpha)))/255;
                int B = (int)((img->imageData[i*img->widthStep + j]*(255-Alpha)) + (((int)frame_vector[i*frame_width*4 +k*4+2])*(Alpha)))/255;
                img->imageData[i*img->widthStep + j+2] = R > 255? 255:R;//R
                img->imageData[i*img->widthStep + j+1] = G > 255? 255:G;//G
                img->imageData[i*img->widthStep + j] = B > 255? 255:B;//B
                
                /*
                img->imageData[i*img->widthStep + j+2] = frame_vector[i*frame_width*4 +k*4];//R
                img->imageData[i*img->widthStep + j+1]=frame_vector[i*frame_width*4 +k*4 +1];//G
                img->imageData[i*img->widthStep + j]=frame_vector[i*frame_width*4 +k*4 +2];//B
                */
                
            }
        }
    }
}

// where is the mouse, return box number if mouse in the box. Create a new box else and return -1.
int mouse_location( int x, int y, int pos )
{
    int AOIx, AOIy, AOIwidth, AOIheight, framePos, AOIx_l, AOIy_l, AOIwidth_l, AOIheight_l, framePos_l;
    for( int i=0 ; i<All_AOI.size( ) ; i++ )
    {
        //AOIx=0, AOIy=0, AOIwidth=0, AOIheight=0;
        for( int j=0 ; j<All_AOI[i].AOI_frame_size( ) ; j++ )
        {
            AOIx=All_AOI[i].get_x( j );
            AOIy=All_AOI[i].get_y( j );
            AOIwidth=All_AOI[i].get_width( j );
            AOIheight=All_AOI[i].get_height( j );
            framePos=All_AOI[i].AOI_frame( j );
            if( j>=1 )
            {
                // _l means last
                AOIx_l=All_AOI[i].get_x( j-1 );
                AOIy_l=All_AOI[i].get_y( j-1 );
                AOIwidth_l=All_AOI[i].get_width( j-1 );
                AOIheight_l=All_AOI[i].get_height( j-1 );
                framePos_l=All_AOI[i].AOI_frame( j-1 );
            }

            if( pos<framePos && j==0 ) break;//在AOI開始的frame pos之前
            else if( pos<framePos && j!=0 )////在AOI開始的frame pos(j-1)之後且在地j個frame pos之前(用內插算AOI位置大小位置大小)
            {
                int x_inter=interpolation( AOIx_l, AOIx, framePos_l, framePos, pos );
                int y_inter=interpolation( AOIy_l, AOIy, framePos_l, framePos, pos );
                int width_inter=interpolation( AOIwidth_l, AOIwidth, framePos_l, framePos, pos );
                int height_inter=interpolation( AOIheight_l, AOIheight, framePos_l, framePos, pos );

                if ( x>=x_inter && y>=y_inter && x<=x_inter+width_inter && y<=y_inter+height_inter )
                {
                    insertAOI=true;
                    insertbox.x=x_inter;
                    insertbox.y=y_inter;
                    insertbox.width=width_inter;
                    insertbox.height=height_inter;
                    AOIindex=j;
                    return i;
                }
                else break;
            }
            else if( pos==framePos )//在AOI第j個pos上
            {
                if( x>=AOIx && y>=AOIy && x<=AOIx+AOIwidth && y<=AOIy+AOIheight )
                {
                    AOIindex=j;
                    return i;
                }
                else break;
            }
            else if( pos>framePos && framePos!=-1 ) continue;//在AOI結束之後的frame上
            else if( pos>framePos && framePos==-1 )//AOI尚未設定結束
            {
                if( x>=AOIx_l && y>=AOIy_l && x<=AOIx_l+AOIwidth_l && y<=AOIy_l+AOIheight_l )
                {
                    insertAOI=true;
                    insertbox.x=AOIx_l;
                    insertbox.y=AOIy_l;
                    insertbox.width=AOIwidth_l;
                    insertbox.height=AOIheight_l;
                    AOIindex=j;
                    return i;
                }
                else break;
            }
        }
    }
    return -1;
}

// return whether is the mouse cursor nearby a corner in a AOI or not(判斷是調整或移動AOI ex:接近角落則是調整AOI)
int nearby_corner( int x, int y, int pos )
{
    int near_dis = 20;
    int tempbox_x, tempbox_y, tempbox_width, tempbox_height;
    int tempbox_number = mouse_location( x, y, pos );
    if( tempbox_number==-1 )
        return -1;
    else
    {
        if( insertAOI )
        {
            tempbox_x=insertbox.x;
            tempbox_y=insertbox.y;
            tempbox_width=insertbox.width;
            tempbox_height=insertbox.height;
        }
        else
        {
            tempbox_x=All_AOI[tempbox_number].get_x( AOIindex );
            tempbox_y=All_AOI[tempbox_number].get_y( AOIindex );
            tempbox_width=All_AOI[tempbox_number].get_width( AOIindex );
            tempbox_height=All_AOI[tempbox_number].get_height( AOIindex );
        }

        if( x>=tempbox_x && x<=tempbox_x+near_dis )
        {
            if( y>=tempbox_y && y<=tempbox_y+near_dis )
                return 1; // left-up
            else if( y>=tempbox_y+tempbox_height-near_dis && y<=tempbox_y+tempbox_height )
                return 2; // left-bottom
        }
        else if( x>=tempbox_x+tempbox_width-near_dis && x<=tempbox_x+tempbox_width )
        {
            if( y>=tempbox_y && y<=tempbox_y+near_dis )
                return 3; // right-up
            else if( y>=tempbox_y+tempbox_height-near_dis && y<=tempbox_y+tempbox_height )
                return 4; // right-bottom
        }
    }
	
    return -1;
}

// judge whether is the mouse cursor in a name area of a AOI and return which AOI
int change_name( int x, int y, int pos )
{
    int AOIx, AOIy, AOIwidth, AOIheight, framePos, AOIx_l, AOIy_l, AOIwidth_l, AOIheight_l, framePos_l;
    for( int i=0 ; i<All_AOI.size( ) ; i++ )
    {
        //AOIx=0, AOIy=0, AOIwidth=0, AOIheight=0;
        for( int j=0 ; j<All_AOI[i].AOI_frame_size( ) ; j++ )//AOI_frame_vector size, AOI_frame_vector: "哪個frame有設定過"
        {
            AOIx=All_AOI[i].get_x( j );
            AOIy=All_AOI[i].get_y( j );
            AOIwidth=All_AOI[i].get_width( j );
            AOIheight=All_AOI[i].get_height( j );
            framePos=All_AOI[i].AOI_frame( j );
            if( j>=1 )
            {
                // _l means last
                AOIx_l=All_AOI[i].get_x( j-1 );
                AOIy_l=All_AOI[i].get_y( j-1 );
                AOIwidth_l=All_AOI[i].get_width( j-1 );
                AOIheight_l=All_AOI[i].get_height( j-1 );
                framePos_l=All_AOI[i].AOI_frame( j-1 );
            }

            if( pos<framePos && j==0 ) break;
            else if( pos<framePos && j!=0 )
            {
                int x_inter=interpolation( AOIx_l, AOIx, framePos_l, framePos, pos );
                int y_inter=interpolation( AOIy_l, AOIy, framePos_l, framePos, pos );
                int width_inter=interpolation( AOIwidth_l, AOIwidth, framePos_l, framePos, pos );
                int height_inter=interpolation( AOIheight_l, AOIheight, framePos_l, framePos, pos );

                if ( x>x_inter && y>y_inter-20 && x<x_inter+width_inter && y<y_inter )
                {
                    insertAOI=true;
                    insertbox.x=x_inter;
                    insertbox.y=y_inter;
                    insertbox.width=width_inter;
                    insertbox.height=height_inter;
                    AOIindex=j;
                    return i;
                }
                else break;
            }
            else if( pos==framePos )
            {
                if( x>AOIx && y>AOIy-20 && x<AOIx+AOIwidth && y<AOIy )
                {
                    AOIindex=j;
                    return i;
                }
                else break;
            }
            else if( pos>framePos && framePos!=-1 ) continue;
            else if( pos>framePos && framePos==-1 )
            {
                if( x>AOIx_l && y>AOIy_l-20 && x<AOIx_l+AOIwidth_l && y<AOIy_l )
                {
                    insertAOI=true;
                    insertbox.x=AOIx_l;
                    insertbox.y=AOIy_l;
                    insertbox.width=AOIwidth_l;
                    insertbox.height=AOIheight_l;
                    AOIindex=j;
                    return i;
                }
                else break;
            }
        }
    }
    return -1;
}

void outputAOI( )
{
    for( int i=0 ; i<All_AOI.size() ; i++ )
    {
        for( int j=0 ; j<All_AOI[i].AOI_size( ) ; j++ )//position size
        {
            if( All_AOI[i].get_width( j )<=0 || All_AOI[i].get_height( j )<=0 )
            {
                All_AOI.erase( All_AOI.begin( )+i );
                i--;
            }
        }
    }
    
    string file_name;
    file_name += "dynamicAOI_";
    file_name += video_name.mb_str( );
    file_name += ".txt";
    fstream file;
    file.open( file_name.c_str( ), ios::out | ios::trunc );

    for( int i=0 ; i<All_AOI.size( ) ; i++ )
    {
        if( All_AOI[i].AOI_name.size( )!=0 )
            file << '$' << All_AOI[i].AOI_name << '$';
        else
            file << '$' << "object" << i+1 << '$';
            
        for( int j=0 ; j<All_AOI[i].AOI_frame_size( ) ; j++ )
        {
            if( j!=All_AOI[i].AOI_frame_size( )-1 )
                file << All_AOI[i].AOI_frame( j ) << ',';
            else
                file << All_AOI[i].AOI_frame( j ) << ',' << ';';
        }
        
        for( int j=0 ; j<All_AOI[i].AOI_size( ) ; j++ )
        {
            file << All_AOI[i].get_x( j ) << '&'
                 << All_AOI[i].get_y( j ) << '&'
                 << All_AOI[i].get_width( j ) << '&'
                 << All_AOI[i].get_height( j ) << '&';
            if( j!=All_AOI[i].AOI_size( )-1 )
                file << ',';
            else
                file << ',' << ';';
        }
        file << "\n#";
    }
    file.close( );
}

//執行Analysis前載入AOI資料
void inputAOI( wxString dynamic_AOI )
{
    fstream file;
    file.open( dynamic_AOI, ios::in );
    
    string Str, tempStr, tempname;
    int temp;
    vector<int> AOI_frame_vector;
    vector<int> temp_CvRect;
    vector<CvRect> AOI_position;
    bool AOI_position_input = true;
    bool AOI_name_input = false;
    // creat a empty class AOI
    AOI tempAOI = AOI( 0, 0, 0 );
    tempAOI.All_clear( );
    CvRect tempbox;
    
    while( file >> tempStr )
    {
        Str += tempStr;
    }
    tempStr = Str;
    
    Str = "";
    //cout << Str << endl;
    //cout << tempStr << endl << tempStr.length();
    
    for( int i=0 ; i<tempStr.length( ) ; i++ )
    {
        //cout << "step:" << i << endl;
        if( tempStr[i]!='#' )
        {
            if( tempStr[i]=='$' )//代表之後接的是AOI_nname 
            {
                if( AOI_name_input )
                {
                    tempname = Str;
                    Str="";
                    AOI_name_input = !AOI_name_input;
                }
                else
                {
                    tempname="";
                    AOI_name_input = !AOI_name_input;
                }
            }
            //cout << "    !='#'";
            else if( tempStr[i]==',' )//代表上一個字元是 "數字(frame)" 
            {
                //cout << "    ==','";
                
                if( AOI_position_input )
                {
                    temp = atoi( Str.c_str( ) );
                    //cout << "  " << temp;
                    Str = "";
                    AOI_frame_vector.push_back( temp );
                }
                else
                {
                    tempbox.x = temp_CvRect[0];
                    tempbox.y = temp_CvRect[1];
                    tempbox.width = temp_CvRect[2];
                    tempbox.height = temp_CvRect[3];
                    AOI_position.push_back( tempbox );
                    temp_CvRect.clear( );
                }
            }
            else if( tempStr[i]=='&' )//AOI Rect 的四個數字(左上點x, y, width, height) 
            {
                //cout << "    =='&'";
                temp = atoi( Str.c_str( ) );
                //cout << "  " << temp;
                Str = "";
                temp_CvRect.push_back( temp );
            }
            else if( tempStr[i]==';' )//數字(frame)以抓取完畢 
            {
                //cout << "    ==';'";
                AOI_position_input = !AOI_position_input;
            }
            else
            {
                //cout << "    nothing";
                Str += tempStr[i];
            }
        }
        else // tempStr=='#'//特定object資訊讀取結束(一行)，全部存到全域變數 
        {
            //cout << "    =='#'";
            //cout << "0";
            tempAOI.input_txt( tempname, AOI_frame_vector, AOI_position );//AOI_frame_vector: "哪個frame有設定過"
            All_AOI.push_back( tempAOI );
            AOI_frame_vector.clear( );
            AOI_position.clear( );
            tempAOI.All_clear( );
            Str = "";
        }
        //cout << endl;
    }
}

void input_other_AOI( wxArrayString other_AOI_path )
{
	other_All_AOI.resize(other_AOI_path_num);
		
	fstream file;
	
	for(int k=0; k<other_AOI_path_num; k++)
	{
		file.open( other_AOI_path[k], ios::in );
		
		string Str, tempStr, tempname;
		int temp;
		vector<int> AOI_frame_vector;
		vector<int> temp_CvRect;
		vector<CvRect> AOI_position;
		bool AOI_position_input = true;
		bool AOI_name_input = false;
		// creat a empty class AOI
		AOI tempAOI = AOI( 0, 0, 0 );
		CvRect tempbox;
		
		tempAOI.All_clear( );
		
		while( file >> tempStr )
		{
			Str += tempStr;
		}
		tempStr = Str;
		Str = "";
		file.close();
		//cout << Str << endl;
		//cout << tempStr << endl << tempStr.length();
		
		for( int i=0 ; i<tempStr.length( ) ; i++ )
		{
			//cout << "step:" << i << endl;
			if( tempStr[i]!='#' )
			{
				if( tempStr[i]=='$' )//代表之後接的是AOI_nname 
				{
					if( AOI_name_input )
					{
						tempname = Str;
						Str="";
						AOI_name_input = !AOI_name_input;
					}
					else
					{
						tempname="";
						AOI_name_input = !AOI_name_input;
					}
				}
				//cout << "    !='#'";
				else if( tempStr[i]==',' )//代表上一個字元是 "數字(frame)" 
				{
					//cout << "    ==','";
					
					if( AOI_position_input )
					{
						temp = atoi( Str.c_str( ) );
						//cout << "  " << temp;
						Str = "";
						AOI_frame_vector.push_back( temp );
					}
					else
					{
						tempbox.x = temp_CvRect[0];
						tempbox.y = temp_CvRect[1];
						tempbox.width = temp_CvRect[2];
						tempbox.height = temp_CvRect[3];
						AOI_position.push_back( tempbox );
						temp_CvRect.clear( );
					}
				}
				else if( tempStr[i]=='&' )//AOI Rect 的四個數字(左上點x, y, width, height) 
				{
					//cout << "    =='&'";
					temp = atoi( Str.c_str( ) );
					//cout << "  " << temp;
					Str = "";
					temp_CvRect.push_back( temp );
				}
				else if( tempStr[i]==';' )//數字(frame)以抓取完畢 
				{
					//cout << "    ==';'";
					AOI_position_input = !AOI_position_input;
				}
				else
				{
					//cout << "    nothing";
					Str += tempStr[i];
				}
			}
			else // tempStr=='#'//特定object資訊讀取結束(一行)，全部存到全域變數 
			{
				//cout << "    =='#'";
				//cout << "0";
				tempAOI.input_txt( tempname, AOI_frame_vector, AOI_position );//AOI_frame_vector: "哪個frame有設定過"
				other_All_AOI[k].push_back( tempAOI );
				AOI_frame_vector.clear( );
				AOI_position.clear( );
				tempAOI.All_clear( );
				Str = "";
			}
			//cout << endl;
		}
	}
}

//執行Analysis前載入EyePosition資料
void inputEyePosition( wxString eye_Position )
{
    fstream file;
    file.open( eye_Position, ios::in );
    /*if( !file )
    {
        cerr << "Can't open file!\n";
        exit( 1 );
    }*/
    
    string Str, tempStr;
    int temp;
    bool isX = true;
    bool AOI_time = true;
    
    eye_time.clear( );
    eye_PositionX.clear( );
    eye_PositionY.clear( );
    
    while( file >> tempStr )
    {
        Str += tempStr;
    }
    tempStr = Str;
    
    Str = "";
    //cout << Str << endl;
    //cout << tempStr << endl << tempStr.length();
    
    for( int i=0 ; i<tempStr.length( ) ; i++ )
    {
        if( tempStr[i]==',' )
        {
            temp = atoi( Str.c_str( ) );
            Str = "";
                
            if( AOI_time )
            {
                eye_time.push_back( temp );
                //cout << "\ntime: " << temp;
            }
            else
            {
                if( isX )
                {
                    //cout << "  X: " << temp;
                    eye_PositionX.push_back( temp );
                    isX = !isX;
                }
                else
                {
                    //cout << "  Y: " << temp;
                    eye_PositionY.push_back( temp );
                    isX = !isX;
                }
            }
        }
        else if( tempStr[i]==';' )
        {
            //cout << "  ;";
            AOI_time = !AOI_time;
        }
        else
        {
            Str += tempStr[i];
        }
    }
    
    file.close( );
    
    eye_PositionX_frame.clear( );
    eye_PositionY_frame.clear( );
    for( int i=1 ; i<=frames ; i++ )
    {
        for( int j=0 ; j<eye_time.size( ) ; j++ )
        {
            if( int(mspf*i)<eye_time[j] )
            {
                eye_PositionX_frame.push_back( -1 );
                eye_PositionY_frame.push_back( -1 );
                break;
            }
            else if( eye_time[j+1]>( int(mspf*i) ) && ( int(mspf*i) )>=eye_time[j] )
            {
                eye_PositionX_frame.push_back( eye_PositionX[j] );
                eye_PositionY_frame.push_back( eye_PositionY[j] );
                break;
            }
            else if( j==eye_time.size( )-1 )
            {
                eye_PositionX_frame.push_back( -1 );
                eye_PositionY_frame.push_back( -1 );
                break;
            }
        }
    }
    
    /*fstream file2;
    file2.open( "test.txt", ios::out | ios::trunc );
    
    for( int i=1 ; i<=frames ; i++ )
    {
        file2 << i << ':' << eye_PositionX_frame[i] << ',' << eye_PositionY_frame[i] << "\n";
    }

    file2.close( );*/
}

//執行Analysis前載入other_EyePosition資料
void input_otherEyePosition( wxArrayString other_eyePosition )
{
    fstream file;
    other_eye_time.resize(other_eyePosition_num);
    other_eye_PositionX.resize(other_eyePosition_num);
    other_eye_PositionY.resize(other_eyePosition_num);
    other_eye_PositionX_frame.resize(other_eyePosition_num);
    other_eye_PositionY_frame.resize(other_eyePosition_num);
    
    
    for(int k=0; k<other_eyePosition_num; k++)
    {
        file.open( other_eyePosition[k], ios::in );
        /*if( !file )
        {
            cerr << "Can't open file!\n";
            exit( 1 );
        }*/
        
        string Str, tempStr;
        int temp;
        bool isX = true;
        bool AOI_time = true;
        
        other_eye_time[k].clear( );
        other_eye_PositionX[k].clear( );
        other_eye_PositionY[k].clear( );
        
        while( file >> tempStr )
        {
            Str += tempStr;
        }
        tempStr = Str;
        
        Str = "";
        //cout << Str << endl;
        //cout << tempStr << endl << tempStr.length();
        
        for( int i=0 ; i<tempStr.length( ) ; i++ )
        {
            if( tempStr[i]==',' )
            {
                temp = atoi( Str.c_str( ) );
                Str = "";
                    
                if( AOI_time )
                {
                    other_eye_time[k].push_back( temp );
                    //cout << "\ntime: " << temp;
                }
                else
                {
                    if( isX )
                    {
                        //cout << "  X: " << temp;
                        other_eye_PositionX[k].push_back( temp );
                        isX = !isX;
                    }
                    else
                    {
                        //cout << "  Y: " << temp;
                        other_eye_PositionY[k].push_back( temp );
                        isX = !isX;
                    }
                }
            }
            else if( tempStr[i]==';' )
            {
                //cout << "  ;";
                AOI_time = !AOI_time;
            }
            else
            {
                Str += tempStr[i];
            }
        }
        
        file.close( );
        
        other_eye_PositionX_frame[k].clear( );
        other_eye_PositionY_frame[k].clear( );
        for( int i=1 ; i<=frames ; i++ )
        {
            for( int j=0 ; j<other_eye_time[k].size( ) ; j++ )
            {
                if( int(mspf*i)<other_eye_time[k][j] )
                {
                    other_eye_PositionX_frame[k].push_back( -1 );
                    other_eye_PositionY_frame[k].push_back( -1 );
                    break;
                }
                else if( other_eye_time[k][j+1]>( int(mspf*i) ) && ( int(mspf*i) )>=other_eye_time[k][j] )
                {
                    other_eye_PositionX_frame[k].push_back(  other_eye_PositionX[k][j] );
                    other_eye_PositionY_frame[k].push_back(  other_eye_PositionY[k][j] );
                    break;
                }
                else if( j==other_eye_time[k].size( )-1 )
                {
                    other_eye_PositionX_frame[k].push_back( -1 );
                    other_eye_PositionY_frame[k].push_back( -1 );
                    break;
                }
            }
        }
    }
}

void outputAnalysis(int type )
{
	//---------------
	//Specific Person
	//---------------
    vector<Analysis_data> All_Analysis_data;
    vector<int> count;
	int object_time = 0;
    int fix_time = 0;
	
	if(type==1 || type ==2)
	{
		// initial a vector to store each Analysis_data to AOI
		for( int i=0 ; i<All_AOI.size( ) ; i++ )
		{
			count.push_back( 0 );
			Analysis_data tempAnalysis = Analysis_data( );
			All_Analysis_data.push_back( tempAnalysis );//All_Analysis_data取決於AOI數量
		}
		
		for(int i=0; i<All_AOI.size(); i++)
		{
			All_Analysis_data[i].fix_times=0;
		}
		
		//int object_time = 0;
		//int fix_time = 0;
		bool object_exist, object_watched;
		for( int t=1 ; t<=frames ; t++ )
		{
			object_exist = false;
			object_watched = false;
			int tempX = eye_PositionX_frame[t];
			int tempY = eye_PositionY_frame[t];

			//CvRect drawbox=cvRect( 0, 0, 0, 0 );
			int AOIx, AOIy, AOIwidth, AOIheight, framePos;
			int AOIx_l, AOIy_l, AOIwidth_l, AOIheight_l, framePos_l;
			for( int i=0 ; i<All_AOI.size( ) ; i++ )
			{   
				CvRect drawbox=cvRect( 0, 0, 0, 0 );
				for( int j=0 ; j<All_AOI[i].AOI_frame_size( ) ; j++ )//return AOI_frame_vector.size( );
				{
					AOIx=All_AOI[i].get_x( j );//return AOI_position[i].x;
					AOIy=All_AOI[i].get_y( j );//return AOI_position[i].y;
					AOIwidth=All_AOI[i].get_width( j );//return AOI_position[i].width;
					AOIheight=All_AOI[i].get_height( j );//return AOI_position[i].height;
					framePos=All_AOI[i].AOI_frame( j );// return AOI_frame_vector[i];
					if( j>=1 )
					{
						// _l means last
						AOIx_l=All_AOI[i].get_x( j-1 );
						AOIy_l=All_AOI[i].get_y( j-1 );
						AOIwidth_l=All_AOI[i].get_width( j-1 );
						AOIheight_l=All_AOI[i].get_height( j-1 );
						framePos_l=All_AOI[i].AOI_frame( j-1 );
					}
					
					//t: frame
					if( t<framePos && j==0 ) break;
					else if( t<framePos && j!=0 )
					{
						drawbox.x=interpolation( AOIx_l, AOIx, framePos_l, framePos, t );
						drawbox.y=interpolation( AOIy_l, AOIy, framePos_l, framePos, t );
						drawbox.width=interpolation( AOIwidth_l, AOIwidth, framePos_l, framePos, t );
						drawbox.height=interpolation( AOIheight_l, AOIheight, framePos_l, framePos, t );
						break;
					}
					else if( t==framePos )
					{
						drawbox.x=AOIx;
						drawbox.y=AOIy;
						drawbox.width=AOIwidth;
						drawbox.height=AOIheight;
						break;
					}
					else if( t>framePos && framePos!=-1 ) continue;
					else if( t>framePos && framePos==-1 )
					{
						drawbox.x=AOIx_l;
						drawbox.y=AOIy_l;
						drawbox.width=AOIwidth_l;
						drawbox.height=AOIheight_l;
						break;
					}
				}
				
				
				if( drawbox.width>0 && drawbox.height>0 )
				{
					if( tempX>=drawbox.x && tempX<=drawbox.x+drawbox.width && tempY>=drawbox.y && tempY<=drawbox.y+drawbox.height )//若視線在AOI矩形裡 
					{
						count[i]++;
						
						if( !object_exist )
						{
							object_time++;
							object_exist = true;
						}
						if( !object_watched )
						{
							fix_time++;
							object_watched = true;
						}
						
						/*wxString welcomeText = "(0)object";
						welcomeText << i;
						welcomeText << "++:";
						welcomeText << count[i];
						wxMessageBox(welcomeText);*/
					}
					else
					{
						if( count[i]!=0 )
						{
							if( int(mspf*count[i])>250 )
								All_Analysis_data[i].fixationTime.push_back( count[i] );
							
							//凝視次數
							if(int(mspf*count[i])>350)
								All_Analysis_data[i].fix_times++;
								
							count[i] = 0;
						}
						
						if( !object_exist )
						{
							object_time++;
							object_exist = true;
						}
					}
				}
				else //!drawbox.width>0 && drawbox.height>0
				{
					if( count[i]!=0 )
					{
						if( int(mspf*count[i])>250 )
							All_Analysis_data[i].fixationTime.push_back( count[i] );
						
						//凝視次數
						if(int(mspf*count[i])>350)
								All_Analysis_data[i].fix_times++;
						/*wxString welcomeText = "(2)object";
						welcomeText << i;
						welcomeText << ":";
						welcomeText << count[i];
						wxMessageBox(welcomeText);*/
							
						count[i] = 0;
					}
				}
			}
		}
		
		int numObject_watched = 0;
		for( int i=0 ; i<All_AOI.size( ) ; i++ )
		{
			if( All_Analysis_data[i].fixationTime.size( )>0 ) numObject_watched++;
		}
		
		string file_name;
		file_name += "SingleAnalysis_";
		file_name += video_name.mb_str( );
		file_name += "_";
		file_name += eyemovement_name.mb_str( );
		file_name += ".txt";
		//wxMessageBox(file_name);
		fstream file;
		file.open( file_name.c_str( ), ios::out | ios::trunc );
		
		file << "Video information : \n"
			 << "    length of time : " << mspf*frames/1000 << "s\n"
			 << "    amount of AOIs : " << All_AOI.size( ) << "\n"
			 << "    amount of AOIs were watched : "  << numObject_watched
			 << " ( " << (float)numObject_watched/All_AOI.size( )*100 << "% AOIs have be watched )\n"
			 << "    length of time AOIs exist : " <<  mspf*object_time/1000 << "s\n"
			 << "    length of fixation time : " << mspf*fix_time/1000 << "s\n"
			 << "    rate of the AOIs were watched : " << (float)fix_time/object_time*100
			 << "% ( " << mspf*fix_time/1000 << "/" << mspf*object_time/1000 << " )\n\n";
			 
		file << "Important object and fixation times as follow --------------------------\n\n";
		//寫入txt: 重要物件輸出
		for( int i=0 ; i<All_AOI.size( ) ; i++ )
		{
			if(All_Analysis_data[i].fix_times > 0)
			{
				if( All_AOI[i].AOI_name.size( )==0 )
					file << "object" << i+1 << " : " << All_Analysis_data[i].fix_times << " times\n";
				else
					file << All_AOI[i].AOI_name << " : " <<  All_Analysis_data[i].fix_times << " times\n";
			}
		}
		
		file << "\n\nThe detail infomation of each AOIs as follow---------------------------\n\n";
		//寫入txt: 個別物件的時間長度和觀看程度
		for( int i=0 ; i<All_AOI.size( ) ; i++ )
		{
			if( All_AOI[i].AOI_name.size( )==0 )
				file << "object" << i+1 << " : " << "\n";
			else
				file << All_AOI[i].AOI_name << " : " << "\n";
			
			file << "    length of time : " << mspf*All_AOI[i].output_AOItime( )/1000;
			if( All_AOI[i].AOI_frame( All_AOI[i].AOI_frame_size( )-1 )!=-1 )
				file << "s ( frame : from " <<  All_AOI[i].AOI_frame( 0 ) << " to " << All_AOI[i].AOI_frame( All_AOI[i].AOI_frame_size( )-1 ) << " )" << "\n";
			else
				file << "s ( frame : from " <<  All_AOI[i].AOI_frame( 0 ) << " to " << frames << " )" << "\n";
			
			file << "    each fixation time : ";
			int sum = 0;
			for( int j=0 ; j<All_Analysis_data[i].fixationTime.size( ) ; j++ )
			{
				sum += All_Analysis_data[i].fixationTime[j];
				file << mspf*All_Analysis_data[i].fixationTime[j]/1000 << "s";
				if( j!=All_Analysis_data[i].fixationTime.size( )-1 ) file << ", ";
				else file << "\n";
			}
			if( All_Analysis_data[i].fixationTime.size( )==0 ) file << "\n";
			
			file << "    sum of all fixation time : " << mspf*sum/1000 << "s\n";
			
			file << "    rate of the object were watched : " << (float)sum/All_AOI[i].output_AOItime( )*100
				 << "% ( " << mspf*sum/1000 << "/" << mspf*All_AOI[i].output_AOItime( )/1000 << " )\n";
				 
			file << "\n";
		}
		file.close( );
    }
	
	if(type==1)
	{
		//寫入csv檔
		String file_name;
		file_name += "SingleAnalysis_";
		file_name += video_name.mb_str( );
		file_name += "_";
		file_name += eyemovement_name.mb_str( );
		file_name += ".csv";
		fstream SpecificOne_csv_file;
		SpecificOne_csv_file.open( file_name.c_str( ), ios::out | ios::trunc );
		
		SpecificOne_csv_file << "," << "time rate of the video (object) be watched (%)" << "," << "Fixation times" <<"\n";
		//SpecificOne_csv_file << "all" << "," << (float)fix_time/object_time*100 << "," << (float)fix_time << "\n";
		SpecificOne_csv_file << "all" << "," << (float)fix_time/object_time*100 << "\n";
		for( int i=0 ; i<All_AOI.size( ) ; i++ )
		{
			if( All_AOI[i].AOI_name.size( )==0 )
				SpecificOne_csv_file << "object" << i+1 << ",";
			else
				SpecificOne_csv_file << All_AOI[i].AOI_name << ",";
			
			int sum = 0;
			for( int j=0 ; j<All_Analysis_data[i].fixationTime.size( ) ; j++ )
			{
				sum += All_Analysis_data[i].fixationTime[j];
			}
			//SpecificOne_csv_file << (float)sum/All_AOI[i].output_AOItime( )*100 << "," << (float)sum << "\n";
			SpecificOne_csv_file << (float)sum/All_AOI[i].output_AOItime( )*100 << "," << All_Analysis_data[i].fix_times << "\n";
			
			/*
			if( All_AOI[i].AOI_name.size( )==0 )
				file << "object" << i+1 << " : " << All_Analysis_data[i].fix_times << " times\n";
			else
				file << All_AOI[i].AOI_name << " : " <<  All_Analysis_data[i].fix_times << " times\n";
			*/
			
		}    
		SpecificOne_csv_file.close( );
	}
	
	
	//---------------
	//Group
	//---------------
	//other_eyePosition_num
	vector<Analysis_data> All_Group_Analysis_data[other_eyePosition_num];
    vector<int> Group_count[other_eyePosition_num];
	int Group_object_time[other_eyePosition_num];
    int Group_fix_time[other_eyePosition_num];
    bool G_object_exist, G_object_watched;
	
	float Aver_Group_object_time=0, Aver_Group_fix_time=0;
	float Average_Group_sum[All_AOI.size( )];//紀錄每個object的團體平均時間
	
	//float Aver_numObject_watched=0, Aver_Group_object_time=0, Aver_Group_fix_time=0;
	//float Average_Group_sum=0;
	if(type == 2 || type == 3)
	{
		// initial a vector to store each Analysis_data to AOI
		for(int m=0; m<other_eyePosition_num; m++)
		{
			for( int i=0 ; i<All_AOI.size( ) ; i++ )
			{
				Group_count[m].push_back( 0 );
				Analysis_data tempAnalysis = Analysis_data( );
				All_Group_Analysis_data[m].push_back( tempAnalysis );//All_Analysis_data取決於AOI數量
			}
			for(int i=0; i<All_AOI.size(); i++)
			{
				All_Group_Analysis_data[m][i].fix_times=0;
			}
		}
		
		/*
		int Group_object_time[other_eyePosition_num];
		int Group_fix_time[other_eyePosition_num];
		bool G_object_exist, G_object_watched;
		
		float Aver_numObject_watched=0, Aver_Group_object_time=0, Aver_Group_fix_time=0;
		*/
		
		for(int i=0; i<other_eyePosition_num; i++)
		{
			Group_object_time[i] = 0;
			Group_fix_time[i] = 0;
		}
		
		for(int m=0; m<other_eyePosition_num; m++)
		{
			for( int t=1 ; t<=frames ; t++ )
			{
				G_object_exist = false;
				G_object_watched = false;
				int tempX = other_eye_PositionX_frame[m][t];
				int tempY = other_eye_PositionY_frame[m][t];

				//CvRect drawbox=cvRect( 0, 0, 0, 0 );
				int AOIx, AOIy, AOIwidth, AOIheight, framePos;
				int AOIx_l, AOIy_l, AOIwidth_l, AOIheight_l, framePos_l;
				for( int i=0 ; i<All_AOI.size( ) ; i++ )
				{   
					CvRect drawbox=cvRect( 0, 0, 0, 0 );
					for( int j=0 ; j<All_AOI[i].AOI_frame_size( ) ; j++ )//return AOI_frame_vector.size( );
					{
						//XXX_1為XXX上一回的資訊，意義?
						AOIx=All_AOI[i].get_x( j );//return AOI_position[i].x;
						AOIy=All_AOI[i].get_y( j );//return AOI_position[i].y;
						AOIwidth=All_AOI[i].get_width( j );//return AOI_position[i].width;
						AOIheight=All_AOI[i].get_height( j );//return AOI_position[i].height;
						framePos=All_AOI[i].AOI_frame( j );// return AOI_frame_vector[i];
						if( j>=1 )
						{
							// _l means last
							AOIx_l=All_AOI[i].get_x( j-1 );
							AOIy_l=All_AOI[i].get_y( j-1 );
							AOIwidth_l=All_AOI[i].get_width( j-1 );
							AOIheight_l=All_AOI[i].get_height( j-1 );
							framePos_l=All_AOI[i].AOI_frame( j-1 );
						}
						
						//t: frame
						if( t<framePos && j==0 ) break;
						else if( t<framePos && j!=0 )
						{
							drawbox.x=interpolation( AOIx_l, AOIx, framePos_l, framePos, t );
							drawbox.y=interpolation( AOIy_l, AOIy, framePos_l, framePos, t );
							drawbox.width=interpolation( AOIwidth_l, AOIwidth, framePos_l, framePos, t );
							drawbox.height=interpolation( AOIheight_l, AOIheight, framePos_l, framePos, t );
							break;
						}
						else if( t==framePos )
						{
							drawbox.x=AOIx;
							drawbox.y=AOIy;
							drawbox.width=AOIwidth;
							drawbox.height=AOIheight;
							break;
						}
						else if( t>framePos && framePos!=-1 ) continue;
						else if( t>framePos && framePos==-1 )
						{
							drawbox.x=AOIx_l;
							drawbox.y=AOIy_l;
							drawbox.width=AOIwidth_l;
							drawbox.height=AOIheight_l;
							break;
						}
					}
					
					
					if( drawbox.width>0 && drawbox.height>0 )
					{
						if( tempX>=drawbox.x && tempX<=drawbox.x+drawbox.width && tempY>=drawbox.y && tempY<=drawbox.y+drawbox.height )//若視線在AOI矩形裡 
						{
							Group_count[m][i]++;
							
							if( !G_object_exist )
							{
								Group_object_time[m]++;
								G_object_exist = true;
							}
							if( !G_object_watched )
							{
								Group_fix_time[m]++;
								G_object_watched = true;
							}
							
							/*wxString welcomeText = "(0)object";
							welcomeText << i;
							welcomeText << "++:";
							welcomeText << count[i];
							wxMessageBox(welcomeText);*/
						}
						else
						{
							if( Group_count[m][i]!=0 )
							{
								if( int(mspf*Group_count[m][i])>250 )
									All_Group_Analysis_data[m][i].fixationTime.push_back( Group_count[m][i] );
								
								//凝視次數
								if(int(mspf*Group_count[m][i])>350)
									All_Group_Analysis_data[m][i].fix_times++;
								/*wxString welcomeText = "(1)object";
								welcomeText << i;
								welcomeText << ":";
								welcomeText << count[i];
								wxMessageBox(welcomeText);
								*/
								Group_count[m][i] = 0;
							}
							
							if( !G_object_exist )
							{
								Group_object_time[m]++;
								G_object_exist = true;
							}
						}
					}
					else //!drawbox.width>0 && drawbox.height>0
					{
						if( Group_count[m][i]!=0 )
						{
							if( int(mspf*Group_count[m][i])>250 )
								All_Group_Analysis_data[m][i].fixationTime.push_back( Group_count[m][i] );
							
							//凝視次數
							if(int(mspf*Group_count[m][i])>350)
								All_Group_Analysis_data[m][i].fix_times++;
							
							/*wxString welcomeText = "(2)object";
							welcomeText << i;
							welcomeText << ":";
							welcomeText << count[i];
							wxMessageBox(welcomeText);*/
								
							Group_count[m][i] = 0;
						}
					}
				}
			}
			
			////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////
			//針對每一份眼動資料個別寫出個人資訊和csv檔
			int numObject_watched = 0;
			for( int i=0 ; i<All_AOI.size( ) ; i++ )
			{
				if( All_Group_Analysis_data[m][i].fixationTime.size( )>0 ) numObject_watched++;
			}
			
			char num_str[4];
            sprintf(num_str, "%d", m+1);
					
			string num_string(num_str);
			string file_name;
			file_name += "OneInGroupAnalysis_";
			file_name += video_name.mb_str( );
			file_name += "_";
			file_name += group_eyemovement_name[m].mb_str( );
			file_name += ".txt";
			//wxMessageBox(file_name);
			fstream file;
			file.open( file_name.c_str( ), ios::out | ios::trunc );
			
			
			wxString Path = other_eyePosition[m];
			file << "Data Path: " << Path.mb_str( ) << endl;			
			file << "Video information : \n"
				 << "    length of time : " << mspf*frames/1000 << "s\n"
				 << "    amount of AOIs : " << All_AOI.size( ) << "\n"
				 << "    amount of AOIs were watched : "  << numObject_watched
				 << " ( " << (float)numObject_watched/All_AOI.size( )*100 << "% AOIs have be watched )\n"
				 << "    length of time AOIs exist : " <<  mspf*Group_object_time[m]/1000 << "s\n"
				 << "    length of fixation time : " << mspf*Group_fix_time[m]/1000 << "s\n"
				 << "    rate of the AOIs were watched : " << (float)Group_fix_time[m]/Group_object_time[m]*100
				 << "% ( " << mspf*Group_fix_time[m]/1000 << "/" << mspf*Group_object_time[m]/1000 << " )\n\n";
			
			
			file << "Important object and fixation times as follow --------------------------\n\n";
			//寫入txt: 重要物件輸出
			for( int i=0 ; i<All_AOI.size( ) ; i++ )
			{
				if(All_Group_Analysis_data[m][i].fix_times > 0)
				{
					if( All_AOI[i].AOI_name.size( )==0 )
						file << "object" << i+1 << " : " << All_Group_Analysis_data[m][i].fix_times << " times\n";
					else
						file << All_AOI[i].AOI_name << " : " <<  All_Group_Analysis_data[m][i].fix_times << " times\n";
				}
			}

			
			file << "\n\nThe detail infomation of each AOIs as follow---------------------------\n\n";
			//寫入txt: 個別物件的時間長度和觀看程度
			for( int i=0 ; i<All_AOI.size( ) ; i++ )
			{
				if( All_AOI[i].AOI_name.size( )==0 )
					file << "object" << i+1 << " : " << "\n";
				else
					file << All_AOI[i].AOI_name << " : " << "\n";
				
				file << "    length of time : " << mspf*All_AOI[i].output_AOItime( )/1000;
				if( All_AOI[i].AOI_frame( All_AOI[i].AOI_frame_size( )-1 )!=-1 )
					file << "s ( frame : from " <<  All_AOI[i].AOI_frame( 0 ) << " to " << All_AOI[i].AOI_frame( All_AOI[i].AOI_frame_size( )-1 ) << " )" << "\n";
				else
					file << "s ( frame : from " <<  All_AOI[i].AOI_frame( 0 ) << " to " << frames << " )" << "\n";
				
				file << "    each fixation time : ";
				int sum = 0;
				for( int j=0 ; j<All_Group_Analysis_data[m][i].fixationTime.size( ) ; j++ )
				{
					sum += All_Group_Analysis_data[m][i].fixationTime[j];
					file << mspf*All_Group_Analysis_data[m][i].fixationTime[j]/1000 << "s";
					if( j!=All_Group_Analysis_data[m][i].fixationTime.size( )-1 ) file << ", ";
					else file << "\n";
				}
				if( All_Group_Analysis_data[m][i].fixationTime.size( )==0 ) file << "\n";
				
				file << "    sum of all fixation time : " << mspf*sum/1000 << "s\n";
				
				file << "    rate of the object were watched : " << (float)sum/All_AOI[i].output_AOItime( )*100
					 << "% ( " << mspf*sum/1000 << "/" << mspf*All_AOI[i].output_AOItime( )/1000 << " )\n";
					 
				file << "\n";
			}
			file.close( );
			
			
			//寫入csv檔
			String csv_file_name;
			csv_file_name = "OneInGroupAnalysis_";
			csv_file_name += video_name.mb_str( );
			csv_file_name += "_";
			csv_file_name += group_eyemovement_name[m].mb_str( );
			csv_file_name += ".csv";
			fstream SpecificOne_csv_file;
			SpecificOne_csv_file.open( csv_file_name.c_str( ), ios::out | ios::trunc );
			
			SpecificOne_csv_file << "," << "time rate of the video (object) be watched (%)" << "," << "Fixation times" <<"\n";
			//SpecificOne_csv_file << "all" << "," << (float)Group_fix_time[m]/Group_object_time[m]*100 << "," << (float)Group_fix_time[m] << "\n";
			SpecificOne_csv_file << "all" << "," << (float)Group_fix_time[m]/Group_object_time[m]*100 << "\n";
			for( int i=0 ; i<All_AOI.size( ) ; i++ )
			{
				if( All_AOI[i].AOI_name.size( )==0 )
					SpecificOne_csv_file << "object" << i+1 << ",";
				else
					SpecificOne_csv_file << All_AOI[i].AOI_name << ",";
				
				int sum = 0;
				for( int j=0 ; j<All_Group_Analysis_data[m][i].fixationTime.size( ) ; j++ )
				{
					sum += All_Group_Analysis_data[m][i].fixationTime[j];
				}
				//SpecificOne_csv_file << (float)sum/All_AOI[i].output_AOItime( )*100 << "," << (float)sum << "\n";
				SpecificOne_csv_file << (float)sum/All_AOI[i].output_AOItime( )*100 << "," << All_Group_Analysis_data[m][i].fix_times << "\n";
			}    
			SpecificOne_csv_file.close( );
			
			//////////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////////////////
		}
		
		
		//fstream test_file;
		//test_file.open( "test_write.txt", ios::out | ios::trunc );
		//other_eyePosition_num
		int Group_numObject_watched[other_eyePosition_num];
		for(int i=0; i<other_eyePosition_num; i++)
		{
			Group_numObject_watched[i]=0;
		}
		
		for(int m=0; m<other_eyePosition_num; m++)
		{
			for( int i=0 ; i<All_AOI.size( ) ; i++ )
			{
				if( All_Group_Analysis_data[m][i].fixationTime.size( )>0 ) 
					Group_numObject_watched[m]++;
			}
			//test_file << "Group_numObject_watched " << m << " : " << Group_numObject_watched[m] << "\n";
		}
			
		string G_file_name;
		G_file_name += "GroupAnalysis_";
		G_file_name += video_name.mb_str( );
		G_file_name += ".txt";
		//wxMessageBox(file_name);
		fstream G_file;
		G_file.open( G_file_name.c_str( ), ios::out | ios::trunc );
		
		int total_numObject_watched=0,  total_Group_object_time=0, total_Group_fix_time=0;
		float Aver_numObject_watched=0; //Aver_Group_object_time=0, Aver_Group_fix_time=0;
		
		for(int m=0; m<other_eyePosition_num; m++)
		{
			total_numObject_watched += Group_numObject_watched[m];
			total_Group_object_time += Group_object_time[m];
			total_Group_fix_time += Group_fix_time[m];
		}
		Aver_numObject_watched = (float)total_numObject_watched/(float)other_eyePosition_num;
		Aver_Group_object_time = (float)total_Group_object_time/other_eyePosition_num;
		Aver_Group_fix_time = (float)total_Group_fix_time/other_eyePosition_num;
		
		G_file << "Number of Group members: " <<  other_eyePosition_num << "\n"
			 << "Video information : \n"
			 << "    length of time : " << mspf*frames/1000 << "s\n"
			 << "    amount of AOIs : " << All_AOI.size( ) << "\n"
			 << "    Average amount of AOIs were watched : "  << (float)Aver_numObject_watched
			 << " ( " << (float)Aver_numObject_watched/All_AOI.size( )*100 << "% AOIs have be watched )\n"
			 << "    length of time AOIs exist : " <<  mspf*Aver_Group_object_time/1000 << "s\n"
			 << "    Average length of fixation time : " << mspf*Aver_Group_fix_time/1000 << "s\n"
			 << "    Average rate of the AOIs were watched : " << (float)Aver_Group_fix_time/Aver_Group_object_time*100
			 << "% ( " << mspf*Aver_Group_fix_time/1000 << "/" << mspf*Aver_Group_object_time/1000 << " )\n\n";
		
		
		
		G_file << "Each object, number of subjects(who fixate at) and average fixation times(who fixate at) as follow -\n\n";
		//寫入txt: 重要物件輸出
		for( int i=0 ; i<All_AOI.size( ) ; i++ )
		{
			float who_fix_count=0;
			float total_who_fix_times=0;
			//float total_fix_times=0;
			for(int m=0; m<other_eyePosition_num; m++)
			{
				if(All_Group_Analysis_data[m][i].fix_times > 0)
				{	
					who_fix_count++;
					total_who_fix_times += All_Group_Analysis_data[m][i].fix_times;
				}
				//total_fix_times += All_Group_Analysis_data[m][i].fix_times;
			}
			
			//float count_ratio = who_fix_count/other_eyePosition_num;
			float average_who_fix_times = (float)total_who_fix_times/who_fix_count;
			
			
			if( All_AOI[i].AOI_name.size( )==0 )
				G_file << "object" << i+1 << " - fixated by " << who_fix_count << " subjects with average " << average_who_fix_times << " times\n";
			else
				G_file << All_AOI[i].AOI_name << " - fixated by " << who_fix_count << " subjects with average " << average_who_fix_times << " times\n";
			
		}

				
		G_file << "\n\nThe detail infomation of each AOIs as follow---------------------------\n\n";	
		//寫入txt: 個別物件的時間長度和觀看程度
		for( int i=0 ; i<All_AOI.size( ) ; i++ )
		{
			if( All_AOI[i].AOI_name.size( )==0 )
				G_file << "object" << i+1 << " : " << "\n";
			else
				G_file << All_AOI[i].AOI_name << " : " << "\n";
			
			G_file << "    length of time : " << mspf*All_AOI[i].output_AOItime( )/1000;
			if( All_AOI[i].AOI_frame( All_AOI[i].AOI_frame_size( )-1 )!=-1 )
				G_file << "s ( frame : from " <<  All_AOI[i].AOI_frame( 0 ) << " to " << All_AOI[i].AOI_frame( All_AOI[i].AOI_frame_size( )-1 ) << " )" << "\n";
			else
				G_file << "s ( frame : from " <<  All_AOI[i].AOI_frame( 0 ) << " to " << frames << " )" << "\n";
			
			G_file << "    each member fixation time : \n";
			int Group_sum[other_eyePosition_num];
			for(int m=0; m<other_eyePosition_num; m++)
			{
				Group_sum[m]=0;
			}
			
			for(int m=0; m<other_eyePosition_num; m++)
			{
				G_file << "        member " << m+1 << " : ";
				for( int j=0 ; j<All_Group_Analysis_data[m][i].fixationTime.size( ) ; j++ )
				{
					Group_sum[m] += All_Group_Analysis_data[m][i].fixationTime[j];
					G_file << mspf*All_Group_Analysis_data[m][i].fixationTime[j]/1000 << "s";
					
					if( j!=All_Group_Analysis_data[m][i].fixationTime.size( )-1 ) 
						G_file << ", ";
					else 
						G_file << "\n";
				}
				if( All_Group_Analysis_data[m][i].fixationTime.size( )==0 ) G_file << "\n";//???????????????????
			}
			
			int total_Group_sum=0;
			//float Average_Group_sum[All_AOI.size( )];
				
			Average_Group_sum[i] = 0;			
			for(int m=0; m<other_eyePosition_num; m++)
			{
				total_Group_sum += Group_sum[m];
			}
			Average_Group_sum[i] = (float)total_Group_sum/other_eyePosition_num;
			
			G_file << "    Average sum of all fixation time : " << mspf*Average_Group_sum[i]/1000 << "s\n";
			
			G_file << "    Average rate of the object were watched : " << (float)Average_Group_sum[i]/All_AOI[i].output_AOItime( )*100
				 << "% ( " << mspf*Average_Group_sum[i]/1000 << "/" << mspf*All_AOI[i].output_AOItime( )/1000 << " )\n";
				 
			G_file << "\n";
		}
		G_file.close( );
    }
    
	if(type == 3)
	{
		//寫入csv檔
		String file_name;
		file_name = "GroupAnalysis_";
		file_name += video_name.mb_str( );
		file_name += ".csv";
		fstream Group_csv_file;
		Group_csv_file.open( file_name.c_str( ), ios::out | ios::trunc );
		
		Group_csv_file << "," << "Average time rate of the video (object) be watched (%)" << "," << "Average Fixation times" <<"\n";
		//Group_csv_file << "all" << "," << (float)Aver_Group_fix_time/Aver_Group_object_time*100 << "," << Aver_Group_fix_time << "\n";
		Group_csv_file << "all" << "," << (float)Aver_Group_fix_time/Aver_Group_object_time*100 << "\n";
		for( int i=0 ; i<All_AOI.size( ) ; i++ )
		{
			if( All_AOI[i].AOI_name.size( )==0 )
				Group_csv_file << "object" << i+1 << ",";
			else
				Group_csv_file << All_AOI[i].AOI_name << ",";
				
			//算 fixate times	
			float who_fix_count=0;
			float total_who_fix_times=0;
			//float total_fix_times=0;
			for(int m=0; m<other_eyePosition_num; m++)
			{				
				total_who_fix_times += All_Group_Analysis_data[m][i].fix_times;			
			}		
			//float count_ratio = who_fix_count/other_eyePosition_num;
			float average_who_fix_times = (float)total_who_fix_times/other_eyePosition_num;				
			
			
			//Group_csv_file << (float)Average_Group_sum[i]/All_AOI[i].output_AOItime( )*100 << "," << (float)Average_Group_sum[i] << "\n";
			Group_csv_file << (float)Average_Group_sum[i]/All_AOI[i].output_AOItime( )*100 << "," << average_who_fix_times << "\n";
		}    
		Group_csv_file.close( );
	}
	else if(type == 2)
	{
		String file_name;
		file_name = "SpecificOne&GroupAnalysis_";
		file_name += video_name.mb_str( );
		file_name += "_SPone-";
		file_name += eyemovement_name.mb_str();
		file_name += ".csv";
		fstream Both_csv_file;
		Both_csv_file.open( file_name.c_str( ), ios::out | ios::trunc );
		
		Both_csv_file << ",time rate of the video (object) be watched (%)" << ",,," << " Fixation times" << "\n";
		Both_csv_file << ",One,Group,Difference,One,Group,Difference" <<"\n";
		Both_csv_file << "all," << (float)fix_time/object_time*100<< "," << (float)Aver_Group_fix_time/Aver_Group_object_time*100 << "," <<  ((float)fix_time/object_time*100) - ((float)Aver_Group_fix_time/Aver_Group_object_time*100) << "\n";
								//<< (float)fix_time << "," << Aver_Group_fix_time << "," << ((float)fix_time) -  (Aver_Group_fix_time)<< "\n";
		
		for( int i=0 ; i<All_AOI.size( ) ; i++ )
		{
			if( All_AOI[i].AOI_name.size( )==0 )
				Both_csv_file << "object" << i+1 << ",";
			else
				Both_csv_file << All_AOI[i].AOI_name << ",";
				
			//算 time rate
			float sum = 0;
			for( int j=0 ; j<All_Analysis_data[i].fixationTime.size( ) ; j++ )
			{
				sum += All_Analysis_data[i].fixationTime[j];
			}
			float diff = ((float)sum/All_AOI[i].output_AOItime( )*100) - ((float)Average_Group_sum[i]/All_AOI[i].output_AOItime( )*100);
			if(diff<0.0001)
				diff=0;				
			
			//算 fixate times	
			float who_fix_count=0;
			float total_who_fix_times=0;
			//float total_fix_times=0;
			for(int m=0; m<other_eyePosition_num; m++)
			{				
				total_who_fix_times += All_Group_Analysis_data[m][i].fix_times;			
			}		
			//float count_ratio = who_fix_count/other_eyePosition_num;
			float average_who_fix_times = (float)total_who_fix_times/other_eyePosition_num;
			
			Both_csv_file << (float)sum/All_AOI[i].output_AOItime( )*100 << "," << (float)Average_Group_sum[i]/All_AOI[i].output_AOItime( )*100 << "," << diff << ","
								<< All_Analysis_data[i].fix_times << "," << (float)average_who_fix_times << "," << (float)All_Analysis_data[i].fix_times - average_who_fix_times << "\n";
		}
		Both_csv_file.close();			
	}		
}
