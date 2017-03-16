namespace train {
/** Graphical display of railway in two dimension ( GDI )

L   distance from A in real world along railway line

P   distance from A in pixels along railway line

X,Y  location in pixels on display screen

 */
class cTwoD : public wxPanel
{
public:
    cTwoD( wxFrame* parent )
        : wxPanel(parent)
    {
        Bind( wxEVT_PAINT, &cTwoD::OnPaint, this );
    }
    void OnPaint( wxPaintEvent& event )
    {
        wxPaintDC DC( this );
        Render( DC );
    }

    void SetConvertL2P();

    void RenderTrack( wxDC& DC, int margin );


    void RenderTracks( wxDC& DC );


    void RenderStations( wxDC& DC );

    void RenderTrains( wxDC& DC );

    void Render( wxDC& DC );

    enum class edge
    {
        none, top, right, bottom, left
    };
    edge Convert( int& x, int& y,       // pixel location
                  int loc,              // location from terminus A
                  int margin
                );


private:
    int myWindowWidth;
    int myWindowHeight;
    int myMaxP;
    int myTopRightP;
    int myBottomRightP;
    int myTopLeftP;
    float myConvertL2P;     // convert factor from real world distance to pixels

};
}


