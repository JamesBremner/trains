#include <string>
#include <vector>
#include <memory>

// Include GLEW
#define GLEW_STATIC
#include <GL/glew.h>
#include "glm.hpp"
#include <gtc/matrix_transform.hpp>

#include <wx/wx.h>
#include <wx/app.h>
#include <wx/grid.h>
#include <wx/glcanvas.h>

#include "cStation.h"
#include "cTrain.h"
#include "cSimulator.h"
#include "c3D.h"

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

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
        DC.SetTextForeground( *wxWHITE );

        SetConvertL2P();

        DC.DrawText( theSim.myTitle.c_str(),
                     0.4 * myWindowWidth, 10 );

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


cGraphic * theGraphic;
train::c3D * the3D;
train::cSimulator theSim;

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
        for( auto station : theSim.Stations )
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
        for( auto train : theSim.Trains )
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
    void ConstructMenu();

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnSize(wxSizeEvent& event);
    void onTimer( wxTimerEvent& event);
    void onViewStations(wxCommandEvent& WXUNUSED(event));
    void onViewTrains(wxCommandEvent& WXUNUSED(event));
    void onView2D(wxCommandEvent& WXUNUSED(event));
    void onView3D(wxCommandEvent& WXUNUSED(event));
    void onFileOpen(wxCommandEvent& WXUNUSED(event));
    void onFileSave(wxCommandEvent& WXUNUSED(event));

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
    IDM_FILE_OPEN,
    IDM_FILE_SAVE,
    IDM_VIEW_STATIONS,
    IDM_VIEW_TRAINS,
    IDM_VIEW_2D,
    IDM_VIEW_3D,
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
        station_t stat( new train::cStation(
                            StationNames[k],
                            k * 1000 / 13 ));
        theSim.Stations.push_back( stat );
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


    ConstructMenu();

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

    the3D = new train::c3D( this );

    Show();

    dlgStations = new cDLGStations(this);
    dlgTrains = new cDLGTrains(this);

    Bind(wxEVT_SIZE,&MyFrame::OnSize,this);
    Bind(wxEVT_TIMER, &MyFrame::onTimer,this);

    myTimer.Start( 100 );
}

void MyFrame::ConstructMenu()
{

    wxMenuBar *menuBar = new wxMenuBar();

    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append(IDM_FILE_OPEN,"Open");
    Bind(wxEVT_MENU,&MyFrame::onFileOpen,this, IDM_FILE_OPEN);
    fileMenu->Append(IDM_FILE_SAVE,"Save");
    Bind(wxEVT_MENU,&MyFrame::onFileSave,this, IDM_FILE_SAVE);

    fileMenu->Append(Minimal_Quit, _T("E&xit\tAlt-X"), _T("Quit this program"));
    menuBar->Append(fileMenu, _T("&File"));

    wxMenu *viewMenu = new wxMenu;
    wxMenu *tableMenu = new wxMenu;
    tableMenu->AppendRadioItem(IDM_VIEW_STATIONS, "Stations");
    Bind(wxEVT_MENU,&MyFrame::onViewStations,this,IDM_VIEW_STATIONS);
    tableMenu->AppendRadioItem(IDM_VIEW_TRAINS, "Trains");
    Bind(wxEVT_MENU,&MyFrame::onViewTrains,this,IDM_VIEW_TRAINS);
    viewMenu->AppendSubMenu( tableMenu, "Tables");
    wxMenu *animMenu = new wxMenu;
    animMenu->AppendRadioItem(IDM_VIEW_2D, "Two D ( GDI )");
    Bind(wxEVT_MENU,&MyFrame::onView2D,this,IDM_VIEW_2D);
    animMenu->AppendRadioItem(IDM_VIEW_3D, "Three D ( OpenGL )");
    Bind(wxEVT_MENU,&MyFrame::onView3D,this,IDM_VIEW_3D);
    menuBar->Append(viewMenu, _T("View"));
    viewMenu->AppendSubMenu( animMenu, "Animation");

    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(Minimal_About, _T("&About...\tF1"), _T("Show about dialog"));
    menuBar->Append(helpMenu, _T("&Help"));

    SetMenuBar(menuBar);
}

// event handlers

void MyFrame::OnSize(wxSizeEvent& event )
{
    Refresh();

    int w, h;
    GetClientSize( &w, &h );
    the3D->Size( w, h );

    event.Skip();
}

void MyFrame::onTimer( wxTimerEvent& event)
{
    theSim.Advance();

    Refresh();

    the3D->Render();

    if( theSim.Time == 1 )
    {
        dlgStations->Populate();
    }
    dlgTrains->Populate();
}

void MyFrame::onFileSave(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog        saveFileDialog(this, _("Save file"), "", "",
                                       "", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;     // the user changed idea...

    if( ! theSim.FileSave( saveFileDialog.GetPath().ToStdString() ) )
        wxMessageBox("Failed to save model");
}
void MyFrame::onFileOpen(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog
    openFileDialog(this, _("Open  file"), "", "",
                   "", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;     // the user changed idea...

    if( ! theSim.FileOpen( openFileDialog.GetPath().ToStdString() ) )
        wxMessageBox("Failed to read model");

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
void MyFrame::onView2D(wxCommandEvent& WXUNUSED(event))
{
    the3D->myCanvas->Hide();
    theGraphic->Show();
}
void MyFrame::onView3D(wxCommandEvent& WXUNUSED(event))
{
     the3D->myCanvas->Show();
    theGraphic->Hide();
       int w, h;
    GetClientSize( &w, &h );
    the3D->Size( w, h );

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

    myConvertL2P = (float)myMaxP / theSim.Stations.back()->myLocation;

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



    for ( auto stat : theSim.Stations )
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

    for( auto train : theSim.Trains )
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
