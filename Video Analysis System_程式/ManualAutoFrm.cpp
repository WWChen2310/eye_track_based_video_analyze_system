///-----------------------------------------------------------------
///
/// @file      ManualAutoFrm.cpp
/// @author    Michael
/// Created:   2016/7/5 下午 02:55:32
/// @section   DESCRIPTION
///            ManualAuto class implementation
///
///------------------------------------------------------------------

#include "ManualAutoFrm.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <cv.h>
#include "cxcore.h"
#include <highgui.h>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
//#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/core/utility.hpp"
#include <opencv2/tracking.hpp>
//#include "opencv2/nonfree/nonfree.hpp"
#include <fstream>
#include <vector>
#include "Global.h"


void AutoMode_Initail(int frames);
void AOI_Period_Setting(int pre_pos, Mat frame);
void Detect_Face(IplImage* img, int pos);

//opencv3.0
void Tracker_space_release(int AOIth, Mat frame);
void AOI_tracking_Initial(Mat frame, int AOIth, int AOIindex);
void AOI_object_tracking(Mat frame, int AOIth, int AOIindex, int pos, bool isAdjust);

/*
void Calculate_feature( IplImage* img, int pos);
void Find_Homography(int pos, int AOIth);
void Find_Similar_Area(int pos, int AOIth);
*/
int scene_change_detect(IplImage* pre_img, IplImage* img);

//global Var
wxString defined_dynamicAOI;//用來防止initialVideo()重置dynamicAOI造成inputAOI(dynamicAOI)失效 

int video_width;
int video_height;

// the minimum face object size
int min_face_height = 30;
int min_face_width = 30;

char filename[]="FindHomo_pt_test_write.txt";
fstream fp;

//Auto Related
vector<int> scene_change_value_frame;
vector<bool> isFaceDetect_frame;

//object tracking
Ptr<Tracker> tracker[100];		
int N=10;






//Do not add custom headers between
//Header Include Start and Header Include End
//wxDev-C++ designer will remove them
////Header Include Start
////Header Include End

//----------------------------------------------------------------------------
// ManualAuto
//----------------------------------------------------------------------------
//Add Custom Events only in the appropriate block.
//Code added in other places will be removed by wxDev-C++
////Event Table Start
BEGIN_EVENT_TABLE(ManualAuto,wxFrame)
	////Manual Code Start
	////Manual Code End
	//EVT_MENU(ID_MNU_INTERVAL_1005, ManualAuto::Mnuinterval1005Click)
	EVT_MENU(ID_MNU_SETTING_1006, ManualAuto::Mnusetting1006Click)
	EVT_BUTTON(ID_WXBUTTON3,ManualAuto::WxButton3Click)
	EVT_BUTTON(ID_WXBUTTON2,ManualAuto::WxButton2Click)
	EVT_BUTTON(ID_WXBUTTON1,ManualAuto::WxButton1Click)
	EVT_CLOSE(ManualAuto::OnClose)
END_EVENT_TABLE()
////Event Table End

ManualAuto::ManualAuto(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &position, const wxSize& size, long style)
: wxFrame(parent, id, title, position, size, style)
{
	CreateGUIControls();
}

ManualAuto::~ManualAuto()
{
}

