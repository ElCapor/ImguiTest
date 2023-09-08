#ifndef TESTCLASS_H
#define TESTCLASS_H
#include <iostream>
#include <filesystem>
#include "imgui_impl_dx11.h"
#include "imgui_internal.h"
#include <DirectXTex.h>
#include "RenderManager.h"
#include <wincodec.h>
#include "imgui.h"

namespace fs = std::filesystem;
/// <summary>
/// A simple namespace to manager vertex buffer rotations with imgui
/// </summary>
namespace Rotation {
    int rotation_start_index;

    void ImRotateStart()
    {
        rotation_start_index = ImGui::GetWindowDrawList()->VtxBuffer.Size;
    }

    ImVec2 ImRotationCenter()
    {
        ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX); // bounds

        const auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
        for (int i = rotation_start_index; i < buf.Size; i++)
            l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);

        return ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2); // or use _ClipRectStack?
    }


    void ImRotateEnd(float rad, ImVec2 center = ImRotationCenter())
    {
        float s = sin(rad), c = cos(rad);
        center = ImRotate(center, s, c) - center;

        auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
        for (int i = rotation_start_index; i < buf.Size; i++)
            buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
    }
}


/// <summary>
/// A class to represent DirectX images
/// </summary>
class ImGuiImage {
public:
    /// <summary>
    /// Empty constructor , this doesnt initialize anything.
    /// </summary>
    ImGuiImage()
    {

        std::cout << "Cant do nothing with an empty constructor lol" << std::endl;

    }
    /// <summary>
    /// Returns an image from the path
    /// </summary>
    /// <param name="path">Path of the image file (it must be a png file)</param>
    ImGuiImage(const wchar_t* path)
    {
        this->rotation = 0;
        ID3D11Device* g_pd3dDevice = RenderManager::GetInstance()->GetDevice();
        // misname in the commit lmao
        if (fs::exists(path))
        {
            original_path = path;

            DirectX::ScratchImage tempImage; // the temp image to store from the file
            DirectX::TexMetadata tempMetadata;
            if (LoadFromFile(path, tempImage))
            {
                HRESULT hr;
                std::vector<uint8_t> tempBytes;
                if (ImageToBytes(tempBytes, tempImage))
                {
                    bytes = std::make_unique<std::vector<uint8_t>>(tempBytes);
                    image_info.height = tempImage.GetMetadata().height;
                    image_info.width = tempImage.GetMetadata().width;
                    image_info.imageSize = tempImage.GetMetadata().arraySize;
                    std::cout << "Bytes set successfully" << std::endl;

                    // we could use the tempImage that we made , but i did this to make sure that the bytes we set are correct

                    if (!bytes->empty()) // we perform check , cuz something bad could happen idk
                    {
                        // we do everything again to make sure it's correct
                        DirectX::ScratchImage anotherImage;
                        if (BytesToImage(*bytes, anotherImage))
                        {
                            ID3D11ShaderResourceView* srv;
                            hr = DirectX::CreateShaderResourceView(g_pd3dDevice, anotherImage.GetImages(), anotherImage.GetImageCount(), anotherImage.GetMetadata(), &srv);
                            if (SUCCEEDED(hr))
                            {
                                this->m_ImageID = (ImTextureID)srv;
                                std::cout << "Finished" << std::endl;
                            }
                            else {
                                std::cout << "Failed to create the shader resource view" << std::endl;
                            }
                        }
                        else {
                            std::cout << "Failed to load memory from the bytes , perhaps they are empty ?" << std::endl;
                        }
                    }
                    else {
                        std::cout << "Bytes are empty , this is a fatal error" << std::endl;
                    }
                }
                else {
                    std::cout << "ImageToBytes failed" << std::endl;
                }
            }
            else {
                std::cout << "Loading image from system failed , please check your path" << std::endl;
            }
        }
        else {
            std::cout << "Image doesnt exist , make sure the path is correct" << std::endl;
        }

    }

    /// <summary>
    /// Get the texture id of the image
    /// </summary>
    /// <returns>ImTextureID of the image</returns>
    ImTextureID GetTextureID()
    {
        return this->m_ImageID;
    }

