#include <string>
#include <vector>
#include <memory>

#include <wx/wx.h>
#include <wx/app.h>
#include <wx/grid.h>


// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------


class cStation
{
public:
    std::string myName;
    int myLocation;


    cStation( const std::string& name, int loc )
        : myName( name )
        , myLocation( loc )
        , myLightGreen( true )
        , myTrackBetaLightGreen( true )
    {

    }

    bool IsLightGreen( int track )
    {
        if( track == 0 )
            return myLightGreen;
        else
            return myTrackBetaLightGreen;
    }
    void SetLight( int track, bool green )
    {
        if( track == 0 )
            myLightGreen = green;
        else
            myTrackBetaLightGreen = green;
    }

private:
    bool myLightGreen;
    bool myTrackBetaLightGreen;
};

typedef std::shared_ptr< cStation > station_t;

class cTrain
{
public:

    cTrain( int track );

    void Advance();

    int LocationL()
    {
        return myLocationL;
    }
    int Track()
    {
        return myTrack;
    }

    std::string TextStatus();
    std::string TextPosition();
    std::string TextTrack();

    bool IsAtDestination() const
    {
        return myfDestination;
    }

private:
    int myTrack;
    int myLocationL;
    bool myfInStation;
    bool myfDestination;
    int myTimeToDepart;
    int myIncrement;
    station_t myStation;
    station_t myPrevStation;
};


typedef std::shared_ptr< cTrain > train_t;


/** Graphical display of railway

L   distance from A in real world along railway line

P   distance from A in pixels along railway line

X,Y  location in pixels on display screen

 */
class cGraphic : public wxPanel
{
public:
    cGraphic( wxFrame* parent )
        : wxPanel(parent)
    {
        //wxClientDC DC(this);
        // Render(DC);
        Bind( wxEVT_PAINT, &cGraphic::OnPaint, this );
    }
    void OnPaint( wxPaintEvent& event )
    {
        wxPaintDC DC( this );
        Render( DC );
    }

    void SetConvertL2P();

    void RenderTrack( wxDC& DC, int margin )
    {

        DC.DrawLine( 0.667 * myWindowWidth, margin,
                     myWindowWidth - margin, margin  );

        DC.DrawLine( myWindowWidth - margin, margin,
                     myWindowWidth - margin, myWindowHeight - margin );

        DC.DrawLine( myWindowWidth - margin, myWindowHeight - margin,
                     margin, myWindowHeight - margin );

        DC.DrawLine( margin, myWindowHeight - margin,
                     margin, margin );

        DC.DrawLine( margin, margin,
                     0.333 * myWindowWidth, margin );

    }

    void RenderTracks( wxDC& DC )
    {
        wxPen pen( *wxBLUE_PEN );
        pen.SetWidth( 5 );
        DC.SetPen( pen );
        RenderTrack( DC, 10 );
        RenderTrack( DC, 30 );
        DC.SetPen( *wxBLUE_PEN );
    }


    void RenderStations( wxDC& DC );

    void RenderTrains( wxDC& DC );

    void Render( wxDC& DC )
    {
        SetBackgroundColour( *wxBLACK );

        SetConvertL2P();

        RenderTracks( DC );

        RenderStations( DC );

        RenderTrains( DC );

    }
    enum class edge
    {
        none, top, right, bottom, left
    };
    edge Convert( int& x, int& y,       // pixel location
                  int loc,              // location from terminus A
                  int margin
                )
    {

        float locP = loc * myConvertL2P;

        if( locP < myTopRightP )
        {
            x = .667 * myWindowWidth + locP;
            y = margin;
            return edge::top;
        }
        if( locP < myBottomRightP )
        {
            x = myWindowWidth - margin;
            y = locP - myTopRightP;
            return edge::right;
        }
        if( locP < 1.33 * myWindowWidth + myWindowHeight )
        {
            x = myWindowWidth - locP + myBottomRightP;
            y = myWindowHeight - margin;
            return edge::bottom;
        }
        if( locP < myTopLeftP )
        {
            x = margin;
            y = myWindowHeight - locP + 1.33 * myWindowWidth + myWindowHeight;
            return edge::left;
        }
        if( locP <= myMaxP )
        {
            x = locP - myTopLeftP;
            y = margin;
            return edge::top;
        }

        x = -10;
        y = -10;
        return edge::none;
    }


private:
    int myWindowWidth;
    int myWindowHeight;
    int myMaxP;
    int myTopRightP;
    int myBottomRightP;
    int myTopLeftP;
    float myConvertL2P;

};