void ManualAuto::CreateGUIControls()
{
	//Do not add custom code between
	//GUI Items Creation Start and GUI Items Creation End
	//wxDev-C++ designer will remove them.
	//Add the custom code before or after the blocks
	////GUI Items Creation Start

	WxPanel1 = new wxPanel(this, ID_WXPANEL1, wxPoint(0, -2), wxSize(355, 228));

	WxButton1 = new wxButton(WxPanel1, ID_WXBUTTON1, _("Manual Settimg"), wxPoint(43, 42), wxSize(112, 65), 0, wxDefaultValidator, _("WxButton1"));

	WxButton2 = new wxButton(WxPanel1, ID_WXBUTTON2, _("Auto Setting"), wxPoint(196, 42), wxSize(112, 65), 0, wxDefaultValidator, _("WxButton2"));

	WxButton3 = new wxButton(WxPanel1, ID_WXBUTTON3, _("Load Defined AOI File"), wxPoint(112, 171), wxSize(127, 34), 0, wxDefaultValidator, _("WxButton3"));

	WxOpenFileDialog1 =  new wxFileDialog(this, _("Choose a file"), _(""), _(""), _("*.*"), wxFD_OPEN);

	WxMenuBar2 = new wxMenuBar();
	wxMenu *ID_MNU_INTERVAL_1005_Mnu_Obj = new wxMenu();
	ID_MNU_INTERVAL_1005_Mnu_Obj->Append(ID_MNU_SETTING_1006, _("&Setting"), _(""), wxITEM_NORMAL);
	//ID_MNU_INTERVAL_1005_Mnu_Obj->AppendSeparator();
	WxMenuBar2->Append(ID_MNU_INTERVAL_1005_Mnu_Obj, _("&Interval"));
	SetMenuBar(WxMenuBar2);

	SetTitle(_("Choose of Manual or Auto AOI"));
	SetIcon(wxNullIcon);
	SetSize(8,8,371,263);
	Center();
	
	////GUI Items Creation End
}
/*
    WxPanel1 = new wxPanel(this, ID_WXPANEL1, wxPoint(0, -2), wxSize(355, 228));

	WxButton1 = new wxButton(WxPanel1, ID_WXBUTTON1, _("Manual Settimg"), wxPoint(43, 42), wxSize(112, 65), 0, wxDefaultValidator, _("WxButton1"));

	WxButton2 = new wxButton(WxPanel1, ID_WXBUTTON2, _("Auto Setting"), wxPoint(196, 42), wxSize(112, 65), 0, wxDefaultValidator, _("WxButton2"));

	WxButton3 = new wxButton(WxPanel1, ID_WXBUTTON3, _("Load Defined AOI File"), wxPoint(100, 171), wxSize(140, 34), 0, wxDefaultValidator, _("WxButton3"));

	WxOpenFileDialog1 =  new wxFileDialog(this, _("Choose a AOI file"), _(""), _(""), _("Text files (*.txt)|*.txt"), wxFD_OPEN);

	SetTitle(_("Choose of Manual or Auto AOI"));
	SetIcon(wxNullIcon);
	SetSize(8,8,371,263);
	Center();
*/

void ManualAuto::OnClose(wxCloseEvent& event)
{
	Destroy();
}


void ManualAuto::Mnusetting1006Click(wxCommandEvent& event)
{
    //wxMessageBox( "xxx" );
    wxTextEntryDialog *WxTextEntryDialog1;
	WxTextEntryDialog1 = new wxTextEntryDialog(NULL, _("Default N=10"), _("Please enter the interval N for tracking."), _(""));
	
	wxString tempname;
	if( WxTextEntryDialog1->ShowModal( ) )
	{
       tempname = WxTextEntryDialog1->GetValue( ); 
    }
	//WxTextEntryDialog1->SetValue( tempname );
	
	int num;
    num = wxAtoi(tempname);  
    N=num;
    
}



/*
 * WxButton1Click, Manual Settimg
 */
