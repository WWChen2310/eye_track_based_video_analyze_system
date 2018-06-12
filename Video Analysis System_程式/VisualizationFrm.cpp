///-----------------------------------------------------------------
///
/// @file      VisualizationFrm.cpp
/// @author    Michael
/// Created:   2016/7/4 下午 10:02:57
/// @section   DESCRIPTION
///            Visualization class implementation
///
///------------------------------------------------------------------

#include "VisualizationFrm.h"
#include "Global.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <cv.h>
#include <highgui.h>
#include <fstream>
#include <vector>


//Global Var
wxString defined2_dynamicAOI;

//Do not add custom headers between
//Header Include Start and Header Include End
//wxDev-C++ designer will remove them
////Header Include Start
////Header Include End

//----------------------------------------------------------------------------
// Visualization
//----------------------------------------------------------------------------
//Add Custom Events only in the appropriate block.
//Code added in other places will be removed by wxDev-C++
////Event Table Start
BEGIN_EVENT_TABLE(Visualization,wxFrame)
	////Manual Code Start
	////Manual Code End
	
	EVT_BUTTON(ID_WXBUTTON3,Visualization::WxButton3Click)
	EVT_BUTTON(ID_WXBUTTON2,Visualization::WxButton2Click)
	EVT_BUTTON(ID_WXBUTTON1,Visualization::WxButton1Click)
	
	EVT_CLOSE(Visualization::OnClose)
END_EVENT_TABLE()
////Event Table End

Visualization::Visualization(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &position, const wxSize& size, long style)
: wxFrame(parent, id, title, position, size, style)
{
	CreateGUIControls();
}

Visualization::~Visualization()
{
}

void Visualization::CreateGUIControls()
{
	//Do not add custom code between
	//GUI Items Creation Start and GUI Items Creation End
	//wxDev-C++ designer will remove them.
	//Add the custom code before or after the blocks
	////GUI Items Creation Start

	WxPanel1 = new wxPanel(this, ID_WXPANEL1, wxPoint(-3, 0), wxSize(366, 195));

	WxButton1 = new wxButton(WxPanel1, ID_WXBUTTON1, _("Singular Spot"), wxPoint(23, 67), wxSize(90, 64), 0, wxDefaultValidator, _("WxButton1"));

	WxButton2 = new wxButton(WxPanel1, ID_WXBUTTON2, _("Plural Spot"), wxPoint(144, 67), wxSize(88, 63), 0, wxDefaultValidator, _("WxButton2"));

	WxButton3 = new wxButton(WxPanel1, ID_WXBUTTON3, _("Heat Map"), wxPoint(260, 68), wxSize(90, 62), 0, wxDefaultValidator, _("WxButton3"));

	WxOpenFileDialog5 =  new wxFileDialog(this, _("Choose other \"eye movement data\" files"), _(""), _(""), _("Text files (*.txt)|*.txt"), wxFD_OPEN | wxFD_MULTIPLE);

	WxOpenFileDialog4 =  new wxFileDialog(this, _("Choose a specific \"eye movement data\" file"), _(""), _(""), _("Text files (*.txt)|*.txt"), wxFD_OPEN);

	WxOpenFileDialog2 =  new wxFileDialog(this, _("Choose a \"eye movement data\" file"), _(""), _(""), _("Text files (*.txt)|*.txt"), wxFD_OPEN);

	WxOpenFileDialog1 =  new wxFileDialog(this, _("Choose a \"dynamic AOI of video\" file"), _(""), _(""), _("Text files (*.txt)|*.txt"), wxFD_OPEN);

	WxOpenFileDialog6 =  new wxFileDialog(this, _("Choose plural \"eye movement data\" files"), _(""), _(""), _("Text files (*.txt)|*.txt"), wxFD_OPEN | wxFD_MULTIPLE);

	WxOpenFileDialog3 =  new wxFileDialog(this, _("Choose a \"dynamic AOI of video\" file"), _(""), _(""), _("Text files (*.txt)|*.txt"), wxFD_OPEN);

	WxOpenFileDialog7 =  new wxFileDialog(this, _("Choose a \"dynamic AOI of video\" file"), _(""), _(""), _("Text files (*.txt)|*.txt"), wxFD_OPEN);
	
	WxOpenFileDialog8 =  new wxFileDialog(this, _("Choose other \"dynamic  AOI of video\" files"), _(""), _(""), _("Text files (*.txt)|*.txt"), wxFD_OPEN | wxFD_MULTIPLE);

	SetTitle(_("Choose of visualization type"));
	SetIcon(wxNullIcon);
	SetSize(8,8,379,230);
	Center();
	
	////GUI Items Creation End
}

