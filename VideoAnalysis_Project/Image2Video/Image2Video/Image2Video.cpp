// Image2Video.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <Mfreadwrite.h>
#include <mferror.h>

#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")

template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

// Format data
UINT32 VIDEO_WIDTH;
UINT32 VIDEO_HEIGHT;
UINT32 VIDEO_FPS = 30;
UINT64 VIDEO_FRAME_DURATION = 10 * 1000 * 1000 / VIDEO_FPS;
UINT32 VIDEO_BIT_RATE = 800000;
GUID   VIDEO_ENCODING_FORMAT = MFVideoFormat_H264;
GUID   VIDEO_INPUT_FORMAT = MFVideoFormat_RGB32;
UINT32 VIDEO_PELS = VIDEO_WIDTH * VIDEO_HEIGHT;
UINT32 VIDEO_FRAME_COUNT = 20 * VIDEO_FPS;

// Buffer to hold the video frame data.
DWORD *videoFrameBuffer;

HRESULT InitializeSinkWriter(IMFSinkWriter **ppWriter, DWORD *pStreamIndex, WCHAR *outputFilename)
{
	*ppWriter = NULL;
	*pStreamIndex = NULL;

	IMFSinkWriter   *pSinkWriter = NULL;
	IMFMediaType    *pMediaTypeOut = NULL;
	IMFMediaType    *pMediaTypeIn = NULL;
	DWORD           streamIndex;

	HRESULT hr = MFCreateSinkWriterFromURL(outputFilename, NULL, NULL, &pSinkWriter);

	// Set the output media type.
	if (SUCCEEDED(hr))
	{
		hr = MFCreateMediaType(&pMediaTypeOut);
	}
	if (SUCCEEDED(hr))
	{
		hr = pMediaTypeOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	}
	if (SUCCEEDED(hr))
	{
		hr = pMediaTypeOut->SetGUID(MF_MT_SUBTYPE, VIDEO_ENCODING_FORMAT);
	}
	if (SUCCEEDED(hr))
	{
		hr = pMediaTypeOut->SetUINT32(MF_MT_AVG_BITRATE, VIDEO_BIT_RATE);
	}
	if (SUCCEEDED(hr))
	{
		hr = pMediaTypeOut->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeSize(pMediaTypeOut, MF_MT_FRAME_SIZE, VIDEO_WIDTH, VIDEO_HEIGHT);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeRatio(pMediaTypeOut, MF_MT_FRAME_RATE, VIDEO_FPS, 1);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeRatio(pMediaTypeOut, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
	}
	if (SUCCEEDED(hr))
	{
		hr = pSinkWriter->AddStream(pMediaTypeOut, &streamIndex);
	}

	// Set the input media type.
	if (SUCCEEDED(hr))
	{
		hr = MFCreateMediaType(&pMediaTypeIn);
	}
	if (SUCCEEDED(hr))
	{
		hr = pMediaTypeIn->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	}
	if (SUCCEEDED(hr))
	{
		hr = pMediaTypeIn->SetGUID(MF_MT_SUBTYPE, VIDEO_INPUT_FORMAT);
	}
	if (SUCCEEDED(hr))
	{
		hr = pMediaTypeIn->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeSize(pMediaTypeIn, MF_MT_FRAME_SIZE, VIDEO_WIDTH, VIDEO_HEIGHT);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeRatio(pMediaTypeIn, MF_MT_FRAME_RATE, VIDEO_FPS, 1);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeRatio(pMediaTypeIn, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
	}
	if (SUCCEEDED(hr))
	{
		hr = pSinkWriter->SetInputMediaType(streamIndex, pMediaTypeIn, NULL);
	}

	// Tell the sink writer to start accepting data.
	if (SUCCEEDED(hr))
	{
		hr = pSinkWriter->BeginWriting();
	}

	// Return the pointer to the caller.
	if (SUCCEEDED(hr))
	{
		*ppWriter = pSinkWriter;
		(*ppWriter)->AddRef();
		*pStreamIndex = streamIndex;
	}

	SafeRelease(&pSinkWriter);
	SafeRelease(&pMediaTypeOut);
	SafeRelease(&pMediaTypeIn);
	return hr;
}

HRESULT WriteFrame(
	IMFSinkWriter *pWriter,
	DWORD streamIndex,
	const LONGLONG& rtStart        // Time stamp.
	)
{
	IMFSample *pSample = NULL;
	IMFMediaBuffer *pBuffer = NULL;

	const LONG cbWidth = 4 * VIDEO_WIDTH;
	const DWORD cbBuffer = cbWidth * VIDEO_HEIGHT;

	BYTE *pData = NULL;

	// Create a new memory buffer.
	HRESULT hr = MFCreateMemoryBuffer(cbBuffer, &pBuffer);

	// Lock the buffer and copy the video frame to the buffer.
	if (SUCCEEDED(hr))
	{
		hr = pBuffer->Lock(&pData, NULL, NULL);
	}
	if (SUCCEEDED(hr))
	{
		hr = MFCopyImage(
			pData,                      // Destination buffer.
			cbWidth,                    // Destination stride.
			(BYTE*)videoFrameBuffer,    // First row in source image.
			cbWidth,                    // Source stride.
			cbWidth,                    // Image width in bytes.
			VIDEO_HEIGHT                // Image height in pixels.
			);
	}
	if (pBuffer)
	{
		pBuffer->Unlock();
	}

	// Set the data length of the buffer.
	if (SUCCEEDED(hr))
	{
		hr = pBuffer->SetCurrentLength(cbBuffer);
	}

	// Create a media sample and add the buffer to the sample.
	if (SUCCEEDED(hr))
	{
		hr = MFCreateSample(&pSample);
	}
	if (SUCCEEDED(hr))
	{
		hr = pSample->AddBuffer(pBuffer);
	}

	// Set the time stamp and the duration.
	if (SUCCEEDED(hr))
	{
		hr = pSample->SetSampleTime(rtStart);
	}
	if (SUCCEEDED(hr))
	{
		hr = pSample->SetSampleDuration(VIDEO_FRAME_DURATION);
	}

	// Send the sample to the Sink Writer.
	if (SUCCEEDED(hr))
	{
		hr = pWriter->WriteSample(streamIndex, pSample);
	}

	SafeRelease(&pSample);
	SafeRelease(&pBuffer);
	return hr;
}

BOOL GetWidthHeightInfoOfBmpFile(LPTSTR szFileName, UINT32 &width, UINT32 &height)
{
	HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, szFileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (hBitmap == NULL)
		return FALSE;

	HDC hdc, hdcMem;
	hdc = GetDC(NULL);
	hdcMem = CreateCompatibleDC(hdc);

	BITMAPINFO MyBMInfo;
	memset(&MyBMInfo, 0, sizeof(MyBMInfo));
	MyBMInfo.bmiHeader.biSize = sizeof(MyBMInfo.bmiHeader);

	if (GetDIBits(hdcMem, hBitmap, 0, 0, NULL, &MyBMInfo, DIB_RGB_COLORS) == 0)
		return FALSE;

	width = MyBMInfo.bmiHeader.biWidth;
	height = MyBMInfo.bmiHeader.biHeight;

	return TRUE;
}

BOOL ReadDataFromBmpFile(LPTSTR szFileName, char *data)
{	
	HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, szFileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (hBitmap == NULL)
		return FALSE;

	HDC hdc, hdcMem;
	hdc = GetDC(NULL);
	hdcMem = CreateCompatibleDC(hdc);

	BITMAPINFO MyBMInfo;
	memset(&MyBMInfo, 0, sizeof(MyBMInfo));
	MyBMInfo.bmiHeader.biSize = sizeof(MyBMInfo.bmiHeader);

	if (GetDIBits(hdcMem, hBitmap, 0, 0, NULL, &MyBMInfo, DIB_RGB_COLORS) == 0)
		return FALSE;

	MyBMInfo.bmiHeader.biSize = sizeof(MyBMInfo.bmiHeader);
	MyBMInfo.bmiHeader.biBitCount = 32;
	MyBMInfo.bmiHeader.biCompression = BI_RGB;
	MyBMInfo.bmiHeader.biHeight = (MyBMInfo.bmiHeader.biHeight < 0) ? (-MyBMInfo.bmiHeader.biHeight) : (MyBMInfo.bmiHeader.biHeight);

	// get the actual bitmap buffer
	if (GetDIBits(hdc, hBitmap, 0, MyBMInfo.bmiHeader.biHeight, (LPVOID)data, &MyBMInfo, DIB_RGB_COLORS) == 0)
		return FALSE;

	return TRUE;
}

void flipVideo()
{
	DWORD ii, jj, kk, mm, nn;
	for (jj = 0; jj < VIDEO_HEIGHT/2; ++jj)
	{
		for (ii = 0; ii < VIDEO_WIDTH; ++ii)
		{
			mm = jj*VIDEO_WIDTH + ii;
			nn = (VIDEO_HEIGHT - 1 - jj)*VIDEO_WIDTH + ii;
			kk = videoFrameBuffer[mm];
			videoFrameBuffer[mm] = videoFrameBuffer[nn];
			videoFrameBuffer[nn] = kk;
		}
	}
	return;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int num_digit, index_begin, index_end;
	WCHAR inputFilename_1[200], inputFilename_2[200], inputFilename[200];
	WCHAR outputFilename[200];

	if (argc < 4)
	{
		MessageBox(NULL, L"More arguments are needed. Usage: Image2Video [First Bmp File] [Last Bmp File] [Video File] [FPS] [Data Rate]\r\nExample:\r\nImage2Video C:\\Temp\\Image\\A_00001.bmp C:\\Temp\\Image\\A_00240.bmp C:\\Temp\\Video\\B.mp4 30 800000", L"Warning", MB_ICONERROR | MB_OK);
		return -1;
	}
	else
	{
		if (wcslen(argv[1]) != wcslen(argv[2]))
		{
			MessageBox(NULL, L"Image filename length mismatch!", L"Warning", MB_ICONERROR | MB_OK);
			return -1;
		}

		int ii, jj, len = wcslen(argv[1]);
		wcsncpy_s(inputFilename_1, argv[1], len);
		wcsncpy_s(inputFilename_2, argv[2], len);

		if (GetWidthHeightInfoOfBmpFile(inputFilename_1, VIDEO_WIDTH, VIDEO_HEIGHT) == FALSE)
		{
			MessageBox(NULL, L"Failed to read image information!", L"Warning", MB_ICONERROR | MB_OK);
			return -1;
		}

		for (ii = 0; ii < len; ii++)
		{
			if (inputFilename_1[ii] != inputFilename_2[ii])
			{
				jj = ii;
				break;
			}
		}

		wcsncpy_s(inputFilename, argv[1], jj);
		wcsncpy_s(inputFilename_1, inputFilename_1 + jj, len - jj);
		wcsncpy_s(inputFilename_2, inputFilename_2 + jj, len - jj);
		wcsncpy_s(outputFilename, argv[3], wcslen(argv[3]));

		num_digit = wcslen(inputFilename_1) - 4;
		wcsncpy_s(inputFilename_1, inputFilename_1, num_digit);
		wcsncpy_s(inputFilename_2, inputFilename_2, num_digit);

		index_begin = _ttoi(inputFilename_1);
		index_end = _ttoi(inputFilename_2);

		_wcslwr_s(argv[3], wcslen(argv[3])+1);
		if (wcsstr(argv[3], _T(".wmv")) != NULL)
			VIDEO_ENCODING_FORMAT = MFVideoFormat_WVC1;

		if (argc >= 5)
			VIDEO_FPS = _ttoi(argv[4]);
		if (argc >= 6)
			VIDEO_BIT_RATE = _ttoi(argv[5]);
	}
	/*
	MessageBox(NULL, argv[0], L"Message", MB_ICONINFORMATION | MB_OK);
	MessageBox(NULL, inputFilename_1, L"Message", MB_ICONINFORMATION | MB_OK);
	MessageBox(NULL, inputFilename_2, L"Message", MB_ICONINFORMATION | MB_OK);
	MessageBox(NULL, inputFilename, L"Message", MB_ICONINFORMATION | MB_OK);
	MessageBox(NULL, outputFilename, L"Message", MB_ICONINFORMATION | MB_OK);
	*/

	VIDEO_FRAME_DURATION = 10 * 1000 * 1000 / VIDEO_FPS;
	VIDEO_PELS = VIDEO_WIDTH * VIDEO_HEIGHT;
	VIDEO_FRAME_COUNT = index_end - index_begin + 1;
	videoFrameBuffer = new DWORD[VIDEO_PELS];

	char *RGB32 = new char[VIDEO_PELS * 4];
	WCHAR buf[100], fn[200];

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(hr))
	{
		hr = MFStartup(MF_VERSION);
		if (SUCCEEDED(hr))
		{
			IMFSinkWriter *pSinkWriter = NULL;
			DWORD stream;

			hr = InitializeSinkWriter(&pSinkWriter, &stream, outputFilename);
			if (SUCCEEDED(hr))
			{
				// Send frames to the sink writer.
				LONGLONG rtStart = 0;

				WCHAR s_str[20];
				swprintf(s_str, 20, L"%%%02dd.bmp", num_digit);

				for (DWORD i = 0; i < VIDEO_FRAME_COUNT; ++i)
				{
					swprintf(buf, 100, s_str, index_begin + i);
					wcsncpy_s(fn, inputFilename, wcslen(inputFilename));
					wcscat_s(fn, buf);
					if(ReadDataFromBmpFile(fn, RGB32)==FALSE)
						MessageBox(NULL, fn, L"Warning", MB_ICONERROR | MB_OK);

					for (DWORD j = 0; j < VIDEO_PELS; ++j)
					{
						videoFrameBuffer[j] = RGB32[4 * j + 2] * 256 * 256 + RGB32[4 * j + 1] * 256 + RGB32[4 * j + 0];
					}

					if (VIDEO_ENCODING_FORMAT == MFVideoFormat_WVC1)
						flipVideo();

					hr = WriteFrame(pSinkWriter, stream, rtStart);
					if (FAILED(hr))
					{
						break;
					}
					rtStart += VIDEO_FRAME_DURATION;
				}
			}
			if (SUCCEEDED(hr))
			{
				hr = pSinkWriter->Finalize();
			}
			SafeRelease(&pSinkWriter);
			MFShutdown();
		}
		CoUninitialize();
	}

	delete[]RGB32;
	delete[]videoFrameBuffer;

	MessageBox(NULL, L"Job completed!", L"Great!", MB_ICONINFORMATION | MB_OK);
	return 0;
}