void ManualAuto::WxButton1Click(wxCommandEvent& event)
{
	// insert your code here	
	initialVideo( );
	//把.txt(AOI, EyeMovement)的資料讀初並存進相關變數 
	//wxMessageBox(defined_dynamicAOI);
	inputAOI( defined_dynamicAOI );//因 initialVideo( )和 inputAOI( defined_dynamicAOI )順序錯誤會造成系統異常，
                                   //且initialVideo( )會把dynamicAOI重置(在button3 load進來的路徑會失效，所以新增全域變數 defined_dynamicAOI) 

	//播放畫面相關設定 
	int const Ini_speed = 32;
    int speed=Ini_speed;
    const char* name="Dynamic AOI Module";
    cvNamedWindow( name, CV_WINDOW_AUTOSIZE );
    capture = cvCreateFileCapture( video_path );
    frames = cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_COUNT );//影格數
    fps = cvGetCaptureProperty( capture, CV_CAP_PROP_FPS );
    float temp_mSec = 1000;
    mspf = temp_mSec/fps;
        
    
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

    IplImage* image;
    bool videoEnd = false;
    bool Pause = false;
    int display_speed = 0;
	
	
	//碩論實驗:比較手動自動
	char filename_AOISetting[]="AOISetting_ManaulCount.txt";
	fstream fp_AOISetting;
	fp_AOISetting.open(filename_AOISetting, ios::out);
	int forward_rewind_cound = 0;
	int pause_count = 0;
	mouse_operation_count=0;
	
    while( (!nofile)&&(!videoEnd) )
    {
        // "pause" at the end of movie
        while( video_position==frames-1 )//影片最後
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
                wxMessageDialog *dial = new wxMessageDialog( NULL, 
                    wxT( "Do you want to save the Dynamic AOI data?" ), wxT( "Exit" ), 
                    wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION );
                if( dial->ShowModal( )==wxID_YES )
                    outputAOI();
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
        image=cvQueryFrame( capture );
        temp=cvCloneImage( image );
        origin_image=cvCloneImage( image );
        draw_box( image, video_position, RED_REC );		
        cvShowImage( name, image );

        type=cvWaitKey( speed );//在這中間有按按鈕，就會繼續下去
        //some operations by keyboard when the video not pause
        // press "Esc" and exit
        if( type==27 )
        {
            wxMessageDialog *dial = new wxMessageDialog( NULL, 
                wxT( "Do you want to save the Dynamic AOI data?" ), wxT( "Exit" ), 
                wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION );
            if( dial->ShowModal( )==wxID_YES )
                outputAOI();
            break;
        }
        // press "space" and pause
        else if( type==' ' )
		{
            Pause = true;
			pause_count++;
		}
        else if( type==','&&display_speed<5 )//播放速度*2
        {
            display_speed++;
            speed = speed*2;
        }
        else if( type=='.'&&display_speed>-5 )//播放速度/2
        {
            display_speed--;
            if( speed/2>=1 )
                speed = speed/2;
        }
        else if( type=='/'||display_speed==0 )//原始播放速度
        {
            display_speed=0;
            speed = Ini_speed;
        }
        
        if( Pause )//影片暫停時可進行的動作
        {
            cvSetMouseCallback( name, my_mouse_callback, (void*) image );
            while( 1 )
            {
                //cvCopyImage( image, temp );
				cvCopy( image, temp );


                if( drawAOI || moveAOI || adjustAOI )
                    draw_box( temp, video_position , RED_REC);

                cvShowImage( name, temp );

                // some operations by keyborad when the video pause
                type=cvWaitKey( speed );
                if( type==' ' )//暫停 -> 開始
                {
                    Pause = false;
					
					pause_count++;
                    break;
                }
                else if( type=='v' )//倒帶
                {
                    if( video_position>0 )
                        video_position--;
						
					forward_rewind_cound++;
                    break;
                }
                else if( type=='n' && video_position!=frames-2 )//前進
                {
                    if( video_position+1>frames )
                        video_position=frames-2;
                    else
                        video_position++;
						
					forward_rewind_cound++;
                    break;
                }
                else if( type=='f' )//倒帶
                {
                    if( video_position<10 )
                        video_position=0;
                    else
                        video_position-=10;
						
					forward_rewind_cound++;
                    break;
                }
                else if( type=='h'&&video_position!=frames-2 )//前進
                {
                    if( video_position+10>frames )
                        video_position=frames-2;
                    else
                        video_position+=10;
						
					forward_rewind_cound++;	
                    break;
                }
                else if( type=='r' )//倒帶
                {
                    if( video_position<100 )
                        video_position=0;
                    else
                        video_position-=100;
						
					forward_rewind_cound++;
                    break;
                }
                else if( type=='y'&&video_position!=frames-2 )//前進
                {
                    if( video_position+100>frames )
                        video_position=frames-2;
                    else
                        video_position+=100;
					
					forward_rewind_cound++;
                    break;
                }
				
				
            }//while(1)
			
            cvSetMouseCallback( name, NULL, NULL );
			
        }//if(pause)
        cvReleaseImage( &temp );
        cvReleaseImage( &origin_image );
		
    }//while( (!nofile)&&(!videoEnd) )

    cvReleaseCapture( &capture );
    cvDestroyWindow( name );
    
	fp_AOISetting << "forward_rewind: " << forward_rewind_cound << "  Pause: " << pause_count << "  Mouse_Operation_Count: " << mouse_operation_count << endl;
	fp_AOISetting.close();
	
}


/*
 * WxButton2Click, Auto Setting
 */
