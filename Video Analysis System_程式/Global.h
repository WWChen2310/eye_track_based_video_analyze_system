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

// frames,fps,mspf are video data for drawing mouse position
extern int frames;
extern float fps;
extern float mspf; // ms per frame
extern int frame_width;
extern int frame_height;

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
extern vector<int> eye_time;
extern vector< vector<int> > other_eye_time;
//基於時間點的EyePosition
extern vector<int> eye_PositionX;
extern vector<int> eye_PositionY;
//基於時間點的複數EyePosition
extern vector<vector<int> > other_eye_PositionX;
extern vector<vector<int> > other_eye_PositionY;


// eye_PositionX, eye_PositionY for each frame
//基於Frame的EyePosition(於inputEyePosition()從時間點轉換至對應的frame )
extern vector<int> eye_PositionX_frame;
extern vector<int> eye_PositionY_frame;
//基於Frame的複數EyePosition(於input_otherEyePosition()從時間點轉換至對應的frame )
extern vector<vector<int> > other_eye_PositionX_frame;
extern vector<vector<int> > other_eye_PositionY_frame;

extern vector<AOI> All_AOI;
extern vector< vector<AOI> > other_All_AOI;//用於碩論實驗
extern bool drawAOI, moveAOI, adjustAOI, insertAOI, draw_eyePosition;
extern int oldboxx, oldboxy, oldx, oldy, box_number, video_position, pre_video_position;

// the index of AOI_frame_vector to match AOI
extern int AOIindex;
extern char type;
extern IplImage* origin_image;
extern IplImage* temp;
extern IplImage* pre_image;
//tracking
extern Mat M_frame;
extern Mat M_preframe;

extern CvCapture* capture;
extern CvCapture* pre_capture;
extern CvRect insertbox;

extern wxString video_path;//載入影片的路徑 
extern wxString video_name;//載入影片的名稱 
extern wxString dynamicAOI;//載入的AOI txt的路徑 
//extern wxString defined_dynamicAOI;//defined_dynamicAOI = dynamicAOI  
extern wxString eyePosition;//載入的eyemovement txt的路徑
extern wxArrayString other_eyePosition; //載入複數眼動資料的路徑
extern int other_eyePosition_num;////載入複數眼動資料的個數
extern wxArrayString other_AOI_path;//載入複數眼動資料的路徑 
extern int other_AOI_path_num;////載入複數眼動資料的個數(default=0)
extern wxArrayString other_AOI_name; 
extern wxString eyemovement_name;
extern wxArrayString group_eyemovement_name;


extern bool videoanalyzer_mode;
//Heat Map Radius
extern int heatmap_Radius;  

extern vector<bool> isIniAOItracker; 
//Auto AOI Setting mode flags
extern bool isAuto_mode ;


//碩論實驗: AOISetting 手動自動比較
extern int mouse_operation_count;

//碩論實驗: 比較不同間隔N與N=1(standard)的offset
extern fstream fp_standard_N;
extern char filename_standard_N[];

void initialVideo( );
void on_track_bar_slide( int pos );
void my_mouse_callback( int event, int x, int y, int flags, void* param );
int interpolation( int x, int y, int timeX, int timeY );
void draw_box( IplImage* img, int pos, int type );
void draw_HeatMap( IplImage* img, int pos );
int mouse_location( int x, int y, int pos );
int nearby_corner( int x, int y, int pos );
int change_name( int x, int y, int pos );
void outputAOI( );

//用於 analysis
void inputAOI( wxString dynamic_AOI );
void input_other_AOI( wxArrayString other_AOI_path );//用於碩論實驗
void inputEyePosition( wxString eye_Position );
void input_otherEyePosition( wxArrayString other_eyePosition );
void outputAnalysis(int type);