    /// <summary>
    /// Get the size of the image
    /// </summary>
    /// <returns>
    /// The size of the image
    /// </returns>
    ImVec2 GetSize()
    {
        return ImVec2(image_info.width, image_info.height);
    }


    /// <summary>
    /// Resize the image on the fly. 
    /// <remarks>
    /// NOTE : This doesnt modify the original image file, all changes happens in the memory and during run time only
    /// </remarks>
    /// </summary>
    /// <param name="newHeight">Desired Height</param>
    /// <param name="newWidth">Desired Width</param>
    /// <returns></returns>
    bool Resize(float newHeight, float newWidth)
    {
        // make sure we're loaded
        if (!bytes->empty() && m_ImageID != NULL)
        {
            if (newHeight == image_info.height && newWidth == image_info.width)
            {
                std::cout << "[WARNING] | " << __FUNCTION__ << " | image wasnt resized because it has the same dimensions" << std::endl;
                return true; // no need to resize if same size
            }
            HRESULT hr;
            DirectX::ScratchImage tempImage;
            ID3D11Device* g_pd3dDevice = RenderManager::GetInstance()->GetDevice();
            if (BytesToImage(*bytes, tempImage))
            {
                DirectX::ScratchImage newImage;
                DirectX::TexMetadata newMetadata;
                hr = DirectX::Resize(
                    tempImage.GetImages(),
                    tempImage.GetImageCount(),
                    tempImage.GetMetadata(),
                    newWidth,
                    newHeight,
                    DirectX::TEX_FILTER_DEFAULT,
                    newImage
                );
                if (SUCCEEDED(hr))
                {
                    DirectX::Blob data;
                    hr = DirectX::SaveToWICMemory(
                        newImage.GetImages(),
                        newImage.GetImageCount(),
                        DirectX::WIC_FLAGS_NONE,
                        GUID_ContainerFormatPng,
                        data
                    );
                    if (SUCCEEDED(hr))
                    {
                        std::vector<uint8_t> tempBytes;
                        if (BlobToBytes(data, tempBytes))
                        {
                            bytes = std::make_unique<std::vector<uint8_t>>(tempBytes);

                            image_info.height = newImage.GetMetadata().height;
                            image_info.width = newImage.GetMetadata().width;
                            image_info.imageSize = newImage.GetMetadata().arraySize;

                            std::cout << "Bytes set successfully" << std::endl;
                            ID3D11ShaderResourceView* srv;
                            hr = DirectX::CreateShaderResourceView(g_pd3dDevice, newImage.GetImages(), newImage.GetImageCount(), newImage.GetMetadata(), &srv);
                            if (SUCCEEDED(hr))
                            {
                                this->m_ImageID = (ImTextureID)srv;
                                std::cout << "Finished" << std::endl;
                                return true;
                            }
                            else {
                                std::cout << "Failed to create the shader resource view" << std::endl;
                                return false;
                            }
                        }
                        else {
                            std::cout << "Failed to convert Blob to bytes" << std::endl;
                            return false;
                        }


                    }
                    else {
                        std::cout << "[ERROR] | " << __FUNCTION__ << " | Failed to write bytes into our own image" << std::endl;
                        return false;
                    }
                }
                else {
                    std::cout << "[ERROR] | " << __FUNCTION__ << " | Failed to resize the image " << std::endl;
                    return false;
                }
            }
            else {
                std::cout << "[ERROR] | " << __FUNCTION__ << " | Failed to load image from bytes" << std::endl;
                return false;
            }
        }

        else {
            std::cout << "[ERROR] You're trying to resize an uninitialized image (maybe try using .Reset()) " << std::endl;
            return false;
        }
    }

    /// <summary>
    /// Reset the image to it's original state
    /// </summary>
    void Reset()
    {
        //new object (= original cuz we only dealin with bytes so no changes to original file
        ImGuiImage tempImage(original_path);

        //swap members , cuz you cant assign the current objects
        Swap(tempImage);
    }

    /// <summary>
    /// Draws the image
    /// </summary>
    void Draw()
    {
        // perform checks cuz u shouldnt be drawin an image with no data lol
        if (!bytes->empty() && m_ImageID != NULL)
        {
            if (this->rotation == 0) // skip
            {
                ImGui::Image(GetTextureID(), GetSize());
            }
            else {
                Rotation::ImRotateStart();
                ImGui::Image(GetTextureID(), GetSize());
                Rotation::ImRotateEnd(DegreesToRadians(this->rotation));

            }
        }
    }