void ManualAuto::WxButton2Click(wxCommandEvent& event)
{
	// insert your code here
	//wxMessageBox("Auto Setting Preparing ...");	
	initialVideo( );
	//把.txt(AOI, EyeMovement)的資料讀初並存進相關變數 
	//wxMessageBox(defined_dynamicAOI);
	inputAOI( defined_dynamicAOI );//因 initialVideo( )和 inputAOI( defined_dynamicAOI )順序錯誤會造成系統異常，
                                   //且initialVideo( )會把dynamicAOI重置(在button3 load進來的路徑會失效，所以新增全域變數 defined_dynamicAOI) 

	//播放畫面相關設定 
	int const Ini_speed = 32;
    int speed=Ini_speed;
    const char* name="Dynamic AOI Module";
    cvNamedWindow( name, CV_WINDOW_AUTOSIZE );
    capture = cvCreateFileCapture( video_path );
	pre_capture = cvCreateFileCapture( video_path );
    frames = cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_COUNT );//影格數
    fps = cvGetCaptureProperty( capture, CV_CAP_PROP_FPS );
	video_width = cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH);
	video_height = cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT);
    float temp_mSec = 1000;
    mspf = temp_mSec/fps;
        
    
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

    IplImage* image;
    bool videoEnd = false;
    bool Pause = false;
    int display_speed = 0;
	int Last_change_frame = -1;
	bool isChange_View = false; 
	
	
	//test write...
	//fp.open(filename, ios::out);
		
	//Auto Initial ...
	isAuto_mode = true;
	AutoMode_Initail(frames);
	
	//碩論實驗
	char filename_AOISetting[]="AOISetting_AutoCount.txt";
	fstream fp_AOISetting;
	fp_AOISetting.open(filename_AOISetting, ios::out);
	int forward_rewind_cound = 0;
	int pause_count = 0;
	mouse_operation_count=0;
	
    while( (!nofile)&&(!videoEnd) )
    {
        // "pause" at the end of movie
		///////////////////////////////////////////////////////////////////////////////////////////////
        while( video_position==frames-4 )//影片最後
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
                wxMessageDialog *dial = new wxMessageDialog( NULL, 
                    wxT( "Do you want to save the Dynamic AOI data?" ), wxT( "Exit" ), 
                    wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION );
                if( dial->ShowModal( )==wxID_YES )
                    outputAOI();
                break;
            }
        }
        
        if( videoEnd )
            break;
        ////////////////////////////////////////////////////////////////////////////////////////////////
		
        //顯示進度條和當前畫面
		/*
        if( !Pause )
            cvSetTrackbarPos( "postion", name, ++video_position );
        else
            cvSetTrackbarPos( "postion", name, video_position );
		*/
		cvSetTrackbarPos( "postion", name, video_position );
		image=cvQueryFrame( capture );
		M_frame = cvarrToMat(image);//object tracking
		temp=cvCloneImage( image );
        origin_image=cvCloneImage( image );
        
		
		//前一張frame(要判斷是否連續畫面)
		if(video_position != 0)
		{
			pre_video_position = video_position-1;
			cvSetCaptureProperty( pre_capture, CV_CAP_PROP_POS_FRAMES, pre_video_position);
			pre_image=cvQueryFrame( pre_capture );
			
			//算相鄰兩畫面的scene_change_value
			if(scene_change_value_frame[video_position-1] == -1 )
			{
				int scene_change_value = scene_change_detect(pre_image, image);
				scene_change_value_frame[video_position-1] = scene_change_value;
			}
		}
		

		
		//偵測畫面變換(scene_change_value && 兩個高scene_change_value是否太近)
		if(scene_change_value_frame[video_position-1] > 50 && video_position >= N)
		{
			isChange_View = true;
			for(int i=1 ; i<N; i++)
			{
				if(scene_change_value_frame[video_position-1-i] > 50)
				{
					isChange_View = false;
					break;
				}
			}
		}
		else
		{
			isChange_View = false;
		}
		
		
		
		//開始自動化調整AOI位置流程
 		if(video_position == 0 || isChange_View/*scene_change_value_frame[video_position-1] > 50*/)//影片一開始 或 變換畫面後的第一張影格(連續影格的第一張frame)
		{
			Pause = true;
			
			//對 All_AOI 的 AOI_frame_vector 中最後的 -1 清除(上一時段的AOI在下一時段已不用tracking)
			if(video_position != 0)
			{
				AOI_Period_Setting(video_position-1, M_frame);
			}
			
			//人臉偵測並新增AOI
			if(!isFaceDetect_frame[video_position])
			{
				Detect_Face(image, video_position);
				isFaceDetect_frame[video_position] = true;
			}
			
			//AOI Tracking Initail
			for(int AOIth=0; AOIth<All_AOI.size(); AOIth++)
			{
				int last_frame_vector = All_AOI[AOIth].AOI_frame_size()-1;
				if(video_position> All_AOI[AOIth].AOI_frame(last_frame_vector) && All_AOI[AOIth].AOI_frame(last_frame_vector) != -1)//此frame已超過All_AOI[i]的範圍
				{
					continue;
				}
				else
				{
					if(!isIniAOItracker[AOIth])
					{
						AOI_tracking_Initial(M_frame, AOIth, 0);
					}
				}
			}
			
			
			//畫AOI
			draw_box( image, video_position, PURPLE_REC );
		}
		else//連續影格
		{		
			for(int AOIth=0; AOIth<All_AOI.size(); AOIth++)
			{
				if(All_AOI[AOIth].get_AOI_Type() == AUTO)//自動模式才做
				{
					bool isMatch = false;
					if( (video_position - All_AOI[AOIth].AOI_frame(0))%N == 0 && (video_position - All_AOI[AOIth].AOI_frame(0)) > 0 )//匹配影格
					{
						int last_frame_vector = All_AOI[AOIth].AOI_frame_size()-1;
						
						//此frame已超過All_AOI[AOIth]的範圍
						if( (video_position > All_AOI[AOIth].AOI_frame(last_frame_vector) && All_AOI[AOIth].AOI_frame(last_frame_vector) != -1)   || last_frame_vector==0 )
						{
							continue;
						}
						else//此frame還在All_AOI[AOIth]的範圍
						{
							//在此frame新加入的AOI
							/*
							if(video_position == All_AOI[AOIth].AOI_frame(0))
							{		
								draw_box( image, video_position, PURPLE_REC );
							}
							else
							{
							*/
								
								bool isAdjust = false;
								int AOIindex;
								if(All_AOI[AOIth].AOI_frame(last_frame_vector) == -1 && video_position > All_AOI[AOIth].AOI_frame(last_frame_vector-1))
								{
									AOIindex = last_frame_vector-1;
								}
								else
								{
									for(int j=0; j<All_AOI[AOIth].AOI_frame_size(); j++)
									{									
										if(video_position == All_AOI[AOIth].AOI_frame(j))
										{
											isMatch = true;
											AOIindex = j;
											if(All_AOI[AOIth].get_isAdjust(j))
											{
												isAdjust = true;
											}
											
											break;
										}
										
										/*
										if(video_position < All_AOI[AOIth].AOI_frame(j))
										{
											break;
										}
										*/
									}
								}
								
								if(!isMatch || isAdjust)//之前沒做過匹配或有被手動調整
								{
									//Tracking ...
									AOI_object_tracking(M_frame, AOIth, AOIindex, video_position, isAdjust);
									
									draw_box( image, video_position, PURPLE_REC );						
								}
								else//之前已做過匹配(倒帶回來時不用再做tracking一次)
								{
									draw_box( image, video_position, PURPLE_REC );
								}
							//}
						}		
					}
					else//內插影格
					{
						draw_box( image, video_position , RED_REC );
					}//end 匹配影格
				}
				else//手動模式AOI
				{
					draw_box( image, video_position , RED_REC );
				}//end 自動模式AOI 
			}//end for()
			//draw_box( image, video_position , RED_REC );
		}//end 連續影格的第一張frame
		
        cvShowImage( name, image );	
        //type=cvWaitKey( speed );//在這中間有按按鈕，就會繼續下去
		type=cvWaitKey( 1 );//在這中間有按按鈕，就會繼續下去
		
        //some operations by keyboard when the video not pause
        // press "Esc" and exit	
		//////////////////////////////////////////////////////////////////////////////////////////
        if( type==27 )
        {
            wxMessageDialog *dial = new wxMessageDialog( NULL, 
                wxT( "Do you want to save the Dynamic AOI data?" ), wxT( "Exit" ), 
                wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION );
            if( dial->ShowModal( )==wxID_YES )
                outputAOI();
            break;
        }
        // press "space" and pause
        else if( type==' ' )
		{
            Pause = true;
			pause_count++;
		}
        else if( type==','&&display_speed<1 )//播放速度/2
        {
            display_speed++;
            speed = speed*2;
        }
        else if( type=='.'&&display_speed>-1 )//播放速度*2
        {
            display_speed--;
            if( speed/2>=1 )
                speed = speed/2;
        }
        else if( type=='/'||display_speed==0 )//原始播放速度
        {
            display_speed=0;
            speed = Ini_speed;
        }
        ////////////////////////////////////////////////////////////////////////////////////////
		
        if( Pause )//影片暫停時可進行的動作
        {
            cvSetMouseCallback( name, my_mouse_callback, (void*) image );
            while( 1 )
            {
                //cvCopyImage( image, temp );
				cvCopy( image, temp );//opencv3.0 函式名稱有變

                if( drawAOI || moveAOI || adjustAOI )
                    draw_box( temp, video_position , RED_REC);

                cvShowImage( name, temp );

                // some operations by keyborad when the video pause
                type=cvWaitKey( speed );
                if( type==' ' )//暫停 -> 開始
                {
                    Pause = false;
					
					pause_count++;
                    break;
                }
                else if( type=='v' )//倒帶
                {
                    if( video_position>0 )
                        video_position--;
						
					forward_rewind_cound++;
                    break;
                }
                else if( type=='n' && video_position!=frames-2 )//前進
                {
                    if( video_position+1>frames )
                        video_position=frames-2;
                    else
                        video_position++;
					
					forward_rewind_cound++;
                    break;
                }
                else if( type=='f' )//倒帶
                {
                    if( video_position<10 )
                        video_position=0;
                    else
                        video_position-=10;
					forward_rewind_cound++;
                    break;
                }
                else if( type=='h'&&video_position!=frames-2 )//前進
                {
                    if( video_position+10>frames )
                        video_position=frames-2;
                    else
                        video_position+=10;
					forward_rewind_cound++;
                    break;
                }
                else if( type=='r' )//倒帶
                {
                    if( video_position<100 )
                        video_position=0;
                    else
                        video_position-=100;
					forward_rewind_cound++;
                    break;
                }
                else if( type=='y'&&video_position!=frames-2 )//前進
                {
                    if( video_position+100>frames )
                        video_position=frames-2;
                    else
                        video_position+=100;
					forward_rewind_cound++;
                    break;
                }
				else if( type=='d'&&video_position!=frames-2 )//前進
                {
                    if( video_position+N>frames )
                        video_position=frames-2;
                    else
                        video_position+=N;
                    break;
                }
				else if( type=='a'&&video_position!=frames-2 )//
                {
                    if( video_position<N )
                        video_position=0;
                    else
                        video_position-=N;
                    break;
                }
				
            }//while(1)
			
            cvSetMouseCallback( name, NULL, NULL );
			
        }//if(pause)
        cvReleaseImage( &temp );
        cvReleaseImage( &origin_image );
		//cvReleaseImage( &temp );
		
		if(!Pause)
		{
			video_position++;
		}
		
    }//while( (!nofile)&&(!videoEnd)  )
	
    cvReleaseCapture( &capture );
	cvReleaseCapture( &pre_capture );
    cvDestroyWindow( name );
	
	fp.close();
    isAuto_mode = false;
	
	fp_AOISetting << "forward_rewind: " << forward_rewind_cound << "  Pause: " << pause_count << "  Mouse_Operation_Count: " << mouse_operation_count << endl;
	fp_AOISetting.close();
	
	return;
}

