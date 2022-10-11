#pragma once
#include <windows.h>
#include <string>
#include <atlimage.h>
class CEdoyunTool
{
public:
	static void Dump(BYTE* pData, size_t nSize) {
        std::string strOut;
        for (size_t i = 0; i < nSize; i++)
        {
            char buf[8] = "";
            if (i > 0 && (i % 16 == 0))strOut += "\n";
            snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
            strOut += buf;
        }
        strOut += "\n";
        OutputDebugStringA(strOut.c_str());
    }
    static int Bytes2Image(CImage& image, const std::string strBuffer) {
		BYTE* pData = (BYTE*)strBuffer.c_str();
		//TODO:存入CImage
		HGLOBAL hMen = GlobalAlloc(GMEM_MOVEABLE, 0);//?创建一个描述符
		if (hMen == NULL) {
			TRACE("内存不足了!!\r\n");
			Sleep(1);
			return -1;
		}
		IStream* pStream = NULL;
		HRESULT hRet = CreateStreamOnHGlobal(hMen, TRUE, &pStream);
		if (hRet == S_OK) {//创建内存成功
			ULONG length = 0;
			pStream->Write(pData, strBuffer.size(), &length);
			LARGE_INTEGER bg = { 0 };
			pStream->Seek(bg, STREAM_SEEK_SET, NULL);//重新指定流指针指向
			if ((HBITMAP)image != NULL)
				image.Destroy();
			hRet = image.Load(pStream);
		}
		return hRet;
    }
};