void Visualization::OnClose(wxCloseEvent& event)
{
	Destroy();
}

/*
 * WxButton1Click, Singular Spot
 */
void Visualization::WxButton1Click(wxCommandEvent& event)
{	     
    // insert your code here
	initialVideo( );
	draw_eyePosition = true;
	videoanalyzer_mode = true;
    
    WxOpenFileDialog1->SetPath( dynamicAOI );//AOI
	if( WxOpenFileDialog1->ShowModal( ) )
    {
        dynamicAOI = WxOpenFileDialog1->GetPath( );
    }
    //defined2_dynamicAOI = dynamicAOI;
    WxOpenFileDialog2->SetPath( eyePosition );//特定一份眼動資料 
    if( WxOpenFileDialog2->ShowModal( ) )
    {
        eyePosition = WxOpenFileDialog2->GetPath( );
		eyemovement_name = WxOpenFileDialog2->GetFilename( );
        if( eyePosition=="" )
            draw_eyePosition = false;
    }
    
    //播放畫面相關設定 
	int const Ini_speed = 32;
    int speed=Ini_speed;
    const char* name="Video Analyzer";
    cvNamedWindow( name, CV_WINDOW_AUTOSIZE );
    capture = cvCreateFileCapture( video_path );//....................
    frames = cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_COUNT );//....................
    fps = cvGetCaptureProperty( capture, CV_CAP_PROP_FPS );
    float temp_mSec = 1000;
    mspf = temp_mSec/fps;
    
    
    //把.txt(AOI, EyeMovement)的資料讀初並存進相關變數 
    //wxMessageBox(defined2_dynamicAOI);
    inputAOI( dynamicAOI );
    inputEyePosition( eyePosition );
    
    if( frames!=0 )
    {
        cvCreateTrackbar( "postion", name, &video_position, frames, on_track_bar_slide );
    }

    bool nofile = false;
    if ( !capture )
    {
        printf( "!!! Failed cvCaptureFromVideo\n" );
        nofile = true;
    }
    
    
    //開始對影片和資料(AOI, Eyemovement)分析並將AOI和EyePosition畫至原始frame上 
    IplImage* image;
    bool videoEnd = false;
    bool Pause = false;
    int count_speed = 0;
    while( (!nofile)&&(!videoEnd) )
    {
        // "pause" at the end of movie
        while( video_position==frames-8 )
        {
            type=cvWaitKey( speed );
            if( type==' ' )
            {
                video_position=0;
                break;
            }
            if( type==27 )
            {
                videoEnd = true;
                outputAnalysis(1);
                break;
            }
        }
        if( videoEnd )
            break;
			
		//顯示進度條和當前畫面
        if( !Pause )
            cvSetTrackbarPos( "postion", name, ++video_position );
        else
            cvSetTrackbarPos( "postion", name, video_position );		
        image=cvQueryFrame( capture );//....................
        temp=cvCloneImage( image );
        origin_image=cvCloneImage( image );
        draw_box( image, video_position, RED_REC );//video_position start from 1
        cvShowImage( name, image );
		//

        type=cvWaitKey( speed );
        //some operations by keyboard when the video not pause
        // press "Esc" and exit
        if( type==27 )
        {
            break;
        }
        // press "space" and pause
        else if( type==' ' )
            Pause = true;
        else if( type==','&&count_speed<5 )
        {
            count_speed++;
            speed = speed*2;
        }
        else if( type=='.'&&count_speed>-5 )
        {
            count_speed--;
            if( speed/2>=1 )
                speed = speed/2;
        }
        else if( type=='/'||count_speed==0 )
        {
            count_speed=0;
            speed = Ini_speed;
        }
        
        if( Pause )
        {
            while( 1 )
            {
                // some operations by keyborad when the video pause
                type=cvWaitKey( speed );
                if( type==' ' )//暫停 -> 開始
                {
                    Pause = false;
                    break;
                }
                else if( type=='v' )//倒帶
                {
                    if( video_position>0 )
                        video_position--;
                    break;
                }
                else if( type=='n' && video_position!=frames-2 )//前進
                {
                    if( video_position+1>frames )
                        video_position=frames-2;
                    else
                        video_position++;
                    break;
                }
                else if( type=='f' )//倒帶
                {
                    if( video_position<10 )
                        video_position=0;
                    else
                        video_position-=10;
                    break;
                }
                else if( type=='h'&&video_position!=frames-2 )//前進
                {
                    if( video_position+10>frames )
                        video_position=frames-2;
                    else
                        video_position+=10;
                    break;
                }
                else if( type=='r' )//倒帶
                {
                    if( video_position<100 )
                        video_position=0;
                    else
                        video_position-=100;
                    break;
                }
                else if( type=='y'&&video_position!=frames-2 )//前進
                {
                    if( video_position+100>frames )
                        video_position=frames-2;
                    else
                        video_position+=100;
                    break;
                }
            }
        }
        cvReleaseImage( &temp );
        cvReleaseImage( &origin_image );
    }//while
	
	videoanalyzer_mode = false;
    cvReleaseCapture( &capture );
    cvDestroyWindow( name );  
	
	return;
}