/*
 * WxButton3Click, Load defined AOI File
 */
void ManualAuto::WxButton3Click(wxCommandEvent& event)
{
	// insert your code here
	WxOpenFileDialog1->SetPath( dynamicAOI );
	if( WxOpenFileDialog1->ShowModal( ) )
    {
        dynamicAOI = WxOpenFileDialog1->GetPath( );
        //wxMessageBox( dynamicAOI );
    }
    defined_dynamicAOI = dynamicAOI;
    wxMessageBox(defined_dynamicAOI);   
}

void AutoMode_Initail(int frames)
{
	scene_change_value_frame.clear();
	scene_change_value_frame.resize(frames);
	isFaceDetect_frame.clear();
	isFaceDetect_frame.resize(frames);

	for(int i=0; i<frames; i++)
	{
		scene_change_value_frame[i] = -1;
		isFaceDetect_frame[i] = false;
	}
	
	return;
}

//上一個連續影格中的AOI設置結束位置
void AOI_Period_Setting(int pre_pos, Mat frame)
{
	M_preframe = cvarrToMat(pre_image);
	for(int AOIth=0; AOIth<All_AOI.size(); AOIth++)
	{
		int Last_AOIindex = All_AOI[AOIth].AOI_frame_size()-1;
		int First_AOIindex_pos = All_AOI[AOIth].AOI_frame(0);
		if( (All_AOI[AOIth].AOI_frame(Last_AOIindex) == -1 && pre_pos > First_AOIindex_pos && Last_AOIindex!=0) ||  All_AOI[AOIth].get_isAdjust(Last_AOIindex) )
		{
			if( All_AOI[AOIth].AOI_frame(Last_AOIindex) == -1)//未完整tracking過
			{
				int AOIindex = Last_AOIindex-1;
				if(pre_pos != All_AOI[AOIth].AOI_frame(AOIindex))
				{
					AOI_object_tracking(M_preframe, AOIth, AOIindex, pre_pos, false);
				}
				Last_AOIindex = All_AOI[AOIth].AOI_frame_size()-1;
				All_AOI[AOIth].AOI_delete(Last_AOIindex);
			}
			else//完整tracking過，但後來AOI某個位置有調整
			{
				int AOIindex = Last_AOIindex;
				AOI_object_tracking(M_preframe, AOIth, AOIindex, pre_pos, true);
			}
			
			/*
			//刪掉最後一個-1
			All_AOI[AOIth].AOI_delete(Last_AOIindex);
			
			
			//traking 上一段連續影格的最後一張
			//AOI_object_tracking(M_frame, AOIth, AOIindex, video_position, isAdjust);
			
			
			//更新最後一個影格的AOI
			int last_x = All_AOI[AOIth].get_x(Last_AOIindex-1);
			int last_y = All_AOI[AOIth].get_y(Last_AOIindex-1);
			int last_width = All_AOI[AOIth].get_width(Last_AOIindex-1);
			int last_height = All_AOI[AOIth].get_height(Last_AOIindex-1);
			CvRect last_box = cvRect( last_x, last_y, last_width, last_height );
			All_AOI[AOIth].AOI_pushback(last_box, pre_pos);
			*/
			
			//tracker clear (不釋放最後用太多程式會爆掉，但用clear()似乎記憶體也沒有釋放，所以tracker重新initail()一個小區塊降低記憶體)
			Tracker_space_release(AOIth, M_preframe);
			
			
		}				
	}
	
	return;
}