class cSimulateTrains
{
public:

    void Advance();

    /** remove trains from simulation that have reached their destination */
    void RemoveTrainsAtDestination();
};

int theTime;
std::vector< station_t > theStations;
std::vector< train_t > theTrains;
cGraphic * theGraphic;

cSimulateTrains theSim;

class cDLGStations : public wxDialog
{
    wxGrid * myGrid;
public:
    cDLGStations( wxWindow* parent )
        : wxDialog( parent, -1, "Stations" )
    {
        wxBoxSizer * szr = new wxBoxSizer( wxVERTICAL );
        myGrid = new wxGrid( this, -1,
                             wxDefaultPosition, // position
                             wxSize(-1,-1) );
        myGrid->CreateGrid( 14, 2 );
        myGrid->EnableEditing( false );
        myGrid->SetColLabelValue(0,"Name");
        myGrid->SetColSize( 0, 220 );
        myGrid->SetColLabelValue(1,"Distance");

        szr->Add( myGrid );
        SetSizerAndFit( szr );

    }

    void Populate()
    {
        int row = 0;
        for( auto station : theStations )
        {
            myGrid->SetCellValue( row,0, station->myName );
            myGrid->SetCellValue( row,1,
                                  wxString::Format("%d",station->myLocation ));
            row++;
        }
    }
};


class cDLGTrains : public wxDialog
{
    wxGrid * myGrid;
public:
    cDLGTrains( wxWindow* parent )
        : wxDialog( parent, -1, "Trains" )
    {
        wxBoxSizer * szr = new wxBoxSizer( wxVERTICAL );
        myGrid = new wxGrid( this, -1,
                             wxDefaultPosition, // position
                             wxSize(400,-1) );
        myGrid->CreateGrid( 100, 3 );
        myGrid->EnableEditing( false );
        myGrid->SetColLabelValue(0,"Status");
        myGrid->SetColLabelValue(1,"Location");
        myGrid->SetColLabelValue(2,"Track");

        szr->Add( myGrid );
        SetSizer( szr );

    }

    void Populate()
    {
        myGrid->ClearGrid();
        int row = 0;
        for( auto train : theTrains )
        {
            myGrid->SetCellValue( row,0, train->TextStatus() );
            myGrid->SetCellValue( row,1,
                                  train->TextPosition());
            myGrid->SetCellValue( row,2,
                                  train->TextTrack());
            row++;
        }
    }
};

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
// override base class virtuals
// ----------------------------

// this one is called on application startup and is a good place for the app
// initialization (doing it here and not in the ctor allows to have an error
// return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
};

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:

    // ctor(s)
    MyFrame(const wxString& title);

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnTimer( wxTimerEvent& event);
    void onViewStations(wxCommandEvent& WXUNUSED(event));
    void onViewTrains(wxCommandEvent& WXUNUSED(event));

    wxTimer myTimer;
    cDLGStations * dlgStations;
    cDLGTrains * dlgTrains;


private:
    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()

};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    Minimal_Quit = wxID_EXIT,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Minimal_About = wxID_ABOUT,

    TIMER_ID,
    IDM_VIEW_STATIONS,
    IDM_VIEW_TRAINS,
};

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(Minimal_Quit,  MyFrame::OnQuit)
    EVT_MENU(Minimal_About, MyFrame::OnAbout)

END_EVENT_TABLE()

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;

    // create the main application window
    MyFrame *frame = new MyFrame(_T("Trains"));

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    //frame->Show(true);

    std::vector< std::string > StationNames
    {
        "berceni",
        "Dimitrie Leonida",
        "Aparatorii Patrei",
        "Piata Sudului",
        "Constantin Brancoveanu",
        "Eroii Revoltiei",
        "Tineretului",
        "Piata Unirii",
        "Universtate",
        "Piata Romana",
        "Piata Victoriei",
        "Aviatorilor",
        "Aurel Viaacu",
        "Pipera"
    };

    for( int k = 0; k < 14; k++ )
    {
        station_t stat( new cStation(
                            StationNames[k],
                            k * 1000 / 13 ));
        theStations.push_back( stat );
    }

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------



