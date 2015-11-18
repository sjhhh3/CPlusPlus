// Video2Image.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Mfreadwrite.h>
#include <Mfobjects.h>
#include <Mfapi.h>
#include <mferror.h>  
#include <Wmcodecdsp.h>
#include <Dshow.h>
#include <Mfidl.h>
#include "SourceReaderCB.h"
#include <new>
#include <atlbase.h>

template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

HRESULT CreateSourceReaderAsync(
	PCWSTR pszURL,
	IMFSourceReaderCallback *pCallback,
	IMFSourceReader **ppReader)
{
	HRESULT hr = S_OK;
	IMFAttributes *pAttributes = NULL;

	hr = MFCreateAttributes(&pAttributes, 1);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, pCallback);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = MFCreateSourceReaderFromURL(pszURL, pAttributes, ppReader);

done:
	SafeRelease(&pAttributes);
	return hr;
}

HRESULT ConfigureDecoder(IMFSourceReader *pReader, DWORD dwStreamIndex)
{
	IMFMediaType *pNativeType = NULL;
	IMFMediaType *pType = NULL;

	// Find the native format of the stream.
	HRESULT hr = pReader->GetNativeMediaType(dwStreamIndex, 0, &pNativeType);
	if (FAILED(hr))
	{
		return hr;
	}

	GUID majorType, subtype;

	// Find the major type.
	hr = pNativeType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
	if (FAILED(hr))
	{
		goto done;
	}

	// Define the output type.
	hr = MFCreateMediaType(&pType);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pType->SetGUID(MF_MT_MAJOR_TYPE, majorType);
	if (FAILED(hr))
	{
		goto done;
	}

	// Select a subtype.
	if (majorType == MFMediaType_Video)
	{
		subtype = MFVideoFormat_RGB32;
	}
	else if (majorType == MFMediaType_Audio)
	{
		subtype = MFAudioFormat_PCM;
	}
	else
	{
		// Unrecognized type. Skip.
		goto done;
	}

	hr = pType->SetGUID(MF_MT_SUBTYPE, subtype);
	if (FAILED(hr))
	{
		goto done;
	}

	// Set the uncompressed format.
	hr = pReader->SetCurrentMediaType(dwStreamIndex, NULL, pType);
	if (FAILED(hr))
	{
		goto done;
	}

done:
	SafeRelease(&pNativeType);
	SafeRelease(&pType);
	return hr;
}


HRESULT ReadMediaFile(PCWSTR pszURL)
{
	HRESULT hr = S_OK;

	IMFSourceReader *pReader = NULL;
	SourceReaderCB *pCallback = NULL;

	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hEvent == NULL)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto done;
	}

	// Create an instance of the callback object.
	pCallback = new (std::nothrow) SourceReaderCB(hEvent);
	if (pCallback == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto done;
	}

	// Create the Source Reader.
	hr = CreateSourceReaderAsync(pszURL, pCallback, &pReader);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = ConfigureDecoder(pReader, MF_SOURCE_READER_FIRST_VIDEO_STREAM);
	if (FAILED(hr))
	{
		goto done;
	}

	// Request the first sample.
	hr = pReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM,
		0, NULL, NULL, NULL, NULL);
	if (FAILED(hr))
	{
		goto done;
	}

	while (SUCCEEDED(hr))
	{
		BOOL bEOS;
		hr = pCallback->Wait(INFINITE, &bEOS);
		if (FAILED(hr) || bEOS)
		{
			break;
		}
		hr = pReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM,
			0, NULL, NULL, NULL, NULL);
	}

done:
	SafeRelease(&pReader);
	SafeRelease(&pCallback);
	return hr;
}

