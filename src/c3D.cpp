#include <memory>

// Include GLEW
#define GLEW_STATIC
#include <GL/glew.h>
#include "glm.hpp"
#include <gtc/matrix_transform.hpp>

#include <wx/wx.h>
#include <wx/glcanvas.h>

#include "cStation.h"
#include "c3D.h"
#include "cSimulator.h"

namespace train
{
cThreeD::cThreeD( wxWindow * parent )
{
    wxGLAttributes dispAttrs;
    dispAttrs.PlatformDefaults().EndList();
    wxGLContextAttrs cxtAttrs;
    cxtAttrs.CoreProfile().OGLVersion(3, 3).Robust().EndList();
    myCanvas = new wxGLCanvas(
        parent,
        dispAttrs,
        -1,
        wxDefaultPosition,
        wxSize(500,500));
    wxGLContext * context = new wxGLContext( myCanvas, 0, &  cxtAttrs );
    if( ! context->IsOK() )
    {
        wxMessageBox( "GL context failed","WXGL");
        exit(1);
    }
    myCanvas->SetCurrent( *context);
    myCanvas->Hide();

    glewInit();

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    glViewport(0, 0, 500,500);
    glGenBuffers(1, &myVertexBufferID);
    glGenBuffers(1, &myColorBufferID);
    LoadShaders();
    myMatrixID = glGetUniformLocation(myShaderID, "MVP");
}


void cThreeD::Render()
{

    myConvertL2F = 7.33 / theSim.Stations.back()->myLocation;

    myVx.clear();
    myClr.clear();
    RenderTracks();
    RenderStations();
    RenderTrains();


    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    glUseProgram( myShaderID );

    Camera();

    // enable vertices attribute buffer
    glEnableVertexAttribArray(0);

    // make current
    glBindBuffer(GL_ARRAY_BUFFER, myVertexBufferID);

    // allocate storage in GPU and copy data
    glBufferData(
        GL_ARRAY_BUFFER,
        myVx.size()*sizeof(GL_FLOAT),
        myVx.data(),
        GL_STREAM_DRAW);

    // let shaders access buffer
    glVertexAttribPointer(
        0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, myColorBufferID);
    glBufferData(
        GL_ARRAY_BUFFER,
        myClr.size()*sizeof(GL_FLOAT),
        myClr.data(),
        GL_STREAM_DRAW);
    glVertexAttribPointer(
        1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
        3,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

    glDrawArrays(GL_TRIANGLES, 0, myVx.size()/3 );

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);


    glFlush();
    myCanvas->SwapBuffers();
}

void cThreeD::Size( int w, int h )
{
    myWindowWidth = w;
    myWindowHeight = h;
    myCanvas->SetSize( -1, -1, myWindowWidth, myWindowHeight );
    glViewport(0, 0, myWindowWidth, myWindowHeight);
}

void cThreeD::Camera()
{

    float aspectRatio = (float)myWindowWidth / myWindowHeight;

    glm::vec3 center(0,0,0);
    glm::mat4 ProjMatrix;


    if( ! myFocusStation )
    {
        float xSpan = 1;
        float ySpan = 1;

        if (aspectRatio > 1)
        {
            // Width > Height, so scale xSpan accordinly.
            xSpan *= aspectRatio;
        }
        else
        {
            // Height >= Width, so scale ySpan accordingly.
            ySpan = xSpan / aspectRatio;
        }

        ProjMatrix = glm::ortho(
                                   -1*xSpan, xSpan, -1*ySpan, ySpan,
                                   100.0f,-100.0f);
    }
    else
    {
        float xSpan = 0.2;
        float ySpan = 0.2;

        if (aspectRatio > 1)
        {
            // Width > Height, so scale xSpan accordinly.
            xSpan *= aspectRatio;
        }
        else
        {
            // Height >= Width, so scale ySpan accordingly.
            ySpan = xSpan / aspectRatio;
        }

        ProjMatrix = glm::ortho(
                                   -1*xSpan, xSpan, -1*ySpan, ySpan,
                                   100.0f,-100.0f);

        float x, y;
        Convert( x, y,
                 myFocusStation->myLocation,
                 0.075 );
        center = glm::vec3( x, y, 0 );

    }

    glm::mat4 ViewMatrix = glm::lookAt(
                               glm::vec3(0,0,10),           // Camera is here
                               center, // and looks here : at the same position, plus "direction"
                               glm::vec3(0,1,0)                // Head is up (set to 0,-1,0 to look upside-down)
                           );
    glm::mat4 MVP = ProjMatrix * ViewMatrix;
// Send our transformation to the currently bound shader,
// in the "MVP" uniform
    glUniformMatrix4fv(
        myMatrixID,
        1,
        GL_FALSE,
        (float*) &MVP );
}

void cThreeD::RenderStations()
{
    std::vector< float > white { 1, 1, 1 };
    std::vector< float > red { 1, 0, 0 };
    std::vector< float > green { 0, 1, 0 };

    for( auto stat : theSim.Stations )
    {
        myCurrentColor = white;
        float x1, y1, x2, y2;
        Convert(
            x1, y1,
            stat->myLocation-5,
            0.075);
        edge e = Convert(
                     x2, y2,
                     stat->myLocation+5,
                     0.075);
        bool fVert = true;
        if( e == edge::top || e == edge::bottom)
            fVert = false;

        DrawLine( x1, y1, x2, y2, .02, fVert );


        Convert(
            x1, y1,
            stat->myLocation - 10,
            0.05 );
        if( stat->IsLightGreen( 0 ) )
            myCurrentColor = green;
        else
            myCurrentColor = red;
        DrawLine( x1, y1, x1+.01, y1, .01, false );

        Convert(
            x1, y1,
            stat->myLocation + 10,
            0.1 );
        if( stat->IsLightGreen( 1 ) )
            myCurrentColor = green;
        else
            myCurrentColor = red;
        DrawLine( x1, y1, x1+.01, y1, .01, false );


    }
}

void cThreeD::RenderTracks()
{
    std::vector< float > color { 0, 0, 1 };
    myCurrentColor = color;
    RenderTrack( 0.05 );
    RenderTrack( 0.1 );
}

void cThreeD::RenderTrack( float margin )
{
    DrawLine(  0.33, 1 - margin, 1 - margin, 1 - margin, 0.01, false );
    DrawLine(  1 - margin, 1 - margin, 1 - margin, -1 + margin, 0.01, true );
    DrawLine(  1 - margin, -1 + margin, -1 + margin, -1 + margin, 0.01, false );
    DrawLine(  -1 + margin, 1 - margin, -1 + margin, -1 + margin, 0.01, true );
    DrawLine( -1 + margin, 1 - margin, -0.33, 1 - margin, 0.01, false );
}

void cThreeD::RenderTrains()
{
    std::vector< float > color { 0, 1, 0 };
    myCurrentColor = color;
    float margin = 0.05;
    //DrawLine( 0.5, 1 - margin, 0.55, 1 - margin, 0.04, false );

    for( auto train : theSim.Trains )
    {
        float margin = 0.05;
        if( train->Track() == 1 )
            margin = 0.1;
        float x1, y1, x2, y2;
        Convert(
            x1, y1,
            train->LocationL()-4,
            margin);
        edge e = Convert(
                     x2, y2,
                     train->LocationL()+4,
                     margin );
        bool fVert = true;
        if( e == edge::top || e == edge::bottom)
            fVert = false;
        DrawLine( x1, y1, x2, y2, 0.01, fVert );
    }
}

void cThreeD::DrawLine(
    float x1, float y1, float x2, float y2,
    float width2, bool isVertical )
{
    if( isVertical )
    {
        myVx.push_back( x1 - width2  );
        myVx.push_back( y1 );
        myVx.push_back( 0 );
        myVx.push_back( x1 + width2  );
        myVx.push_back( y1 );
        myVx.push_back( 0 );
        myVx.push_back( x2 - width2  );
        myVx.push_back( y2 );
        myVx.push_back( 0 );

        myVx.push_back( x2 + width2  );
        myVx.push_back( y1 );
        myVx.push_back( 0 );
        myVx.push_back( x2 - width2  );
        myVx.push_back( y2 );
        myVx.push_back( 0 );
        myVx.push_back( x1 + width2  );
        myVx.push_back( y2 );
        myVx.push_back( 0 );
    }
    else
    {
        myVx.push_back( x1  );
        myVx.push_back( y1 - width2 );
        myVx.push_back( 0 );
        myVx.push_back( x1  );
        myVx.push_back( y1 + width2 );
        myVx.push_back( 0 );
        myVx.push_back( x2  );
        myVx.push_back( y2 - width2 );
        myVx.push_back( 0 );

        myVx.push_back( x1  );
        myVx.push_back( y1 + width2 );
        myVx.push_back( 0 );
        myVx.push_back( x2  );
        myVx.push_back( y2 - width2 );
        myVx.push_back( 0 );
        myVx.push_back( x2  );
        myVx.push_back( y2 + width2 );
        myVx.push_back( 0 );

    }

    for( int k = 0; k<6; k++ )
    {
        myClr.insert( myClr.end(),
                      myCurrentColor.begin(), myCurrentColor.end());
    }
}

void cThreeD::LoadShaders()
{

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);


