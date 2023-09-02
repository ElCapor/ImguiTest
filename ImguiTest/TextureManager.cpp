#include "TextureManager.h"

std::unique_ptr<TextureManager> TextureManager::instance = nullptr;

std::unique_ptr<TextureManager>& TextureManager::GetInstance()
{
	if (instance == nullptr)
	{
		instance = std::make_unique<TextureManager>();
		
	}
	return instance;
}

std::stringstream TextureManager::ToString() const {
	// Include relevant information about the object's state
	std::stringstream ss;
	ss << "TextureManager: [m_ImageMap Size =" << m_ImageMap.size() << "]" << std::endl;
	return ss;
}

void TextureManager::SetDevice(ID3D11Device* newDevice)
{
	this->m_Device = newDevice;
}

HRESULT TextureManager::LoadImageFromFile(const wchar_t* filePath, std::string name, DX11Image* image)
{
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage scratchImage;

	HRESULT hr = DirectX::LoadFromWICFile(filePath, DirectX::WIC_FLAGS_NONE, &metadata, scratchImage);
	if (SUCCEEDED(hr))
	{
		
		std::shared_ptr<DX11Image> result = std::make_shared<DX11Image>(scratchImage);
		this->m_ImageMap[name] = result;

		if (image != nullptr)
		{
			*image = *result.get();
		}

		return S_OK;
	}
	else {
		return E_FAIL;
	}
}


std::shared_ptr<DX11Image> TextureManager::GetImage(const std::string& name)
{
	auto it = this->m_ImageMap.find(name);
	if (it != this->m_ImageMap.end())
	{
		return it->second; // Return the shared_ptr if found
	}
	else
	{
		return nullptr; // Return nullptr if not found
	}
}


ID3D11Device* TextureManager::GetDevice()
{
	return this->m_Device;
}