#define WIDTH_BYTES(bits) (((bits) + 31) / 32 * 4)
BOOL SaveBMPFile(TCHAR *lpFileName, BITMAPINFO *pBmpInfo, BYTE *pImageBuffer)
{
	BOOL bRVal = TRUE;
	DWORD dwBytesRead = 0;
	DWORD dwSize = 0;
	BITMAPFILEHEADER bfh = { 0 };
	int nTable = 0;
	DWORD dwImageSize = 0;

	if (pBmpInfo->bmiHeader.biBitCount > 8) {
		nTable = 0;
	}
	else{
		nTable = 256;
	}

	dwImageSize = WIDTH_BYTES(pBmpInfo->bmiHeader.biWidth * pBmpInfo->bmiHeader.biBitCount) *
		pBmpInfo->bmiHeader.biHeight;

	if (dwImageSize <= 0) {
		bRVal = FALSE;
	}
	else{
		bfh.bfType = (WORD)'M' << 8 | 'B';
		bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nTable * sizeof(RGBQUAD);
		bfh.bfSize = bfh.bfOffBits + dwImageSize;

		HANDLE hFile = ::CreateFile(lpFileName,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);
		if (hFile == INVALID_HANDLE_VALUE) {
			bRVal = FALSE;
		}
		else{
			dwSize = sizeof(BITMAPFILEHEADER);
			::WriteFile(hFile, &bfh, dwSize, &dwBytesRead, NULL);

			dwSize = sizeof(BITMAPINFOHEADER) + nTable * sizeof(RGBQUAD);
			::WriteFile(hFile, pBmpInfo, dwSize, &dwBytesRead, NULL);

			dwSize = dwImageSize;
			WriteFile(hFile, pImageBuffer, dwSize, &dwBytesRead, NULL);

			CloseHandle(hFile);
		}
	}

	return bRVal;
}

void SaveDataToGrayImage(char *t_buf, int width, int height, TCHAR *m_str, int dpi)
{
	LPBITMAPINFO m_lpbminfo = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD))];
	m_lpbminfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_lpbminfo->bmiHeader.biWidth = width;
	m_lpbminfo->bmiHeader.biHeight = height;
	m_lpbminfo->bmiHeader.biPlanes = 1;
	m_lpbminfo->bmiHeader.biBitCount = 8;
	m_lpbminfo->bmiHeader.biCompression = BI_RGB;
	m_lpbminfo->bmiHeader.biSizeImage = 0;
	m_lpbminfo->bmiHeader.biXPelsPerMeter = int(dpi / 0.0254 + 0.5);
	m_lpbminfo->bmiHeader.biYPelsPerMeter = int(dpi / 0.0254 + 0.5);
	m_lpbminfo->bmiHeader.biClrUsed = 256;
	m_lpbminfo->bmiHeader.biClrImportant = 256;

	for (int i = 0; i < 256; i++)
	{
		m_lpbminfo->bmiColors[i].rgbRed = (BYTE)i;
		m_lpbminfo->bmiColors[i].rgbGreen = (BYTE)i;
		m_lpbminfo->bmiColors[i].rgbBlue = (BYTE)i;
		m_lpbminfo->bmiColors[i].rgbReserved = (BYTE)0;
	}

	SaveBMPFile(m_str, m_lpbminfo, (unsigned char *)t_buf);

	delete[](BYTE *)m_lpbminfo;
	return;
}

void SaveDataToColorImage(char *t_buf, int width, int height, TCHAR *m_str, int dpi)
{
	int nTable = 0;
	LPBITMAPINFO lpbminfo_common_24 = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER) + (nTable * sizeof(RGBQUAD))];
	lpbminfo_common_24->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpbminfo_common_24->bmiHeader.biWidth = width;
	lpbminfo_common_24->bmiHeader.biHeight = height;
	lpbminfo_common_24->bmiHeader.biPlanes = 1;
	lpbminfo_common_24->bmiHeader.biBitCount = 24;
	lpbminfo_common_24->bmiHeader.biCompression = BI_RGB;
	lpbminfo_common_24->bmiHeader.biSizeImage = 0;
	lpbminfo_common_24->bmiHeader.biXPelsPerMeter = int(dpi / 0.0254 + 0.5);
	lpbminfo_common_24->bmiHeader.biYPelsPerMeter = int(dpi / 0.0254 + 0.5);
	lpbminfo_common_24->bmiHeader.biClrUsed = 0;
	lpbminfo_common_24->bmiHeader.biClrImportant = 0;

	SaveBMPFile(m_str, lpbminfo_common_24, (unsigned char *)t_buf);

	delete[](BYTE *)lpbminfo_common_24;
	return;
}