void Tracker_space_release(int AOIth, Mat frame)
{
	Rect2d aoi;
	aoi.x = 10;
	aoi.y = 10;
	aoi.width = 10;
	aoi.height = 10;
			
	tracker[AOIth]->clear();
	Ptr<Tracker> trackerNew = Tracker::create( "KCF" );
			
	tracker[AOIth]=trackerNew;
	tracker[AOIth]->init( frame, aoi );

	return;
}


void Detect_Face(IplImage* img, int pos)
{   
    string cascade_name="haarcascades/haarcascade_frontalface_alt.xml";
    //haarcascade_mcs_mouth, haarcascade_mcs_nose, haarcascade_frontalface_default, haarcascade_frontalface_alt_tree, haarcascade_eye, haarcascade_upperbody, haarcascade_mcs_upperbody
    // Load cascade
    CvHaarClassifierCascade* classifier=(CvHaarClassifierCascade*)cvLoad(cascade_name.c_str(), 0, 0, 0);
    if(!classifier){
        cerr<<"ERROR: Could not load classifier cascade."<<endl;
        return;
    }
    CvMemStorage* facesMemStorage=cvCreateMemStorage(0);

    cvClearMemStorage(facesMemStorage);
    CvSeq* faces=cvHaarDetectObjects(img, classifier, facesMemStorage, 1.1, 3, CV_HAAR_DO_CANNY_PRUNING, cvSize(min_face_width, min_face_height));
    
    if(faces)
	{
		box_number=All_AOI.size( );
        for(int i=0; i<faces->total; i++)
		{
            // Setup two points that define the extremes of the rectangle,
            // then draw it to the image
            //原始抓到的人臉區域 
            CvPoint point1, point2;
            CvRect* rectangle = (CvRect*)cvGetSeqElem(faces, i);
            point1.x = rectangle->x;
            point2.x = rectangle->x + rectangle->width;          
            point1.y = rectangle->y;
            point2.y = rectangle->y + rectangle->height;
            //cvRectangle(img, point1, point2, CV_RGB(255,0,0), 3, 8, 0);
            
            //將抓到的人臉區域擴充至半身 
            CvPoint scale_point1, scale_point2;
			int face_height = rectangle->height;
			if( (float)(face_height)/(video_height) > 0.25 )
			{
				scale_point1.x = rectangle->x - 0.2*rectangle->width;
				scale_point1.y = rectangle->y - 0.1*rectangle->height;
				scale_point2.x = rectangle->x + 1.4*rectangle->width;
				scale_point2.y = rectangle->y + 1.6*rectangle->height;
			}
			else if( (float)(face_height)/(video_height) > 0.1 )
			{
				scale_point1.x = rectangle->x - 0.7*rectangle->width;
				scale_point1.y = rectangle->y - 0.3*rectangle->height;
				scale_point2.x = rectangle->x + 1.8*rectangle->width;
				scale_point2.y = rectangle->y + 3*rectangle->height;
			}
			else
			{
				scale_point1.x = rectangle->x - 1*rectangle->width;
				scale_point1.y = rectangle->y - 0.5*rectangle->height;
				scale_point2.x = rectangle->x + 2*rectangle->width;
				scale_point2.y = rectangle->y + 4.5*rectangle->height;
			}
            
            if(scale_point1.x<0)
                scale_point1.x = 0;
            if(scale_point1.y<0)
                scale_point1.y = 0;
            if(scale_point2.x > img->width)
                scale_point2.x = img->width;
            if(scale_point2.y > img->height)
                scale_point2.y = img->height;
            
			//把半身的範圍加入AOI
			AOI tempbox=AOI( scale_point1.x, scale_point1.y, video_position );
			All_AOI.push_back( tempbox );
			All_AOI[box_number +i].set_width( scale_point2.x - scale_point1.x, 0 );
			All_AOI[box_number +i].set_height( scale_point2.y - scale_point1.y, 0 );
			All_AOI[box_number +i].set_AOI_Type( AUTO );
			//All_AOI[box_number +i].set_isAdjust( false, 0 );
			
			isIniAOItracker.push_back(false);
			
            //cvRectangle(img, scale_point1, scale_point2, CV_RGB(255,0,0), 3, 8, 0);    
            
        }
    }
	cvReleaseMemStorage(&facesMemStorage);
	cvReleaseHaarClassifierCascade(&classifier);
	
	return;	
}