// frame constructor
MyFrame::MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title)
    , myTimer(this, TIMER_ID)
{
    // set the frame icon
    SetIcon(wxICON(sample));


    // create a menu bar
    wxMenu *fileMenu = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(Minimal_About, _T("&About...\tF1"), _T("Show about dialog"));

    fileMenu->Append(Minimal_Quit, _T("E&xit\tAlt-X"), _T("Quit this program"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, _T("&File"));

    wxMenu *viewMenu = new wxMenu;
    viewMenu->AppendRadioItem(IDM_VIEW_STATIONS, "Stations");
    Bind(wxEVT_MENU,&MyFrame::onViewStations,this,IDM_VIEW_STATIONS);
    viewMenu->AppendRadioItem(IDM_VIEW_TRAINS, "Trains");
    Bind(wxEVT_MENU,&MyFrame::onViewTrains,this,IDM_VIEW_TRAINS);
    menuBar->Append(viewMenu, _T("View"));

    menuBar->Append(helpMenu, _T("&Help"));


    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);

//
//#if wxUSE_STATUSBAR
//    // create a status bar just for fun (by default with 1 pane only)
//    CreateStatusBar(2);
//    SetStatusText(_T("Welcome to Pinmed Licenser!"));
//#endif // wxUSE_STATUSBAR

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    theGraphic = new cGraphic(this);
    sizer->Add(theGraphic, 1, wxEXPAND);
    SetSizer(sizer);
    SetAutoLayout(true);

    Show();

    dlgStations = new cDLGStations(this);
    dlgTrains = new cDLGTrains(this);

    Bind(wxEVT_SIZE,&MyFrame::OnSize,this);
    Bind(wxEVT_TIMER, &MyFrame::OnTimer,this);

    theTime = 0;
    myTimer.Start( 100 );
}


// event handlers

void MyFrame::OnSize(wxSizeEvent& event )
{
    Refresh();

    event.Skip();
}

void MyFrame::OnTimer( wxTimerEvent& event)
{
    theTime++;

    if( theTime == 1 )
    {
        dlgStations->Populate();
    }

    if( ! ( theTime % 50 ) )
    {
        theTrains.push_back( train_t( new cTrain( 0 )));
        theTrains.push_back( train_t( new cTrain( 1 )));
    }

    for( auto train : theTrains )
    {
        train->Advance();
    }

    Refresh();

    theSim.Advance();

    dlgTrains->Populate();
}

void MyFrame::onViewStations(wxCommandEvent& WXUNUSED(event))
{
    dlgStations->Show();
    dlgTrains->Hide();
}
void MyFrame::onViewTrains(wxCommandEvent& WXUNUSED(event))
{
    dlgStations->Hide();
    dlgTrains->Show();
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox("Train Simulator",
                 _T("Trains"),
                 wxOK | wxICON_INFORMATION,
                 this);
}