    std::string VertexShaderCode =
        "#version 330 core\n"
        "layout(location = 0) in vec3 vertexPosition_modelspace;\n"
        "layout(location = 1) in vec3 vertexColor;\n"
        "uniform mat4 MVP;\n"
        "out vec3 fragmentColor;\n"
        "void main(){\n"
        "    gl_Position =  MVP * vec4(vertexPosition_modelspace,1);\n"
        "    fragmentColor = vertexColor;\n"
        "}\n";

//        "  gl_Position =  MVP * vec4(vertexPosition_modelspace,1);\n"
//

    std::string FragmentShaderCode =
        "#version 330 core\n"
        "in vec3 fragmentColor;\n"
        "out vec3 color;\n"
        "void main()\n"
        "{\n"
        "	color = fragmentColor;\n"
        "}\n";


    //           "	color = vec3(1,0,0);\n"

    GLint Result = GL_FALSE;
    int InfoLogLength;


    // Compile Vertex ShadermyData.TraceLength
    //printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    if( Result == GL_FALSE )
    {
        glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 )
        {
            std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
            //printf("%s\n", &VertexShaderErrorMessage[0]);
            wxMessageBox(&VertexShaderErrorMessage[0],"Vertex shader failed");
            return ;
        }
    }



    // Compile Fragment Shader
    //printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    Result = GL_FALSE;
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    if( Result == GL_FALSE )
    {
        glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 )
        {
            std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
            // printf("%s\n", &FragmentShaderErrorMessage[0]);
            wxMessageBox(&FragmentShaderErrorMessage[0],"Fragment shader failed");
        }
    }



    // Link the program
    printf("Linking program\n");
    myShaderID = glCreateProgram();
    glAttachShader(myShaderID, VertexShaderID);
    glAttachShader(myShaderID, FragmentShaderID);
    glLinkProgram(myShaderID);

    // Check the program
    Result = GL_FALSE;
    glGetProgramiv(myShaderID, GL_LINK_STATUS, &Result);
    if( Result == GL_FALSE )
    {
        glGetProgramiv(myShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 )
        {
            std::vector<char> ProgramErrorMessage(InfoLogLength+1);
            glGetProgramInfoLog(myShaderID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
            //printf("%s\n", &ProgramErrorMessage[0]);
            wxMessageBox(&ProgramErrorMessage[0],"Linking shaders failed");
        }
    }


    glDetachShader(myShaderID, VertexShaderID);
    glDetachShader(myShaderID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);


}

cThreeD::edge cThreeD::Convert( float& x, float& y,       // pixel location
                                int loc,              // location from terminus A
                                float margin
                              )
{

    float locF = loc * myConvertL2F;
    if( locF < .667 )
    {
        x = .33 + locF;
        y = 1 - margin;
        return edge::top;
    }
    else if ( locF < 2.667 )
    {
        x = 1 - margin;
        y = 1 - locF + 0.667;
        return edge::right;
    }
    else if ( locF < 4.667 )
    {
        x = 1 - locF + 2.667;
        y = -1 + margin;
        return edge::bottom;
    }
    else if( locF < 6.667 )
    {
        x = -1 + margin;
        y = -1 + locF - 4.667;
        return edge::left;
    }
    else if( locF < 7.33 )
    {
        x = -1 + locF - 6.667;
        y = 1 - margin;
        return edge::top;
    }
}

}