void AOI_tracking_Initial(Mat frame, int AOIth, int AOIIndex)
{
	//Mat M_img(img, 0); //opncv2.4 IplImage 轉 Mat 的用法
	//Mat frame = cvarrToMat(img);//opncv3.0 IplImage 轉 Mat 的用法...
	
	Rect2d aoi;
	aoi.x = All_AOI[AOIth].get_x(AOIIndex);
	aoi.y = All_AOI[AOIth].get_y(AOIIndex);
	aoi.width = All_AOI[AOIth].get_width(AOIIndex);
	aoi.height = All_AOI[AOIth].get_height(AOIIndex);
	
	if(aoi.width<10 || aoi.height<10)
		return;
		
	Ptr<Tracker> trackerNew = Tracker::create( "KCF" );
	tracker[AOIth]=trackerNew;
		
	//tracker[AOIth] = Tracker::create( "KCF" );
	tracker[AOIth] -> init(frame, aoi);	
	//test...
	tracker[AOIth] -> update(frame,aoi);
	
	
	isIniAOItracker[AOIth] = true;
	
	return;
}

void AOI_object_tracking(Mat frame, int AOIth, int AOIIndex, int pos, bool isAdjust)
{
	Rect2d aoi;
    
	tracker[AOIth]->update(frame,aoi);
	
	int x = aoi.x;
	int y = aoi.y;
	int width = aoi.width;
	int height = aoi.height;
	
	//CvRect box = cvRect( x, y, width, height );
	//All_AOI[AOIth].AOI_insert(box, AOIIndex+1, pos);
	
	//對 All_AOI 更新此範圍
	if(!isAdjust)//若是 新的AOI position, 則用insert
	{
		CvRect box = cvRect( x, y, width, height );
		All_AOI[AOIth].AOI_insert(box, AOIIndex+1, pos);
	}
	else//若是 調整過重作AOI position, 則用set
	{
		All_AOI[AOIth].set_x( x, AOIIndex );
		All_AOI[AOIth].set_y( y, AOIIndex );
		All_AOI[AOIth].set_width( width, AOIIndex );
		All_AOI[AOIth].set_height( height, AOIIndex );
		
		All_AOI[AOIth].set_isAdjust(false, AOIIndex);
	}
	
	
    return;
}