    /// <summary>
    /// A simple function to get/set the rotation of the image
    /// </summary>
    /// <returns>A reference to the rotation value</returns>
    float& Rotation()
    {
        return this->rotation;
    }

private:
    /// <summary>
    /// <value>ImGui Texture ID of the image</value>
    /// </summary>
    ImTextureID m_ImageID;
    std::unique_ptr<std::vector<uint8_t>> bytes; // image bytes

    const wchar_t* original_path; // keep this in case , i might write a reset function that will revert it to it's old state maybe
    // perhaps i should consider storing everything as new bytes but i'm not sure yet
    // or maybe not saving edits but idk yet
    struct {
        float width;
        float height;
        size_t imageSize;
    } image_info;

    float rotation; // newly added after ms devs ghosted me in DirectXTex github page



    // internal functions to make my life easier
private:

    float DegreesToRadians(float degrees) {
        return degrees * (DirectX::XM_PI / 180);
    }

    void Swap(ImGuiImage& other)
    {
        std::swap(m_ImageID, other.m_ImageID);
        std::swap(bytes, other.bytes);
        std::swap(image_info, other.image_info);
    }

    bool BlobToBytes(DirectX::Blob& blob, std::vector<uint8_t>& bytes) const
    {
        if (blob.GetBufferPointer() != nullptr || blob.GetBufferSize() > 0)
        {
            const uint8_t* dataPtr = static_cast<const uint8_t*>(blob.GetBufferPointer());
            size_t dataSize = blob.GetBufferSize();
            std::vector<uint8_t> imageBytes(dataPtr, dataPtr + dataSize);
            bytes = std::move(imageBytes);
            return true;
        }
        return false;


    }

    bool ImageToBytes(std::vector<uint8_t>& bytelist, DirectX::ScratchImage& image) const
    {
        HRESULT hr;
        DirectX::Blob data;
        hr = DirectX::SaveToWICMemory(image.GetImages(), image.GetImageCount(), DirectX::WIC_FLAGS_NONE, GUID_ContainerFormatPng, data);
        if (SUCCEEDED(hr))
        {
            if (BlobToBytes(data, bytelist))
            {
                return true;
            }
            else {
                std::cout << "Blob was empty" << std::endl;
                return false;
            }
        }
        else {
            std::cout << "Failed to parse image bytes" << std::endl;
            return false;
        }
    }

    bool BytesToImage(std::vector<uint8_t> bytes, DirectX::ScratchImage& image) const
    {
        if (!bytes.empty())
        {
            HRESULT hr;
            DirectX::ScratchImage tempImage;
            DirectX::TexMetadata tempMeta;
            hr = DirectX::LoadFromWICMemory(
                bytes.data(),
                bytes.size(),
                DirectX::WIC_FLAGS_NONE,
                &tempMeta,
                tempImage
            );
            if (SUCCEEDED(hr))
            {
                image = std::move(tempImage);
                return true;
            }
            else {
                std::cout << "Failed to parse image bytes" << std::endl;
                return false;
            }
        }
        else {
            std::cout << "[ERROR] " << __FUNCTION__ << " Can't get an image from empty bytes" << std::endl;
            return false;
        }

    }

    bool LoadFromFile(const wchar_t* path, DirectX::ScratchImage& image)
    {
        if (fs::exists(path))
        {
            HRESULT hr;
            DirectX::ScratchImage tempImage; // the temp image to store from the file
            DirectX::TexMetadata tempMetadata;

            // loading the image from the file
            hr = DirectX::LoadFromWICFile(path, DirectX::WIC_FLAGS_NONE, &tempMetadata, tempImage);
            if (SUCCEEDED(hr))
            {
                image = std::move(tempImage);
                return true;
            }
            else {
                std::cout << "Loading image from system failed , please check your path" << std::endl;
                return false;
            }
        }
        else {
            std::cout << "Image doesnt exist , make sure the path is correct" << std::endl;
            return false;
        }
    }
};

#endif // !TESTCLASS_H

