/*
TextureManager , a simple class to handle textures with imgui & dx11
written by mogus from scratch
*/
#include <DirectXTex.h>
#include <map>
#include <string>
#include <sstream>
#include <comdef.h>
#include <iostream>
#include "Utils.h"
class DX11Image;
class TextureManager {
public:

	static std::unique_ptr<TextureManager>& GetInstance();

	std::stringstream ToString() const;

	void SetDevice(ID3D11Device* newDevice);

	ID3D11Device* GetDevice();

	HRESULT LoadImageFromFile(const wchar_t* filePath, std::string name, DX11Image* image = nullptr);

	std::shared_ptr<DX11Image> GetImage(const std::string& name);

	TextureManager()
	{
		m_Device = nullptr;
	}



private:

	static std::unique_ptr<TextureManager> instance;
	std::map<std::string, std::shared_ptr<DX11Image>> m_ImageMap;
	ID3D11Device* m_Device;


};

class DX11Image
{
public:
	DX11Image() : m_Image(nullptr)
	{

	}

	DX11Image(DirectX::ScratchImage& scratchImage)
	{
		m_Image = &scratchImage;
	}

	std::string ToString() const
	{
		std::stringstream stream;
		if (m_Image)
		{
			
			stream << "[DX11IMAGE] : " << m_Image->GetImageCount() << " " << m_Image->GetMetadata().format << " " << m_Image->GetMetadata().width << std::endl;
		}
		else
		{
			stream << "[DX11IMAGE] : Null Image" << std::endl;
		}
		return stream.str();
	}

	DirectX::TexMetadata GetMetadata()
	{
		return m_Image->GetMetadata();
	}

	ID3D11ShaderResourceView* asSrv()
	{
		ID3D11ShaderResourceView* srv = nullptr;
		HRESULT hr = DirectX::CreateShaderResourceView(TextureManager::GetInstance()->GetDevice(), m_Image->GetImages(), m_Image->GetImageCount(), m_Image->GetMetadata(), &srv);
		if (FAILED(hr))
		{
			printf("Error in asSrv()\n");
		}
		return srv;
	}

private:
	DirectX::ScratchImage* m_Image;
};