void cGraphic::SetConvertL2P()
{
    GetClientSize( &myWindowWidth, &myWindowHeight );

//        myWindowHeight -= 50;
//        myWindowWidth  -= 50;

    // Maximum line length in pixels
    myMaxP = 1.667 * myWindowWidth + 2.0 * myWindowHeight;

    myConvertL2P = (float)myMaxP / theStations.back()->myLocation;

    myTopRightP = 0.33 * myWindowWidth;
    myBottomRightP = myTopRightP + myWindowHeight;
    myTopLeftP = 1.333 * myWindowWidth + 2 * myWindowHeight;

}
void cGraphic::RenderStations( wxDC& DC )
{
    wxPen green( *wxGREEN_PEN );
    green.SetWidth( 3 );

    wxPen red( *wxRED_PEN );
    red.SetWidth( 3 );

    DC.SetTextForeground( *wxWHITE );

    for ( auto stat : theStations )
    {
        int x, y;
        edge E = Convert(
                     x, y,
                     stat->myLocation,
                     20
                 );
        DC.SetPen( *wxBLACK_PEN );
        DC.DrawCircle( x, y, 20 );

        // station name
        int xt = x;
        int yt = y;
        switch( E )
        {
        case edge::top:
            xt -= 50;
            yt += 20;
            break;
        case edge::right:
            xt -= 130;
            break;
        case edge::bottom:
            xt -= 50;
            yt -= 40;
            break;
        case edge::left:
            xt += 40;
            break;

        }
        DC.DrawText( stat->myName, xt, yt );


        if( stat->IsLightGreen( 0 ) )
            DC.SetPen( green );
        else
            DC.SetPen( red );

        Convert(
            x, y,
            stat->myLocation + 10,
            10
        );

        DC.DrawCircle( x, y, 5 );


        if( stat->IsLightGreen( 1 ) )
            DC.SetPen( green );
        else
            DC.SetPen( red );

        Convert(
            x, y,
            stat->myLocation - 10,
            30
        );

        DC.DrawCircle( x, y, 5 );


    }

    DC.SetPen( *wxWHITE_PEN );
}
void cGraphic::RenderTrains( wxDC& DC )
{
    wxPen pen( *wxGREEN_PEN );
    pen.SetWidth( 8 );
    DC.SetPen( pen );

    for( auto train : theTrains )
    {
        int margin = 10;
        if( train->Track() == 1 )
            margin = 30;
        wxPoint p1, p2;
        Convert(
            p1.x, p1.y,
            train->LocationL()-2,
            margin);
        Convert(
            p2.x, p2.y,
            train->LocationL()+2,
            margin );
        if( p2.x < 0 )
        {
            DC.SetPen( *wxBLUE_PEN );
            return;
        }
        DC.DrawLine( p1, p2 );
    }

    DC.SetPen( *wxBLUE_PEN );
}

cTrain::cTrain( int track )
    : myTrack( track )
    , myfInStation( true )
    , myfDestination( false )
    , myTimeToDepart( theTime )
{
    if( myTrack == 0 )
    {
        myIncrement = 5;
        myLocationL = 0;
        myStation = *theStations.begin();
    }
    else
    {
        myLocationL = theStations.back()->myLocation - 2;
        myStation = theStations.back();
        myIncrement = -5;
    }
}

void cTrain::Advance()
{
    if( myfInStation )
    {
        if( theTime < myTimeToDepart )
            return;

        // train ready to leave station
        // check the light
        if( ! myStation->IsLightGreen( myTrack ) )
            return;

        // Off we go!
        if( myPrevStation )
            myPrevStation->SetLight( myTrack, true );
        myPrevStation = myStation;
        myStation->SetLight( myTrack, false );
        myfInStation = false;
        if( myTrack == 0 )
            myLocationL += 6;
        else
            myLocationL -= 6;
    }

    myLocationL += myIncrement;


    for( auto station : theStations )
    {
        if( abs( myLocationL - station->myLocation) < 5 )
        {
            // train has arrived in station

            myLocationL  = station->myLocation;
            myfInStation = true;
            myTimeToDepart = theTime + rand() % 10 + 1;
            myStation = station;

            // check if train is at final station
            if(
                ( myTrack == 0 && myLocationL == theStations.back()->myLocation ) ||
                ( myTrack == 1 && myLocationL == theStations[0]->myLocation ) )
            {
                myfDestination = true;
                myPrevStation->SetLight( myTrack, true );
            }
            break;
        }
    }
}
std::string cTrain::TextStatus()
{
    if( myfInStation )
        return "at";
    else
        return "leaving";
}
std::string cTrain::TextPosition()
{
    if( myPrevStation )
        return( myPrevStation->myName );
    else
        return "";
}
std::string cTrain::TextTrack()
{
    if( myTrack == 0 )
        return "Alpha";
    else
        return "Beta";
}

void cSimulateTrains::Advance()
{
    RemoveTrainsAtDestination();
}

void cSimulateTrains::RemoveTrainsAtDestination()
{
    theTrains.erase(
        std::remove_if( theTrains.begin(), theTrains.end(),
                        []( const train_t& train )
    {
        return train->IsAtDestination();
    }
                      ),
    theTrains.end());
}