/*
 * WxButton1Click, Plural Spot
 */
void Visualization::WxButton2Click(wxCommandEvent& event)
{
	// insert your code here
	initialVideo( );
	draw_eyePosition = true;
	videoanalyzer_mode = true;
    
    WxOpenFileDialog3->SetPath( dynamicAOI );//AOI 
	if( WxOpenFileDialog3->ShowModal( ) )
    {
        dynamicAOI = WxOpenFileDialog3->GetPath( );
    }
	
	
	//用於碩論實驗
	if( WxOpenFileDialog8->ShowModal( ) )//多份AOI
    {
        WxOpenFileDialog8->GetPaths(other_AOI_path);
		WxOpenFileDialog8->GetFilenames(other_AOI_name);
        
        other_AOI_path_num = other_AOI_path.GetCount();
    }
	////
	
    WxOpenFileDialog4->SetPath( eyePosition );//特定一份眼動資料 
    if( WxOpenFileDialog4->ShowModal( ) )
    {
        eyePosition = WxOpenFileDialog4->GetPath( );
		eyemovement_name = WxOpenFileDialog4->GetFilename( );
        if( eyePosition=="" )
            draw_eyePosition = false;
    }
    //WxOpenFileDialog5->SetPath( other_eyePosition );//其他多份眼動資料 
    if( WxOpenFileDialog5->ShowModal( ) )
    {
        WxOpenFileDialog5->GetPaths(other_eyePosition);
		WxOpenFileDialog5->GetFilenames(group_eyemovement_name);
        //wxMessageBox(other_eyePosition[0]);
        //wxMessageBox(other_eyePosition[2]);
        
        other_eyePosition_num = other_eyePosition.GetCount();
    }
    
    //播放畫面相關讀取和設定 
	int const Ini_speed = 32;
    int speed=Ini_speed;
    const char* name="Video Analyzer";
    cvNamedWindow( name, CV_WINDOW_AUTOSIZE );
    capture = cvCreateFileCapture( video_path );//....................
    frames = cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_COUNT );//....................
    fps = cvGetCaptureProperty( capture, CV_CAP_PROP_FPS );
    float temp_mSec = 1000;
    mspf = temp_mSec/fps;
    
	//碩論實驗: 比較不同間隔N與N=1(standard)的offset
	fp_standard_N.open(filename_standard_N, ios::out);
	for(int i=0; i<other_AOI_path_num; i++)
	{
		fp_standard_N << other_AOI_name[i] << ", offset_x, offset_y, distance,,";
	}
    ////
	
    //把.txt(AOI, EyeMovement)的資料讀初並存進相關變數 
    //wxMessageBox(defined2_dynamicAOI);
    inputAOI( dynamicAOI );
	input_other_AOI( other_AOI_path );////用於碩論實驗
    inputEyePosition( eyePosition );//載入特定一份eyePosition資料
    input_otherEyePosition( other_eyePosition );//載入多份eyePosition資料 
    
    if( frames!=0 )
    {
        cvCreateTrackbar( "postion", name, &video_position, frames, on_track_bar_slide );
    }

    bool nofile = false;
    if ( !capture )
    {
        printf( "!!! Failed cvCaptureFromVideo\n" );
        nofile = true;
    }
	
	//開始對影片和資料(AOI, Eyemovement)分析並將AOI和EyePosition畫至原始frame上 
    IplImage* image;
    bool videoEnd = false;
    bool Pause = false;
    int count_speed = 0;
    while( (!nofile)&&(!videoEnd) )
    {
        // "pause" at the end of movie
        while( video_position==frames-8 )
        {
            type=cvWaitKey( speed );
            if( type==' ' )
            {
                video_position=0;
                break;
            }
            if( type==27 )
            {
                videoEnd = true;
                outputAnalysis(2);
                break;
            }
        }
        if( videoEnd )
            break;
			
		//顯示進度條和當前畫面
        if( !Pause )
            cvSetTrackbarPos( "postion", name, ++video_position );
        else
            cvSetTrackbarPos( "postion", name, video_position );		
        image=cvQueryFrame( capture );//....................
        temp=cvCloneImage( image );//IplImage: temp
        origin_image=cvCloneImage( image );//IplImage: origin_image
        draw_box( image, video_position, RED_REC );//video_position start from 1
        cvShowImage( name, image );
		//

        type=cvWaitKey( speed );
        //some operations by keyboard when the video not pause
        // press "Esc" and exit
        if( type==27 )
        {
            break;
        }
        // press "space" and pause
        else if( type==' ' )
            Pause = true;
        else if( type==','&&count_speed<5 )
        {
            count_speed++;
            speed = speed*2;
        }
        else if( type=='.'&&count_speed>-5 )
        {
            count_speed--;
            if( speed/2>=1 )
                speed = speed/2;
        }
        else if( type=='/'||count_speed==0 )
        {
            count_speed=0;
            speed = Ini_speed;
        }
        
        if( Pause )
        {
            while( 1 )
            {
                // some operations by keyborad when the video pause
                type=cvWaitKey( speed );
                if( type==' ' )//暫停 -> 開始
                {
                    Pause = false;
                    break;
                }
                else if( type=='v' )//倒帶
                {
                    if( video_position>0 )
                        video_position--;
                    break;
                }
                else if( type=='n' && video_position!=frames-2 )//前進
                {
                    if( video_position+1>frames )
                        video_position=frames-2;
                    else
                        video_position++;
                    break;
                }
                else if( type=='f' )//倒帶
                {
                    if( video_position<10 )
                        video_position=0;
                    else
                        video_position-=10;
                    break;
                }
                else if( type=='h'&&video_position!=frames-2 )//前進
                {
                    if( video_position+10>frames )
                        video_position=frames-2;
                    else
                        video_position+=10;
                    break;
                }
                else if( type=='r' )//倒帶
                {
                    if( video_position<100 )
                        video_position=0;
                    else
                        video_position-=100;
                    break;
                }
                else if( type=='y'&&video_position!=frames-2 )//前進
                {
                    if( video_position+100>frames )
                        video_position=frames-2;
                    else
                        video_position+=100;
                    break;
                }
            }//while
        }//if
        cvReleaseImage( &temp );
        cvReleaseImage( &origin_image );
    }//while

    cvReleaseCapture( &capture );
    cvDestroyWindow( name );
	
	//reset
    other_eyePosition_num = 0;
	videoanalyzer_mode = false;	
	
	fp_standard_N.close();
}

