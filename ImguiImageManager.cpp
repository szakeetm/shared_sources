#include "ImguiImageManager.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>
#include <Helper/MathTools.h>
#include <GLToolkit.h>

ImImage::ImImage()
{
    if (glGetString(GL_VERSION)[0] == '1') oldOpenGL = true;
}

ImImage::ImImage(const std::string& filename)
{
    if (glGetString(GL_VERSION)[0] == '1') oldOpenGL = true;
    LoadFromFile(filename);
}

ImImage::~ImImage()
{
    free(GetTexture());
}

bool ImImage::LoadFromFile(const std::string& filename)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    int channels = 0;
    unsigned char* image_data = stbi_load(filename.c_str(), &image_width, &image_height, &channels, 4);
    if (image_data == NULL)
        return false;

    int newWidth = MathHelper::NextPowOfTwo(image_width);
    int newHeight = MathHelper::NextPowOfTwo(image_height);
    unsigned char* resizedData = new unsigned char[newWidth * newHeight * channels];
    if (oldOpenGL) {
        stbir_resize_uint8(image_data, image_width, image_height, 0, resizedData, newWidth, newHeight, 0, channels);
    }
    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    if (oldOpenGL) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same
    }

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif

    if (oldOpenGL) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, resizedData);
    }
    else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    }
    stbi_image_free(image_data);

    texture = image_texture;
    width = image_width;
    height = image_height;

    if (texture == 0) return false;
    if (width == 0) return false;
    if (height == 0) return false;

    return true;
}

void* ImImage::GetTexture()
{
    void* ret = (void*)(intptr_t)texture;
	return ret;
}

ImVec2 ImImage::GetSize()
{
	return ImVec2(static_cast<float>(width), static_cast<float>(height));
}

void ImImage::Draw()
{
	ImGui::Image(GetTexture(), GetSize(), ImVec2(0, 0), ImVec2(1, 1));
}

std::pair<ImVec2, ImVec2> ImImage::GetTextureUVs()
{
    ImVec2 UV0, UV1;
    return std::pair<ImVec2, ImVec2>(UV0, UV1);
}
