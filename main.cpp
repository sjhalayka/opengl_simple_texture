#include <GL/glut.h>
#include <stdio.h>



#include <vector>
#include <fstream>
using namespace std;




#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif


#ifdef _WIN32
#include <GL/gl.h>
#include <GL/glu.h>
#endif

typedef union PixelInfo
{
    std::uint32_t Colour;
    struct
    {
        std::uint8_t R, G, B, A;
    };
} *PPixelInfo;




class BMP
{
public:
    std::uint32_t width, height;
    std::uint16_t BitsPerPixel;
    std::vector<std::uint8_t> Pixels;

public:
    BMP(const char* FilePath);
    std::vector<std::uint8_t> GetPixels() const { return this->Pixels; }
    std::uint32_t GetWidth() const { return this->width; }
    std::uint32_t GetHeight() const { return this->height; }
    bool HasAlphaChannel() { return BitsPerPixel == 32; }
};

BMP::BMP(const char* FilePath)
{
    std::fstream hFile(FilePath, std::ios::in | std::ios::binary);
    if (!hFile.is_open()) throw std::invalid_argument("Error: File Not Found.");

    hFile.seekg(0, std::ios::end);
    std::size_t Length = hFile.tellg();
    hFile.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> FileInfo(Length);
    hFile.read(reinterpret_cast<char*>(FileInfo.data()), 54);

    if (FileInfo[0] != 'B' && FileInfo[1] != 'M')
    {
        hFile.close();
        throw std::invalid_argument("Error: Invalid File Format. Bitmap Required.");
    }

   // if (FileInfo[28] != 24 && FileInfo[28] != 32)
    if (FileInfo[28] != 32)
    {
        hFile.close();
        throw std::invalid_argument("Error: Invalid File Format. 32-bit Image Required.");
    }

    BitsPerPixel = FileInfo[28];
    width = FileInfo[18] + (FileInfo[19] << 8);
    height = FileInfo[22] + (FileInfo[23] << 8);
    std::uint32_t PixelsOffset = FileInfo[10] + (FileInfo[11] << 8);
    std::uint32_t size = ((width * BitsPerPixel + 31) / 32) * 4 * height;
    Pixels.resize(size);

    hFile.seekg(PixelsOffset, std::ios::beg);
    hFile.read(reinterpret_cast<char*>(Pixels.data()), size);
    hFile.close();
}




vector<GLfloat> pixels;

GLuint texid[] = { 0 };

static inline void init()
{
    BMP info = BMP("card_texture.bmp");

    const size_t num_channels = 4;

    pixels.resize(info.GetWidth() * info.GetHeight() * num_channels);

    for (size_t i = 0; i < info.GetWidth(); i++)
    {
        for (size_t j = 0; j < info.GetHeight(); j++)
        {
            pixels[num_channels * (i * info.GetHeight() + j) + 2] = info.Pixels[num_channels * (i * info.GetHeight() + j) + 0] / 255.0f;
            pixels[num_channels * (i * info.GetHeight() + j) + 1] = info.Pixels[num_channels * (i * info.GetHeight() + j) + 1] / 255.0f;
            pixels[num_channels * (i * info.GetHeight() + j) + 0] = info.Pixels[num_channels * (i * info.GetHeight() + j) + 2] / 255.0f;
            pixels[num_channels * (i * info.GetHeight() + j) + 3] = info.Pixels[num_channels * (i * info.GetHeight() + j) + 3] / 255.0f;
        }
    }

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, texid);
    glBindTexture(GL_TEXTURE_2D, texid[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, info.GetWidth(), info.GetHeight(), 0, GL_RGBA, GL_FLOAT, &pixels[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

static inline void display()
{
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2, 2, -2, 2, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(1, 1, 1);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    glBindTexture(GL_TEXTURE_2D, texid[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    glTexCoord2f(1, 0);
    glVertex2f(1, 0);
    glTexCoord2f(1, 1);
    glVertex2f(1, 1);
    glTexCoord2f(0, 1);
    glVertex2f(0, 1);
    glEnd();

    glutSwapBuffers();
}

int main(int argc, char** argv)
{
    glMatrixMode(GL_MODELVIEW);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(300, 300);
    glutCreateWindow("OpenGL Texture Experiment");
    init();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}