/*
 * WxButton1Click, Heat Map
 */
void Visualization::WxButton3Click(wxCommandEvent& event)
{
    //wxArrayString other_eyePosition;//用於載入多個EyeMovement.txt路徑 
 	// insert your code here
	initialVideo( );
	//draw_eyePosition = true;
	WxOpenFileDialog7->SetPath( dynamicAOI );//AOI 
	if( WxOpenFileDialog7->ShowModal( ) )
    {
        dynamicAOI = WxOpenFileDialog7->GetPath( );
    }
    //WxOpenFileDialog6->SetPath( other_eyePosition );//其他多份眼動資料 
    if( WxOpenFileDialog6->ShowModal( ) )
    {
        WxOpenFileDialog6->GetPaths(other_eyePosition);
		WxOpenFileDialog6->GetFilenames(group_eyemovement_name);
        other_eyePosition_num = other_eyePosition.GetCount();
    }
    //wxMessageBox(eyePosition);
    
    //播放畫面相關讀取和設定 
	int const Ini_speed = 32;
    int speed=Ini_speed;
    const char* name="Video Analyzer";
    cvNamedWindow( name, CV_WINDOW_AUTOSIZE );
    capture = cvCreateFileCapture( video_path );//....................
    frames = cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_COUNT );//CV_CAP_PROP_FRAME_WIDTH =3,CV_CAP_PROP_FRAME_HEIGHT =4
    frame_width = cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH );
    frame_height = cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT );
    fps = cvGetCaptureProperty( capture, CV_CAP_PROP_FPS );
    float temp_mSec = 1000;
    mspf = temp_mSec/fps;
    
    //把.txt(AOI, EyeMovement)的資料讀初並存進相關變數
    inputAOI( dynamicAOI ); 
    input_otherEyePosition( other_eyePosition );//載入多份eyePosition資料 
    
    if( frames!=0 )
    {
        cvCreateTrackbar( "postion", name, &video_position, frames, on_track_bar_slide );
    }

    bool nofile = false;
    if ( !capture )
    {
        printf( "!!! Failed cvCaptureFromVideo\n" );
        nofile = true;
    }
    
    //開始對影片和資料(AOI, Eyemovement)分析並將AOI和EyePosition畫至原始frame上 
    IplImage* image;
    bool videoEnd = false;
    bool Pause = false;
    int count_speed = 0;
    while( (!nofile)&&(!videoEnd) )
    {
        // "pause" at the end of movie
        while( video_position==frames-8 )
        {
            type=cvWaitKey( speed );
            if( type==' ' )
            {
                video_position=0;
                break;
            }
            if( type==27 )
            {
                videoEnd = true;
                outputAnalysis(3);
                break;
            }
        }
        if( videoEnd )
            break;
			
		//顯示進度條和當前畫面
        if( !Pause )
            cvSetTrackbarPos( "postion", name, ++video_position );
        else
            cvSetTrackbarPos( "postion", name, video_position );		
        image=cvQueryFrame( capture );//....................
        temp=cvCloneImage( image );//IplImage: temp
        origin_image=cvCloneImage( image );//IplImage: origin_image
        //draw_box( image, video_position );//video_position start from 1
        draw_HeatMap(image, video_position);
        cvShowImage( name, image );
		//

        type=cvWaitKey( speed );
        //some operations by keyboard when the video not pause
        // press "Esc" and exit
        if( type==27 )
        {
            break;
        }
        // press "space" and pause
        else if( type==' ' )
            Pause = true;
        else if( type==','&&count_speed<5 )
        {
            count_speed++;
            speed = speed*2;
        }
        else if( type=='.'&&count_speed>-5 )
        {
            count_speed--;
            if( speed/2>=1 )
                speed = speed/2;
        }
        else if( type=='/'||count_speed==0 )
        {
            count_speed=0;
            speed = Ini_speed;
        }
        
        if( Pause )
        {
            while( 1 )
            {
                // some operations by keyborad when the video pause
                type=cvWaitKey( speed );
                if( type==' ' )//暫停 -> 開始
                {
                    Pause = false;
                    break;
                }
                else if( type=='v' )//倒帶
                {
                    if( video_position>0 )
                        video_position--;
                    break;
                }
                else if( type=='n' && video_position!=frames-2 )//前進
                {
                    if( video_position+1>frames )
                        video_position=frames-2;
                    else
                        video_position++;
                    break;
                }
                else if( type=='f' )//倒帶
                {
                    if( video_position<10 )
                        video_position=0;
                    else
                        video_position-=10;
                    break;
                }
                else if( type=='h'&&video_position!=frames-2 )//前進
                {
                    if( video_position+10>frames )
                        video_position=frames-2;
                    else
                        video_position+=10;
                    break;
                }
                else if( type=='r' )//倒帶
                {
                    if( video_position<100 )
                        video_position=0;
                    else
                        video_position-=100;
                    break;
                }
                else if( type=='y'&&video_position!=frames-2 )//前進
                {
                    if( video_position+100>frames )
                        video_position=frames-2;
                    else
                        video_position+=100;
                    break;
                }
            }//while
        }//if
        cvReleaseImage( &temp );
        cvReleaseImage( &origin_image );
    }//while

    cvReleaseCapture( &capture );
    cvDestroyWindow( name );

    other_eyePosition_num = 0; 
    
	
    //wxMessageBox("Heat Map Preparing ...");
       
}

