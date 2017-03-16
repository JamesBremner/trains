namespace train {
class cThreeD
{
public:
    wxGLCanvas * myCanvas;

    cThreeD( wxWindow * parent );

    void Render();
    void RenderTracks();
    void RenderTrains();
    void RenderStations();

    void Size( int w, int h );

    void FocusStation( station_t stat )
    {
        myFocusStation = stat;
    }


private:

    std::vector< float > myVx;
    std::vector< float > myClr;
    std::vector< float > myCurrentColor;
    GLuint myMatrixID;
    GLuint myShaderID;
    GLuint myVertexBufferID;
    GLuint myColorBufferID;
    int myWindowWidth;
    int myWindowHeight;
    float myConvertL2F;
    station_t myFocusStation;

    /** Load GL shaders into GPU */
    void LoadShaders();

    void RenderTrack( float margin );

    void DrawLine(
                   float x1, float y1, float x2, float y2,
                   float width2, bool isVertical );

    void Camera();

        enum class edge
    {
        none, top, right, bottom, left
    };
    edge Convert( float& x, float& y,       // pixel location
                  int loc,              // location from terminus A
                  float margin
                );

    void Tri2dTo3DVertex( float* pt, float z, int trcount );

};

}
