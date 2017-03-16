#include <memory>

#include <wx/wx.h>
#include "cTwoD.h"
#include "cSimulator.h"

namespace train
{

void cTwoD::Render( wxDC& DC )
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

void cTwoD::RenderTrack( wxDC& DC, int margin )
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

void cTwoD::RenderTracks( wxDC& DC )
{
    wxPen pen( *wxBLUE_PEN );
    pen.SetWidth( 5 );
    DC.SetPen( pen );
    RenderTrack( DC, 10 );
    RenderTrack( DC, 30 );
    DC.SetPen( *wxBLUE_PEN );
}
void cTwoD::SetConvertL2P()
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
void cTwoD::RenderStations( wxDC& DC )
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
void cTwoD::RenderTrains( wxDC& DC )
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
   cTwoD::edge cTwoD::Convert( int& x, int& y,       // pixel location
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

}

