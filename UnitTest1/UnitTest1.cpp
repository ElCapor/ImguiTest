#include "pch.h"
#include "CppUnitTest.h"
#include "../ImguiTest/TextureManager.h"
#include <iostream>
#include <comdef.h>
// Writing unit tests cuz why not ?
// It's more professional and i love to see green checkmarks everywhere

using namespace Microsoft::VisualStudio::CppUnitTestFramework;



namespace UnitTest1
{
	TEST_CLASS(TextureManagerTest)
	{
	public:
		
		TEST_METHOD(GetInstanceTest)
		{
			// test if GetInstance works
			Logger::WriteMessage(TextureManager::GetInstance()->ToString().str().c_str());
			Assert::IsNotNull(TextureManager::GetInstance().get(), L"Texture Manager is null !");
		}

		TEST_METHOD(LoadImageFromFileTest)
		{
			//test loadicon
			Assert::IsNotNull(TextureManager::GetInstance().get(), L"Texture manager is null !");
			DX11Image* image = new DX11Image();
			HRESULT hr = TextureManager::GetInstance().get()->LoadImageFromFile(L"icon.png", "sunglass");
			Assert::AreEqual(S_OK, hr, L"LoadFromFile error");
			Assert::IsFalse(TextureManager::GetInstance().get()->GetImage("sunglass").get() == nullptr, L"Image is null !");
		}
	};
}
