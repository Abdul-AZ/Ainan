#include "D3D11RendererContext.h"

#include <locale>
#include <codecvt>

namespace Ainan {
	namespace D3D11 {
		RendererType D3D11RendererContext::GetType() const
		{
			return RendererType::D3D11;
		}

		std::string D3D11RendererContext::GetVersionString()
		{
			D3D_FEATURE_LEVEL selectedLevel = Device->GetFeatureLevel();
			if (selectedLevel == D3D_FEATURE_LEVEL_11_1)
				return "11.1";
			else if (selectedLevel == D3D_FEATURE_LEVEL_11_0)
				return "11.0";

			return "";
		}

		std::string D3D11RendererContext::GetPhysicalDeviceName()
		{
			IDXGIFactory* pFactory;
			IDXGIAdapter* adapter;
			HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));
			pFactory->EnumAdapters(0, &adapter);
			DXGI_ADAPTER_DESC desc;
			adapter->GetDesc(&desc);

			//setup converter
			using convert_type = std::codecvt_utf8<wchar_t>;
			std::wstring_convert<convert_type, wchar_t> converter;

			//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
			return converter.to_bytes(desc.Description);
		}
	}
}