int scene_change_detect(IplImage* pre_img, IplImage* img)
{

	IplImage *grayImage, *pre_grayImage;
	pre_grayImage = cvCreateImage(cvGetSize(pre_img), IPL_DEPTH_8U, 1);//1代表灰階圖
	grayImage = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);//1代表灰階圖
	
	cvCvtColor(img, grayImage, CV_RGB2GRAY);
	cvCvtColor(pre_img, pre_grayImage, CV_RGB2GRAY);
	
	//cvShowImage( "Test", grayImage );
	
	unsigned int sum=0;
	for(int i=0;i<grayImage->height;i++){
        for(int j=0;j<grayImage->widthStep;j=j+3){
			//fp << (int)grayImage->imageData[i*grayImage->widthStep+j] << "," << (int)pre_grayImage->imageData[i*pre_grayImage->widthStep+j] << "," << endl;
            sum += abs((int)((char)grayImage->imageData[i*grayImage->widthStep+j] - (char)pre_grayImage->imageData[i*pre_grayImage->widthStep+j]));
            //Green[i][(int)(j/3)]=Image1->imageData[i*Image1->widthStep+j+1];
            //Red[i][(int)(j/3)]=Image1->imageData[i*Image1->widthStep+j+2];
        }
    }
	int XY = grayImage->height*grayImage->width;
	int scene_change_value = (sum*3)/XY;
	
	//fp << scene_change_value << "," << endl;
	
	cvReleaseImage( &pre_grayImage );
	cvReleaseImage( &grayImage );
	
	return scene_change_value;
	
}






















