#include <string>
#include <vector>
#include <memory>


#include <wx/wx.h>
#include <wx/app.h>
#include <wx/grid.h>
#include <wx/glcanvas.h>

#include "cStation.h"
#include "cTrain.h"
#include "cSimulator.h"
#include "c3D.h"
#include "cTwoD.h"

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------


train::cTwoD * theGraphic;
train::cThreeD * the3D;
train::cSimulator theSim;

/** Display table of stations */
class cDLGStations : public wxDialog
{
    wxGrid * myGrid;
public:
    cDLGStations( wxWindow* parent );

    void Populate();

    /** User has double clicked on station table

    Click on a particular station to zoom in the 3D animation on that stations
    Click on the table header row to zoom out on the entire railway

    */
    void onSelectStation( wxGridEvent& event );
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

    // event handlers
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

    theGraphic = new train::cTwoD(this);
    sizer->Add(theGraphic, 1, wxEXPAND);
    SetSizer(sizer);
    SetAutoLayout(true);

    the3D = new train::cThreeD( this );

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

void cDLGStations::onSelectStation( wxGridEvent& event )
{
    int index = event.GetRow();
    if( index >= 0 )
        the3D->FocusStation( theSim.Stations[ index ] );
    else
    {
        station_t null;
        the3D->FocusStation( null );
    }
}

    cDLGStations::cDLGStations( wxWindow* parent )
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

        Bind( wxEVT_GRID_CELL_LEFT_DCLICK, &cDLGStations::onSelectStation, this );
        Bind( wxEVT_GRID_LABEL_LEFT_DCLICK, &cDLGStations::onSelectStation, this );

    }

        void cDLGStations::Populate()
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
