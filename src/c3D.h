namespace train {
class c3D
{
public:
    wxGLCanvas * myCanvas;

    c3D( wxWindow * parent );

    void Render();
    void RenderTracks();

private:

    std::vector< float > myVx;
    std::vector< float > myClr;
    GLuint myMatrixID;
    GLuint myShaderID;
    GLuint myVertexBufferID;
    GLuint myColorBufferID;

    /** Load GL shaders into GPU */
    void LoadShaders();

    void RenderTrack( float margin );

    void DrawLine(
                   float x1, float y1, float x2, float y2,
                   float width2, bool isVertical );

    void Camera();
};

}