void SaveRGB32DataToImage(BYTE *outData, int Width, int Height, TCHAR* output_filename, BOOL flag_gray)
{
	int ii, jj, kk, Length = Width * Height;
	

	if (flag_gray)
	{
		int R, G, B, I;
		char *outData_Gray = new char[Length];
		for (jj = 0; jj < Height; jj++)
		{
			for (ii = 0; ii < Width; ii++)
			{
				B = (int)(outData[4 * (jj*Width + ii) + 0]);
				G = (int)(outData[4 * (jj*Width + ii) + 1]);
				R = (int)(outData[4 * (jj*Width + ii) + 2]);
				I = int(0.3*R + 0.59*G + 0.11*B + 0.5);
				outData_Gray[(Height - 1 - jj)*Width + ii] = (char)I;
			}
		}		
		SaveDataToGrayImage(outData_Gray, Width, Height, output_filename, 0);
		delete[]outData_Gray;
	}
	else
	{
		char *outData_RGB = new char[Length * 3];
		for (jj = 0; jj < Height; jj++)
			for (ii = 0; ii < Width; ii++)
				for (kk = 0; kk < 3; kk++)
					outData_RGB[3 * ((Height - 1 - jj)*Width + ii) + kk] = outData[4 * (jj*Width + ii) + kk];

		SaveDataToColorImage(outData_RGB, Width, Height, output_filename, 0);
		delete[]outData_RGB;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	const WCHAR *pszURL = argv[1];
	WCHAR outputFilename[200];
	BOOL flag_gray = FALSE;

	if (argc < 3)
	{
		MessageBox(NULL, L"More arguments are needed. Usage: Video2Image [Video File] [Bmp File] [0/1] \r\nExample:\r\nVideo2Image C:\\Temp\\Video\\A.mp4 C:\\Temp\\Image\\B.bmp 1", L"Warning", MB_ICONERROR | MB_OK);
		return -1;
	}
	else
	{
		wcsncpy_s(outputFilename, argv[2], wcslen(argv[2]) - 4);
	}

	if (argc >=4 )
		flag_gray = BOOL(_ttoi(argv[3]));

	/*
	MessageBox(NULL, argv[0], L"Message", MB_ICONINFORMATION | MB_OK);
	MessageBox(NULL, pszURL, L"Message", MB_ICONINFORMATION | MB_OK);
	MessageBox(NULL, outputFilename, L"Message", MB_ICONINFORMATION | MB_OK);
	MessageBox(NULL, flag_gray?_T("TRUE"):_T("FALSE"), L"Message", MB_ICONINFORMATION | MB_OK);
	*/

	// Initialize the COM runtime.
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (SUCCEEDED(hr))
	{
		// Initialize the Media Foundation platform.
		hr = MFStartup(MF_VERSION);
		if (SUCCEEDED(hr))
		{
			// Create the source reader.
			IMFSourceReader *SourceReader;
			hr = MFCreateSourceReaderFromURL(pszURL, NULL, &SourceReader);
			if (SUCCEEDED(hr))
			{
				MessageBox(NULL, L"Video file has been loaded successfully. Now ready to extract each frame.", L"Message", MB_ICONINFORMATION | MB_OK);

				UINT32 Width, Height;
				for (DWORD MediaTypeIndex = 0;; MediaTypeIndex++)
				{
					IMFMediaType* pNativeType = NULL;
					IMFMediaType* pInputType = NULL;

					hr = SourceReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, MediaTypeIndex, &pNativeType);
					if (!SUCCEEDED(hr))
					{
						MessageBox(NULL, L"GetNativeMediaType Error", L"Message", MB_ICONINFORMATION | MB_OK);
						break;
					}

					MFGetAttributeSize(pNativeType, MF_MT_FRAME_SIZE, &Width, &Height);
					GUID majorType;
					pNativeType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
					if (majorType == MFMediaType_Video)
					{
						GUID NativeSubtype;
						pNativeType->GetGUID(MF_MT_SUBTYPE, &NativeSubtype);

						MFCreateMediaType(&pInputType);
						pInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
						pInputType->SetGUID(MF_MT_SUBTYPE, MEDIASUBTYPE_NV12);
						hr = SourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, pInputType);
						if(!SUCCEEDED(hr))
							MessageBox(NULL, L"SetCurrentMediaType Failed", L"Message", MB_ICONINFORMATION | MB_OK);
						hr = MFSetAttributeSize(pInputType, MF_MT_FRAME_SIZE, Width, Height);
						if (!SUCCEEDED(hr))
							MessageBox(NULL, L"MFSetAttributeSize Failed", L"Message", MB_ICONINFORMATION | MB_OK);
						break;
					}
				}

				CComPtr<IMFTransform > pMFTransform;
				hr = pMFTransform.CoCreateInstance(CLSID_CColorConvertDMO);
				if (!SUCCEEDED(hr))
					MessageBox(NULL, L"CoCreateInstance Failed", L"Message", MB_ICONINFORMATION | MB_OK);
				CComPtr <IMFMediaType>pTransformInputType;
				MFCreateMediaType(&pTransformInputType);
				pTransformInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
				pTransformInputType->SetGUID(MF_MT_SUBTYPE, MEDIASUBTYPE_NV12);
				hr = MFSetAttributeSize(pTransformInputType, MF_MT_FRAME_SIZE, Width, Height);
				if (!SUCCEEDED(hr))
					MessageBox(NULL, L"MFSetAttributeSize Failed", L"Message", MB_ICONINFORMATION | MB_OK);
				hr = pMFTransform->SetInputType(0, pTransformInputType, 0);
				if (!SUCCEEDED(hr))
					MessageBox(NULL, L"SetInputType Failed", L"Message", MB_ICONINFORMATION | MB_OK);

				CComPtr <IMFMediaType >pTransformOutputType;
				MFCreateMediaType(&pTransformOutputType);
				pTransformOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
				pTransformOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
				hr = MFSetAttributeSize(pTransformOutputType, MF_MT_FRAME_SIZE, Width, Height);
				if (!SUCCEEDED(hr))
					MessageBox(NULL, L"MFSetAttributeSize Failed", L"Message", MB_ICONINFORMATION | MB_OK);
				hr = pMFTransform->SetOutputType(0, pTransformOutputType, 0);
				if (!SUCCEEDED(hr))
					MessageBox(NULL, L"SetOutputType Failed", L"Message", MB_ICONINFORMATION | MB_OK);

				SourceReader->SetStreamSelection(MF_SOURCE_READER_ANY_STREAM, FALSE);
				SourceReader->SetStreamSelection(MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);
				DWORD StreamIndex, Flags;
				LONGLONG TimeStamp;

				CComPtr<IMFSample> Sample;
				CComPtr<IMFMediaBuffer>MediaBuffer;
				IMFMediaBuffer *pBuffer = NULL;
				IMFSample      *pOutputSample;
				IMFMediaBuffer *pOutputBuffer;
				MFCreateSample(&pOutputSample);
				MFCreateMemoryBuffer(Width * Height * 4, &pOutputBuffer);
				pOutputSample->AddBuffer(pOutputBuffer);
				MFT_OUTPUT_DATA_BUFFER RGBOutputDataBuffer;
				RGBOutputDataBuffer.dwStreamID = 0;
				RGBOutputDataBuffer.dwStatus = 0;
				RGBOutputDataBuffer.pEvents = NULL;
				RGBOutputDataBuffer.pSample = pOutputSample;
				DWORD ProcessStatus;
				DWORD BufferCount;
				BYTE *outData;
				DWORD outDataLen = 0;
				TCHAR fn[200];
				int kkk = 0;
				while (true)
				{
					kkk++;
					SourceReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &StreamIndex, &Flags, &TimeStamp, &Sample);
					if (Flags & MF_SOURCE_READERF_ENDOFSTREAM)
						break;

					hr = pMFTransform->ProcessInput(0, Sample, 0);
					hr = pMFTransform->ProcessOutput(0, 1, &RGBOutputDataBuffer, &ProcessStatus);

					RGBOutputDataBuffer.pSample->GetBufferCount(&BufferCount);
					pOutputBuffer->Lock(&outData, NULL, &outDataLen);

					StringCbPrintf(fn, 200, TEXT("%ls_%d_%d_%d_%05d.bmp"), outputFilename, BufferCount, Width, Height, kkk);
					SaveRGB32DataToImage(outData, Width, Height, fn, flag_gray);

					pOutputBuffer->Unlock();
				}
				SourceReader->Release();
			}
			else
				MessageBox(NULL, L"MFCreateSourceReaderFromURL Failed", L"Warning", MB_ICONERROR | MB_OK);

			// Shut down Media Foundation.
			MFShutdown();
		}
		CoUninitialize();
	}

	MessageBox(NULL, L"Job completed!", L"Great!", MB_ICONINFORMATION | MB_OK);
	return 0;
}

