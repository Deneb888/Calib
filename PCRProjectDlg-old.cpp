
// PCRProjectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"
#include "afxdialogex.h"

//....................................................................
#include <wtypes.h>
#include <initguid.h>

#define MAX_LOADSTRING 256

extern "C" {

	// This file is in the Windows DDK available from Microsoft.
#include "hidsdi.h"

#include <setupapi.h>
#include <dbt.h>
}
//....................................................................

#ifdef _DEBUG
#define new DEBUG_NEW
//....................................................................
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//....................................................................
#endif
//....................................................................

//....................................................................

//....................................................................
//function prototypes

BOOL DeviceNameMatch(LPARAM lParam);
bool FindTheHID();
LRESULT Main_OnDeviceChange(WPARAM wParam, LPARAM lParam);
void CloseHandles();
void DisplayInputReport();
void DisplayReceivedData(char ReceivedByte);
void GetDeviceCapabilities();
void PrepareForOverlappedTransfer();
void ReadAndWriteToDevice();
void ReadHIDInputReport();
void RegisterForDeviceNotifications();
void WriteHIDOutputReport();

UINT ReadReportThread(LPVOID pParam);	// 读报告线程

//....................................................................

//....................................................................
//Application global variables 
DWORD								ActualBytesRead;
DWORD								BytesRead;
HIDP_CAPS							Capabilities;
DWORD								cbBytesRead;
PSP_DEVICE_INTERFACE_DETAIL_DATA	detailData;
HANDLE								DeviceHandle;
DWORD								dwError;
char								FeatureReport[256];
HANDLE								hEventObject;
HANDLE								hDevInfo;
GUID								HidGuid;
OVERLAPPED							HIDOverlapped;
char								InputReport[HIDREPORTNUM];
ULONG								Length;
LPOVERLAPPED						lpOverLap;
bool								MyDeviceDetected = FALSE; 
CString								MyDevicePathName;
DWORD								NumberOfBytesRead;
char								OutputReport[HIDREPORTNUM];
HANDLE								ReadHandle;
DWORD								ReportType;
ULONG								Required;
CString								ValueToDisplay;
HANDLE								WriteHandle;


//These are the vendor and product IDs to look for.
//Uses Lakeview Research's Vendor ID.
int VendorID = 0x0483;
int ProductID = 0x5750;
//....................................................................

//*****************************************************
//Global variable definition
//*****************************************************
BYTE TxData[TxNum];		// the buffer of sent data to COMX
BYTE RxData[RxNum];		// the buffer of received data from COMX
CByteArray array;		// the buffer used to send data to COMX

CString RegRecStr;				//接收数据字符串buffer 十六进制
CString Dec_RegRecStr;			//接收数据字符串buffer 十进制
CString Valid_RegRecStr;		//有效接收数据字符串buffer 十六进制
CString Valid_Dec_RegRecStr;	//有效接收数据字符串buffer 十进制

int pnum;		//CommPort number

int mRegFlag;	//主对话框发送消息标志

//接收报告用的OVERLAPPED。
OVERLAPPED ReadOverlapped;

//指向读报告线程的指针
CWinThread * pReadReportThread;

//*****************************************************
//External variable definition
//*****************************************************
extern int RegFlag;		// register dialog message flag
extern int GraFlag;		// graphic dialog message flag
extern int TrimFlag;	// trim dialog message flag

extern bool Gra_pageFlag;		// graphic dialog 画页循环标志
extern bool Gra_videoFlag;		// graphic dialog video循环标志

extern BYTE RegBuf [regdatanum];	// register dialog transmitted data buffer
extern BYTE GraBuf[gradatanum];	// graphic dialog transmitted data buffer


//****************************************************
//Own function prototype 
//****************************************************
unsigned char AsicConvert (unsigned char i, unsigned char j);				//ASIC convert to HEX
int ChangeNum (CString str, int length);									//十六进制字符串转十进制整型
 char* EditDataCvtChar (CString strCnv,  char * charRec);	//编辑框取值转字符变量


 BOOL g_DeviceDetected = false;

 CTrimDlg *g_pTrimDlg = 0;


//*****************************************************
//Own function
//*****************************************************

//ASIC字符转十六进制函数
unsigned char AsicConvert (unsigned char i, unsigned char j)
{
	switch (i) 
	{	               
	case 0x30:return (j=0x00);break; 
	case 0x31:return (j=0x01);break; 
	case 0x32:return (j=0x02);break; 
	case 0x33:return (j=0x03);break; 
	case 0x34:return (j=0x04);break; 
	case 0x35:return (j=0x05);break; 
	case 0x36:return (j=0x06);break; 
	case 0x37:return (j=0x07);break; 
	case 0x38:return (j=0x08);break; 
	case 0x39:return (j=0x09);break; 
	case 0x41:
	case 0x61:return (j=0x0A);break;
	case 0x42:	
	case 0x62:return (j=0x0B);break; 
	case 0x43:
	case 0x63:return (j=0x0C);break; 
	case 0x44:
	case 0x64:return (j=0x0d);break;
	case 0x45:	
	case 0x65:return (j=0x0e);break; 
	case 0x46:
	case 0x66:return (j=0x0f);break;
	case 0x20:return (' ');break; 
	default: return(j=0x10);break;
	}
}

//字符串转十进制函数
int ChangeNum (CString str, int length)
{
	char  revstr[16]={0};  
	int   num[16]={0};  
	int   count=1;  
	int   result=0;  
	strcpy_s(revstr,str);  
	for   (int i=length-1;i>=0;i--)  
	{  
		if ((revstr[i]>='0') && (revstr[i]<='9'))  
			num[i]=revstr[i]-48;//字符0的ASCII值为48
		else if ((revstr[i]>='a') && (revstr[i]<='f'))  
			num[i]=revstr[i]-'a'+10;  
		else if ((revstr[i]>='A') && (revstr[i]<='F'))  
			num[i]=revstr[i]-'A'+10;  
		else  
			num[i]=0;
		result=result+num[i]*count;  
		count=count*16;  
	}  
	return result;
}

//从编辑框取值转char型
 char* EditDataCvtChar (CString strCnv,  char * charRec)
{
	//从RowNum编辑框取值
	//	CString RNum;
	char * BSNum;
	//	unsigned char * BANum;
	int CNumByte;
	unsigned char k=0;
	int i,j=0;

	CNumByte=strCnv.GetLength();
	BSNum = new char [CNumByte];
	charRec = new char [CNumByte];

	BSNum = (char*)(LPCSTR)strCnv;

	for(i=0;i<(CNumByte/2);i++)
	{
		charRec[i] = (AsicConvert(BSNum[j],k)<<4) | AsicConvert(BSNum[j+1],k);
		j += 2;
	}

	return charRec;
}

//向串口发送数据，其中参数为发送数据的byte个数
void CPCRProjectDlg:: CommSend(int num)
{
	int a;

	array.RemoveAll();
	array.SetSize(num);
	for (a=0;a<num;a++)
	{
		array.SetAt(a,TxData[a]);
	}
	m_mscomm.put_Output(COleVariant(array));	//send data
}

//
void CPCRProjectDlg::SendHIDRead()
{
	WPARAM a = 8;
	LPARAM b = 9;
	HWND hwnd = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	::SendMessage(hwnd,WM_ReadHID_event,a,b);
}

//读报告线程
UINT ReadReportThread(LPVOID pParam)
{

	return 0;
}

//....................................................................
LRESULT CPCRProjectDlg::Main_OnDeviceChange(WPARAM wParam, LPARAM lParam)  
{

	//DisplayData("Device change detected.");

	PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;

		if(FindTheHID()) m_TrimDlg.ResetTrim();


	switch(wParam) 
	{
		// Find out if a device has been attached or removed.
		// If yes, see if the name matches the device path name of the device we want to access.

	case DBT_DEVICEARRIVAL:
//		DisplayData("A device has been attached.");

		if (DeviceNameMatch(lParam))
		{
//			DisplayData("My device has been attached.");
		}

		return TRUE; 

	case DBT_DEVICEREMOVECOMPLETE:
//		DisplayData("A device has been removed.");

		if (DeviceNameMatch(lParam))
		{
//			DisplayData("My device has been removed.");

			// Look for the device on the next transfer attempt.

			MyDeviceDetected = false;
		}
		return TRUE; 

	default:
		return TRUE; 
	} 
}

BOOL CPCRProjectDlg::DeviceNameMatch(LPARAM lParam)
{

	// Compare the device path name of a device recently attached or removed 
	// with the device path name of the device we want to communicate with.

	PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;

//	DisplayData("MyDevicePathName = " + MyDevicePathName);

	if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) 
	{

		PDEV_BROADCAST_DEVICEINTERFACE lpdbi = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;


		CString DeviceNameString;

		//The dbch_devicetype parameter indicates that the event applies to a device interface.
		//So the structure in LParam is actually a DEV_BROADCAST_INTERFACE structure, 
		//which begins with a DEV_BROADCAST_HDR.

		//The dbcc_name parameter of DevBroadcastDeviceInterface contains the device name. 

		//Compare the name of the newly attached device with the name of the device 
		//the application is accessing (myDevicePathName).

		DeviceNameString = lpdbi->dbcc_name;

//		DisplayData("DeviceNameString = " + DeviceNameString);


		if ((DeviceNameString.CompareNoCase(MyDevicePathName)) == 0)

		{
			//The name matches.

			return true;
		}
		else
		{
			//It's a different device.

			return false;
		}

	}
	else
	{
		return false;
	}	
}

bool CPCRProjectDlg::FindTheHID()
{
	//Use a series of API calls to find a HID with a specified Vendor IF and Product ID.

	HIDD_ATTRIBUTES						Attributes;
	DWORD								DeviceUsage;
	SP_DEVICE_INTERFACE_DATA			devInfoData;
	bool								LastDevice = FALSE;
	int									MemberIndex = 0;
	LONG								Result;	
	CString								UsageDescription;

	Length = 0;
	detailData = NULL;
	DeviceHandle=NULL;

	/*
	API function: HidD_GetHidGuid
	Get the GUID for all system HIDs.
	Returns: the GUID in HidGuid.
	*/

	HidD_GetHidGuid(&HidGuid);	
	
	/*
	API function: SetupDiGetClassDevs
	Returns: a handle to a device information set for all installed devices.
	Requires: the GUID returned by GetHidGuid.
	*/
	
	hDevInfo=SetupDiGetClassDevs 
		(&HidGuid, 
		NULL, 
		NULL, 
		DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);
		
	devInfoData.cbSize = sizeof(devInfoData);

	//Step through the available devices looking for the one we want. 
	//Quit on detecting the desired device or checking all available devices without success.

	MemberIndex = 0;
	LastDevice = FALSE;

	do
	{
		/*
		API function: SetupDiEnumDeviceInterfaces
		On return, MyDeviceInterfaceData contains the handle to a
		SP_DEVICE_INTERFACE_DATA structure for a detected device.
		Requires:
		The DeviceInfoSet returned in SetupDiGetClassDevs.
		The HidGuid returned in GetHidGuid.
		An index to specify a device.
		*/

		Result=SetupDiEnumDeviceInterfaces 
			(hDevInfo, 
			0, 
			&HidGuid, 
			MemberIndex, 
			&devInfoData);

		if (Result != 0)
		{
			//A device has been detected, so get more information about it.

			/*
			API function: SetupDiGetDeviceInterfaceDetail
			Returns: an SP_DEVICE_INTERFACE_DETAIL_DATA structure
			containing information about a device.
			To retrieve the information, call this function twice.
			The first time returns the size of the structure in Length.
			The second time returns a pointer to the data in DeviceInfoSet.
			Requires:
			A DeviceInfoSet returned by SetupDiGetClassDevs
			The SP_DEVICE_INTERFACE_DATA structure returned by SetupDiEnumDeviceInterfaces.
			
			The final parameter is an optional pointer to an SP_DEV_INFO_DATA structure.
			This application doesn't retrieve or use the structure.			
			If retrieving the structure, set 
			MyDeviceInfoData.cbSize = length of MyDeviceInfoData.
			and pass the structure's address.
			*/
			
			//Get the Length value.
			//The call will return with a "buffer too small" error which can be ignored.

			Result = SetupDiGetDeviceInterfaceDetail 
				(hDevInfo, 
				&devInfoData, 
				NULL, 
				0, 
				&Length, 
				NULL);

			//Allocate memory for the hDevInfo structure, using the returned Length.

			detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(Length);
			
			//Set cbSize in the detailData structure.

			detailData -> cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			//Call the function again, this time passing it the returned buffer size.

			Result = SetupDiGetDeviceInterfaceDetail 
				(hDevInfo, 
				&devInfoData, 
				detailData, 
				Length, 
				&Required, 
				NULL);

			// Open a handle to the device.
			// To enable retrieving information about a system mouse or keyboard,
			// don't request Read or Write access for this handle.

			/*
			API function: CreateFile
			Returns: a handle that enables reading and writing to the device.
			Requires:
			The DevicePath in the detailData structure
			returned by SetupDiGetDeviceInterfaceDetail.
			*/

			DeviceHandle=CreateFile 
				(detailData->DevicePath, 
				0, 
				FILE_SHARE_READ|FILE_SHARE_WRITE, 
				(LPSECURITY_ATTRIBUTES)NULL,
				OPEN_EXISTING, 
				0, 
				NULL);

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			HidD_SetNumInputBuffers(DeviceHandle,HIDBUFSIZE);
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//			DisplayLastError("CreateFile: ");

			/*
			API function: HidD_GetAttributes
			Requests information from the device.
			Requires: the handle returned by CreateFile.
			Returns: a HIDD_ATTRIBUTES structure containing
			the Vendor ID, Product ID, and Product Version Number.
			Use this information to decide if the detected device is
			the one we're looking for.
			*/

			//Set the Size to the number of bytes in the structure.

			Attributes.Size = sizeof(Attributes);

			Result = HidD_GetAttributes 
				(DeviceHandle, 
				&Attributes);
			
//			DisplayLastError("HidD_GetAttributes: ");
			
			//Is it the desired device?

			MyDeviceDetected = FALSE;
			

			if (Attributes.VendorID == VendorID)
			{
				if (Attributes.ProductID == ProductID)
				{
					//Both the Vendor ID and Product ID match.

					MyDeviceDetected = TRUE;
					MyDevicePathName = detailData->DevicePath;
//					DisplayData("Device detected");

					//Register to receive device notifications.

					RegisterForDeviceNotifications();

					//Get the device's capablities.

					GetDeviceCapabilities();

					// Find out if the device is a system mouse or keyboard.
					
					DeviceUsage = (Capabilities.UsagePage * 256) + Capabilities.Usage;

					if (DeviceUsage == 0x102)
						{
						UsageDescription = "mouse";
						}
				
					if (DeviceUsage == 0x106)
						{
						UsageDescription = "keyboard";
						}

					if ((DeviceUsage == 0x102) | (DeviceUsage == 0x106)) 
						{
//						DisplayData("");
//						DisplayData("*************************");
//						DisplayData("The device is a system " + UsageDescription + ".");
//						DisplayData("Windows 2000 and Windows XP don't allow applications");
//						DisplayData("to directly request Input reports from or "); 
//						DisplayData("write Output reports to these devices.");
//						DisplayData("*************************");
//						DisplayData("");
						}

					// Get a handle for writing Output reports.

					WriteHandle=CreateFile 
						(detailData->DevicePath, 
						GENERIC_WRITE, 
						FILE_SHARE_READ|FILE_SHARE_WRITE, 
						(LPSECURITY_ATTRIBUTES)NULL,
						OPEN_EXISTING, 
						0, 
						NULL);

					//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
					HidD_SetNumInputBuffers(WriteHandle,HIDBUFSIZE);
					//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//					DisplayLastError("CreateFile: ");

					// Prepare to read reports using Overlapped I/O.

					PrepareForOverlappedTransfer();

					// Set HID driver input buffer
//					HidD_SetNumInputBuffers(DeviceHandle,64);

				} //if (Attributes.ProductID == ProductID)

				else
					//The Product ID doesn't match.

					CloseHandle(DeviceHandle);

			} //if (Attributes.VendorID == VendorID)

			else
				//The Vendor ID doesn't match.

				CloseHandle(DeviceHandle);

		//Free the memory used by the detailData structure (no longer needed).

		free(detailData);

		}  //if (Result != 0)

		else
			//SetupDiEnumDeviceInterfaces returned 0, so there are no more devices to check.

			LastDevice=TRUE;

		//If we haven't found the device yet, and haven't tried every available device,
		//try the next one.

		MemberIndex = MemberIndex + 1;

	} //do

	while ((LastDevice == FALSE) && (MyDeviceDetected == FALSE));

	if (MyDeviceDetected == FALSE) {
//		DisplayData("Device not detected");
		SetDlgItemText(IDC_STATICOpenComm,"Device Not Detected");
		g_DeviceDetected = false;
	}
	else {
//		DisplayData("Device detected");
		SetDlgItemText(IDC_STATICOpenComm,"ULS24 Device Detected");
		g_DeviceDetected = true;
	}

	//Free the memory reserved for hDevInfo by SetupDiClassDevs.

	SetupDiDestroyDeviceInfoList(hDevInfo);
//	DisplayLastError("SetupDiDestroyDeviceInfoList");

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	HidD_SetNumInputBuffers(DeviceHandle,HIDBUFSIZE);
	HidD_SetNumInputBuffers(WriteHandle,HIDBUFSIZE);
	HidD_SetNumInputBuffers(ReadHandle,HIDBUFSIZE);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	return MyDeviceDetected;
}

void CPCRProjectDlg::CloseHandles()
{
	//Close open handles.

	if (DeviceHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(DeviceHandle);
	}

	if (ReadHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(ReadHandle);
	}

	if (WriteHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(WriteHandle);
	}
}

void CPCRProjectDlg::DisplayInputReport()
{
	USHORT	ByteNumber;
	CHAR	ReceivedByte;

	//Display the received data in the log and the Bytes Received List boxes.
	//Start at the top of the List Box.

	m_BytesReceived.ResetContent();

	//Step through the received bytes and display each.

	for (ByteNumber=0; ByteNumber < Capabilities.InputReportByteLength; ByteNumber++)
	{
		//Get a byte.

		ReceivedByte = InputReport[ByteNumber];

		//Display it.

		DisplayReceivedData(ReceivedByte);
	}
}

void CPCRProjectDlg::DisplayReceivedData(char ReceivedByte)
{
	//Display data received from the device.

	CString	strByteRead;

	//Convert the value to a 2-character Cstring.

	strByteRead.Format("%02X", ReceivedByte);
	strByteRead = strByteRead.Right(2); 

	//Display the value in the Bytes Received List Box.

	m_BytesReceived.InsertString(-1, strByteRead);

	//Display the value in the log List Box (optional).
	//MessageToDisplay.Format("%s%s", "Byte 0: ", strByteRead); 
	//DisplayData(MessageToDisplay);	
	//UpdateData(false);
}

void CPCRProjectDlg::GetDeviceCapabilities()
{
	//Get the Capabilities structure for the device.

	PHIDP_PREPARSED_DATA	PreparsedData;

	/*
	API function: HidD_GetPreparsedData
	Returns: a pointer to a buffer containing the information about the device's capabilities.
	Requires: A handle returned by CreateFile.
	There's no need to access the buffer directly,
	but HidP_GetCaps and other API functions require a pointer to the buffer.
	*/

	HidD_GetPreparsedData 
		(DeviceHandle, 
		&PreparsedData);
//	DisplayLastError("HidD_GetPreparsedData: ");

	/*
	API function: HidP_GetCaps
	Learn the device's capabilities.
	For standard devices such as joysticks, you can find out the specific
	capabilities of the device.
	For a custom device, the software will probably know what the device is capable of,
	and the call only verifies the information.
	Requires: the pointer to the buffer returned by HidD_GetPreparsedData.
	Returns: a Capabilities structure containing the information.
	*/
	
	HidP_GetCaps 
		(PreparsedData, 
		&Capabilities);
//	DisplayLastError("HidP_GetCaps: ");

	//Display the capabilities

	ValueToDisplay.Format("%s%X", "Usage Page: ", Capabilities.UsagePage);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Input Report Byte Length: ", Capabilities.InputReportByteLength);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Output Report Byte Length: ", Capabilities.OutputReportByteLength);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Feature Report Byte Length: ", Capabilities.FeatureReportByteLength);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of Link Collection Nodes: ", Capabilities.NumberLinkCollectionNodes);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of Input Button Caps: ", Capabilities.NumberInputButtonCaps);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of InputValue Caps: ", Capabilities.NumberInputValueCaps);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of InputData Indices: ", Capabilities.NumberInputDataIndices);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of Output Button Caps: ", Capabilities.NumberOutputButtonCaps);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of Output Value Caps: ", Capabilities.NumberOutputValueCaps);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of Output Data Indices: ", Capabilities.NumberOutputDataIndices);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of Feature Button Caps: ", Capabilities.NumberFeatureButtonCaps);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of Feature Value Caps: ", Capabilities.NumberFeatureValueCaps);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of Feature Data Indices: ", Capabilities.NumberFeatureDataIndices);
///	DisplayData(ValueToDisplay);

	//No need for PreparsedData any more, so free the memory it's using.

	HidD_FreePreparsedData(PreparsedData);
//	DisplayLastError("HidD_FreePreparsedData: ") ;
}

void CPCRProjectDlg::PrepareForOverlappedTransfer()
{
	//Get a handle to the device for the overlapped ReadFiles.

	ReadHandle=CreateFile 
		(detailData->DevicePath, 
		GENERIC_READ, 
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		(LPSECURITY_ATTRIBUTES)NULL, 
		OPEN_EXISTING, 
		FILE_FLAG_OVERLAPPED,
		NULL);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		HidD_SetNumInputBuffers(ReadHandle,HIDBUFSIZE);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//	DisplayLastError("CreateFile (ReadHandle): ");

	//Get an event object for the overlapped structure.

	/*API function: CreateEvent
	Requires:
	  Security attributes or Null
	  Manual reset (true). Use ResetEvent to set the event object's state to non-signaled.
	  Initial state (true = signaled) 
	  Event object name (optional)
	Returns: a handle to the event object
	*/

	if (hEventObject == 0)
	{
		hEventObject = CreateEvent 
			(NULL, 
			TRUE, 
			TRUE, 
			"");
//	DisplayLastError("CreateEvent: ") ;

	//Set the members of the overlapped structure.

	HIDOverlapped.hEvent = hEventObject;
	HIDOverlapped.Offset = 0;
	HIDOverlapped.OffsetHigh = 0;
	}
}

void CPCRProjectDlg::ReadAndWriteToDevice()
{
	//If necessary, find the device and learn its capabilities.
	//Then send a report and request a report.

	//Clear the List Box (optional).
	//m_ResultsList.ResetContent();

//	DisplayData("***HID Test Report***");
//	DisplayCurrentTime();

	//If the device hasn't been detected already, look for it.

	if (MyDeviceDetected==FALSE)
	{
		MyDeviceDetected=FindTheHID();
	}

	// Do nothing if the device isn't detected.

	if (MyDeviceDetected==TRUE)
	{
		//Write a report to the device.

		WriteHIDOutputReport();

		//Active read message

		SendHIDRead();
	} 
}

void CPCRProjectDlg::ReadHIDInputReport()
{

	// Retrieve an Input report from the device.

	DWORD	Result;
	
	//The first byte is the report number.
	InputReport[0]=0;

 	
	/*API call:ReadFile
	'Returns: the report in InputReport.
	'Requires: a device handle returned by CreateFile
	'(for overlapped I/O, CreateFile must be called with FILE_FLAG_OVERLAPPED),
	'the Input report length in bytes returned by HidP_GetCaps,
	'and an overlapped structure whose hEvent member is set to an event object.
	*/

	if (ReadHandle != INVALID_HANDLE_VALUE)
		{
		Result = ReadFile 
		(ReadHandle, 
		InputReport, 
		Capabilities.InputReportByteLength, 
		&NumberOfBytesRead,
		(LPOVERLAPPED) &HIDOverlapped
		); 
		}
 
//	DisplayLastError("ReadFile: ") ;

	/*API call:WaitForSingleObject
	'Used with overlapped ReadFile.
	'Returns when ReadFile has received the requested amount of data or on timeout.
	'Requires an event object created with CreateEvent
	'and a timeout value in milliseconds.
	*/

	Result = WaitForSingleObject 
		(hEventObject, 
		100000);


	CString strtest;		//接收到下位机返回的错误的命令buffer
	CString strtemp;		//临时接收字符串buffer,暂存一个byte数据，十六进制
	CString DStrtemp;		//临时接收字符串buffer,暂存一个byte数据，十进制
	CString Valid_strtemp;	//有效临时接收字符串buffer,暂存一个byte数据，十六进制
	CString Valid_DStrtemp;	//有效临时接收字符串buffer,暂存一个byte数据，十进制
	long k;
	BYTE rCmd;	//下位机返回命令寄存buffer
	BYTE rType;	//下位机返回type寄存buffer
 
	switch (Result)
	{
	case WAIT_OBJECT_0:
		{
				//将各转换字符串buffer清零
				RegRecStr="";	// Clear之前的显示数据
				Dec_RegRecStr="";	// Clear之前的显示数据
				Valid_RegRecStr="";
				Valid_Dec_RegRecStr="";

				//处理接收的数据
				for (k=0;k<HIDREPORTNUM-1;k++)
					RxData[k] = InputReport[k+1];	//将接收的数据按字节分配到字节存储buffer(RxData[200])
				//取出返回的命令和type
				rCmd = RxData[2];	//取出下位机返回的命令
				rType = RxData[4];	//取出下位机返回的type
				strtest.Format("%2x",rCmd);

				//将返回的数据整理成各所需的字符串
				//取所有数据
				for(k=0;k<HIDREPORTNUM;k++)			 //将字符数组的各字符转成字符串
				{
					strtemp.Format("%02X ",RxData[k]);			//每个字符转化，十六进制显示,如有“0”也会显示如“02”
					//若不要显示“0”，strtemp.Format("%x",bt)
					DStrtemp.Format("%02d",RxData[k]);			//十进制显示，其它如上

					RegRecStr+=strtemp;					//将每个十六进制字符转成的字符串组合成一个字符串
					//十六进制转化中相邻各byte间自带空格
					//接收的每个包的数据都存在该字符串，除非对该字符串清零，否则之前所有数据保存

					Dec_RegRecStr += (DStrtemp+" ");	//将每个十进制字符转成的字符串组合成一个字符串
					//十进制转化相邻两个字符不自带空格，在这里添加
					//接收的每个包的数据都存在该字符串，除非对该字符串清零，否则之前所有数据保存
				}
				//每行数据间加入回车
				//		RegRecStr += "\r\n";
				//		Dec_RegRecStr += "\r\n";

				//根据下位机返回的命令，进入各自的消息处理程序
				switch(rCmd)
				{
				case GraCmd:
					{
						//取相应的有效数据
						if ((rType==0x01)|(rType==0x02)|(rType==0x03) )		//返回12 pixel 数据时
						{	
							if (RxData[5]==0x0b)		// HID读到第12行后停止读取
								Gra_pageFlag = false;
							else
								Gra_pageFlag = true;

							for(k=0; k<26; k++)
							{
								Valid_strtemp.Format("%02X",RxData[k+6]);	//十六进制显示字符串buffer, 每个byte间加空格
								Valid_DStrtemp.Format("%02d",RxData[k+6]);	//十进制显示字符串buffer, 每个byte间加空格

								Valid_RegRecStr += (Valid_strtemp+" ");
								Valid_Dec_RegRecStr += (Valid_DStrtemp+" ");
							}
							//每行数据间加入回车
							//					Valid_RegRecStr += "\r\n";
							//					Valid_Dec_RegRecStr += "\r\n";
						}
						else
						{
							if ((rType==0x07)|(rType==0x08)|(rType==0x0b))	//返回24 pixel 数据时
							{
								if (RxData[5]==0x17)	// HID读到第24行后停止读取
									Gra_pageFlag = false;
								else
									Gra_pageFlag = true;

								for(k=0; k<50; k++)
								{
									Valid_strtemp.Format("%02X",RxData[k+6]);	//十六进制显示字符串 buffer, 每个 byte 间加空格
									Valid_DStrtemp.Format("%02d",RxData[k+6]);	//十进制显示字符串 buffer, 每个 byte 间加空格

									Valid_RegRecStr += (Valid_strtemp+" ");
									Valid_Dec_RegRecStr += (Valid_DStrtemp+" ");
								}
								//每行数据间加入回车
								//						Valid_RegRecStr += "\r\n";
								//						Valid_Dec_RegRecStr += "\r\n";
							}
							else	//其它type待定
							{
								Valid_RegRecStr = "";
								Valid_Dec_RegRecStr = "";
							}
						}

						//启动Graphic dialog消息处理程序
						m_GraDlg.SendMessage(UM_GRAPROCESS);				
						break;
					}
				case TempCmd:
				case PidCmd:
				case PidReadCmd:
					{
						m_RegDlg.SendMessage(UM_REGPROCESS);
						//AfxMessageBox(_T("receive tempture command"));
						break;
					}
				case 0x14:
					{
						RegRecStr="";	// Clear之前的显示数据
						Valid_RegRecStr="";	// Clear之前的显示数据

						RegRecStr.Format("cycle number is %d",RxData[4]);
						//				Dec_RegRecStr.Format("cycle number is %d",RxData[4]);
						Valid_RegRecStr.Format("cycle number is %d",RxData[4]);
						//				Valid_Dec_RegRecStr.Format("cycle number is %d",RxData[4]);

						//启动Graphic dialog消息处理程序
						m_GraDlg.SendMessage(UM_GRAPROCESS);				
						break;
					}
				default:
					{
						//				AfxMessageBox(strtest);
						break;
					}
				}
					
		break;
		}
	case WAIT_TIMEOUT:
		{
		SetDlgItemText(IDC_STATICOpenComm, "ReadFile timeout");

		/*API call: CancelIo
		Cancels the ReadFile
        Requires the device handle.
        Returns non-zero on success.
		*/
		
		Result = CancelIo(ReadHandle);
		
		//A timeout may mean that the device has been removed. 
		//Close the device handles and set MyDeviceDetected = False 
		//so the next access attempt will search for the device.
		CloseHandles();
		MyDeviceDetected = FALSE;
		break;
		}
	default:
		{
		//Close the device handles and set MyDeviceDetected = False 
		//so the next access attempt will search for the device.

		CloseHandles();
		SetDlgItemText(IDC_STATICOpenComm,"Can't read from device");
		MyDeviceDetected = FALSE;
		break;
		}
	}

	/*
	API call: ResetEvent
	Sets the event object to non-signaled.
	Requires a handle to the event object.
	Returns non-zero on success.
	*/

	ResetEvent(hEventObject);

	//Display the report data.

	DisplayInputReport();

}

void CPCRProjectDlg::RegisterForDeviceNotifications()
{

	// Request to receive messages when a device is attached or removed.
	// Also see WM_DEVICECHANGE in BEGIN_MESSAGE_MAP(CPCRProjectDlg, CDialog).

	DEV_BROADCAST_DEVICEINTERFACE DevBroadcastDeviceInterface;
	HDEVNOTIFY DeviceNotificationHandle;

	DevBroadcastDeviceInterface.dbcc_size = sizeof(DevBroadcastDeviceInterface);
	DevBroadcastDeviceInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	DevBroadcastDeviceInterface.dbcc_classguid = HidGuid;

	DeviceNotificationHandle =
		RegisterDeviceNotification(m_hWnd, &DevBroadcastDeviceInterface, DEVICE_NOTIFY_WINDOW_HANDLE);

}

void CPCRProjectDlg::WriteHIDOutputReport()
{
	//Send a report to the device.

	DWORD	BytesWritten = 0;
	INT		Index =0;
	ULONG	Result;
	CString	strBytesWritten = "";

	UpdateData(true);

	//The first byte is the report number.

	OutputReport[0]=0;

	for (int i=1; i<TxNum+1; i++)
	 OutputReport[i] = TxData[i-1];
/*
	OutputReport[1]=0xaa;
	OutputReport[2]=0x13;
	OutputReport[3]=0x0c;
	OutputReport[4]=0x04;
	OutputReport[5]=0x01;
	OutputReport[6]=0x03;
	OutputReport[7]=0x03;
	OutputReport[8]=0x00;
	OutputReport[9]=0x00;
	OutputReport[10]=0x48;
	OutputReport[11]=0x42;
	OutputReport[12]=0x00;
	OutputReport[13]=0x10;
	OutputReport[14]=0x00;
	OutputReport[15]=0x00;
	OutputReport[16]=0xa0;
	OutputReport[17]=0x42;
	OutputReport[18]=0x00;
	OutputReport[19]=0x10;
	OutputReport[20]=0xb6;
	OutputReport[21]=0x17;
	OutputReport[22]=0x17;
*/
	/*
		API Function: WriteFile
		Sends a report to the device.
		Returns: success or failure.
		Requires:
		A device handle returned by CreateFile.
		A buffer that holds the report.
		The Output Report length returned by HidP_GetCaps,
		A variable to hold the number of bytes written.
	*/

		if (WriteHandle != INVALID_HANDLE_VALUE)
			{
			Result = WriteFile 
			(WriteHandle, 
			OutputReport, 
			Capabilities.OutputReportByteLength, 
			&BytesWritten, 
			NULL);
		}

		//Display the result of the API call and the report bytes.

		if (!Result)
			{
			//The WriteFile failed, so close the handles, display a message,
			//and set MyDeviceDetected to FALSE so the next attempt will look for the device.

			CloseHandles();
			SetDlgItemText(IDC_STATICOpenComm,"Can't write to device");
			MyDeviceDetected = FALSE;
			}

}

//....................................................................


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPCRProjectDlg dialog




CPCRProjectDlg::CPCRProjectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPCRProjectDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_StaticOpenComm = _T("");
	//  m_strBytesReceived = _T("");
	m_strBytesReceived = _T("");
}

void CPCRProjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MSCOMM1, m_mscomm);
	DDX_Control(pDX, IDC_TAB1, m_tab);
	DDX_Text(pDX, IDC_STATICOpenComm, m_StaticOpenComm);
	//  DDX_Text(pDX, IDC_EDIT_HIDRECEIVE, m_strBytesReceived);
	//  DDX_Control(pDX, IDC_EDIT_HIDRECEIVE, m_BytesReceived);
	DDX_Control(pDX, IDC_lstBytesReceived, m_BytesReceived);
	DDX_LBString(pDX, IDC_lstBytesReceived, m_strBytesReceived);
}

BEGIN_MESSAGE_MAP(CPCRProjectDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CPCRProjectDlg::OnTcnSelchangeTab1)
	ON_MESSAGE(WM_RegDlg_event,OnRegDlg)
	ON_MESSAGE(WM_GraDlg_event,OnGraDlg)
	ON_MESSAGE(WM_TrimDlg_event,OnTrimDlg)
	ON_MESSAGE(WM_ReadHID_event,OnReadHID)
	ON_BN_CLICKED(IDC_BTN_OPENCOMM, &CPCRProjectDlg::OnBnClickedBtnOpencomm)
	ON_BN_CLICKED(IDC_BTN_OPENHID, &CPCRProjectDlg::OnBnClickedBtnOpenhid)
	//....................................................................
	//ON_WM_DEVICECHANGE()
	ON_MESSAGE(WM_DEVICECHANGE, Main_OnDeviceChange)
	//....................................................................
	ON_BN_CLICKED(IDC_BTN_SENDHID, &CPCRProjectDlg::OnBnClickedBtnSendhid)
	ON_BN_CLICKED(IDC_BTN_READHID, &CPCRProjectDlg::OnBnClickedBtnReadhid)
	ON_WM_SHOWWINDOW()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CPCRProjectDlg message handlers

BOOL CPCRProjectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	//Initial TAB component

	CRect tabRect;

//	m_tab.InsertItem(0, _T("Thermal Cycle"));         // 插入第一个标签“Register”     
	m_tab.InsertItem(0, _T("Image window"));  // 插入第二个标签“Graphic”  
	m_tab.InsertItem(1, _T("Parameters"));
//	m_RegDlg.Create(IDD_REGISTER_DIALOG, &m_tab);	// 创建第一个标签页     
	m_GraDlg.Create(IDD_GRAPHIC_DIALOG, &m_tab);	// 创建第二个标签页  
	m_TrimDlg.Create(IDD_TRIM_DIALOG, &m_tab);		//创建第三个标签页

	m_tab.GetClientRect(&tabRect);    // 获取标签控件客户区Rect     
	// 调整tabRect，使其覆盖范围适合放置标签页     
	tabRect.left += 1;                    
	tabRect.right -= 1;     
	tabRect.top += 25;     
	tabRect.bottom -= 1;     
	// 根据调整好的tabRect放置m_RegDlg子对话框，并设置为显示     
//	m_RegDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);     
	// 根据调整好的tabRect放置m_GraDlg子对话框，并设置为隐藏     
	m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);  // SWP_HIDEWINDOW);
	// 根据调整好的tabRect放置m_TrimDlg子对话框，并设置为隐藏     
	m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
/*
	//Auto detect and open the CommPort
	GetCom();

	if (m_mscomm.get_PortOpen())  
		m_mscomm.put_PortOpen(FALSE); //关闭串口  

	m_mscomm.put_CommPort(pnum);     //设定串口为COM4  


	m_mscomm.put_Settings(_T("9600,n,8,1"));  //设定波特率9600，无奇偶校验，8位数据位，1位停止位  
	m_mscomm.put_InputMode(1);    //设定数据接收模式，1为二进制，0为文本  
	m_mscomm.put_InputLen(ONCOMNUM);     //设定当前接收区数据长度，0表示全部读取  
	m_mscomm.put_InBufferSize(1024);  //设定输入缓冲区大小为1024 byte  
	m_mscomm.put_OutBufferSize(1024); //设定输出缓冲区大小为1024 byte  
	m_mscomm.put_RThreshold(ONCOMNUM);   //每接收到1个字符时，触发OnComm事件  
	m_mscomm.put_SThreshold(0);   //设定每发送多少个字符触发OnComm事件，0表示不触发OnComm事件  

	if (!m_mscomm.get_PortOpen())  
	{
		m_mscomm.put_PortOpen(TRUE);  //打开串口 
		m_StaticOpenComm = _T("Connect the ") + m_StaticOpenComm;
		UpdateData(FALSE);
	}
	else  
		AfxMessageBox(_T("can't open the comm port"));
*/
	if(FindTheHID()) m_TrimDlg.ResetTrim();

	g_pTrimDlg = &m_TrimDlg;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPCRProjectDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPCRProjectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPCRProjectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BEGIN_EVENTSINK_MAP(CPCRProjectDlg, CDialogEx)
	ON_EVENT(CPCRProjectDlg, IDC_MSCOMM1, 1, CPCRProjectDlg::OnCommMscomm1, VTS_NONE)
END_EVENTSINK_MAP()


void CPCRProjectDlg::OnCommMscomm1()
{
	// TODO: Add your message handler code here
	int nEvent;
	nEvent = m_mscomm.get_CommEvent();

	CString strtest;		//接收到下位机返回的错误的命令buffer
	CString strtemp;		//临时接收字符串buffer,暂存一个byte数据，十六进制
	CString DStrtemp;		//临时接收字符串buffer,暂存一个byte数据，十进制
	CString Valid_strtemp;	//有效临时接收字符串buffer,暂存一个byte数据，十六进制
	CString Valid_DStrtemp;	//有效临时接收字符串buffer,暂存一个byte数据，十进制
	VARIANT variant_inp;
	COleSafeArray safearray_inp;
	long len,k;
	BYTE rCmd;	//下位机返回命令寄存buffer
	BYTE rType;	//下位机返回type寄存buffer

	if (nEvent==2)	//判断进入串口中断是因为接收数据满足设置
	{
		//将各转换字符串buffer清零
		RegRecStr="";	// Clear之前的显示数据
		Dec_RegRecStr="";	// Clear之前的显示数据
		Valid_RegRecStr="";
		Valid_Dec_RegRecStr="";

		//处理接收的数据
		variant_inp = m_mscomm.get_Input();		//将接收的数据存到接收buffer(variant_inp)
		safearray_inp = variant_inp;			//类型转换
		len = safearray_inp.GetOneDimSize();	//获取接收数据个数
		for (k=0;k<len;k++)
			safearray_inp.GetElement(&k,RxData+k);	//将接收的数据按字节分配到字节存储buffer(RxData[200])
		//取出返回的命令和type
		rCmd = RxData[2];	//取出下位机返回的命令
		rType = RxData[4];	//取出下位机返回的type
		strtest.Format("%2x",rCmd);

		//将返回的数据整理成各所需的字符串
		//取所有数据
		for(k=0;k<len;k++)			 //将字符数组的各字符转成字符串
		{
			strtemp.Format("%02X ",RxData[k]);			//每个字符转化，十六进制显示,如有“0”也会显示如“02”
			//若不要显示“0”，strtemp.Format("%x",bt)
			DStrtemp.Format("%02d",RxData[k]);			//十进制显示，其它如上

			RegRecStr+=strtemp;					//将每个十六进制字符转成的字符串组合成一个字符串
			//十六进制转化中相邻各byte间自带空格
			//接收的每个包的数据都存在该字符串，除非对该字符串清零，否则之前所有数据保存

			Dec_RegRecStr += (DStrtemp+" ");	//将每个十进制字符转成的字符串组合成一个字符串
			//十进制转化相邻两个字符不自带空格，在这里添加
			//接收的每个包的数据都存在该字符串，除非对该字符串清零，否则之前所有数据保存
		}
		//每行数据间加入回车
//		RegRecStr += "\r\n";
//		Dec_RegRecStr += "\r\n";

		//根据下位机返回的命令，进入各自的消息处理程序
		switch(rCmd)
		{
		case GraCmd:
			{
				//取相应的有效数据
				if ((rType==0x01)|(rType==0x02)|(rType==0x03) )		//返回12 pixel 数据时
				{
					for(k=0; k<26; k++)
					{
						Valid_strtemp.Format("%02X",RxData[k+6]);	//十六进制显示字符串buffer, 每个byte间加空格
						Valid_DStrtemp.Format("%02d",RxData[k+6]);	//十进制显示字符串buffer, 每个byte间加空格

						Valid_RegRecStr += (Valid_strtemp+" ");
						Valid_Dec_RegRecStr += (Valid_DStrtemp+" ");
					}
					//每行数据间加入回车
//					Valid_RegRecStr += "\r\n";
//					Valid_Dec_RegRecStr += "\r\n";
				}
				else
				{
					if ((rType==0x07)|(rType==0x08)|(rType==0x0b))	//返回24 pixel 数据时
					{
						for(k=0; k<50; k++)
						{
							Valid_strtemp.Format("%02X",RxData[k+6]);	//十六进制显示字符串 buffer, 每个 byte 间加空格
							Valid_DStrtemp.Format("%02d",RxData[k+6]);	//十进制显示字符串 buffer, 每个 byte 间加空格

							Valid_RegRecStr += (Valid_strtemp+" ");
							Valid_Dec_RegRecStr += (Valid_DStrtemp+" ");
						}
						//每行数据间加入回车
//						Valid_RegRecStr += "\r\n";
//						Valid_Dec_RegRecStr += "\r\n";
					}
					else	//其它type待定
					{
						Valid_RegRecStr = "";
						Valid_Dec_RegRecStr = "";
					}
				}

				//启动Graphic dialog消息处理程序
				m_GraDlg.SendMessage(UM_GRAPROCESS);				
				break;
			}
		case TempCmd:
		case PidCmd:
		case PidReadCmd:
			{
				m_RegDlg.SendMessage(UM_REGPROCESS);
				//AfxMessageBox(_T("receive tempture command"));
				break;
			}
		case 0x14:
			{
				RegRecStr="";	// Clear之前的显示数据
				Valid_RegRecStr="";	// Clear之前的显示数据
				
				RegRecStr.Format("cycle number is %d",RxData[4]);
//				Dec_RegRecStr.Format("cycle number is %d",RxData[4]);
				Valid_RegRecStr.Format("cycle number is %d",RxData[4]);
//				Valid_Dec_RegRecStr.Format("cycle number is %d",RxData[4]);

				//启动Graphic dialog消息处理程序
				m_GraDlg.SendMessage(UM_GRAPROCESS);				
				break;
			}
		default:
			{
//				AfxMessageBox(strtest);
				break;
			}
		}
	}
}

LRESULT CPCRProjectDlg::OnReadHID(WPARAM wParam, LPARAM lParam)
{
	ReadHIDInputReport();

	return 0;
}

LRESULT CPCRProjectDlg::OnRegDlg(WPARAM wParam, LPARAM lParam)
{
	switch(RegFlag)
	{

	case sendregmsg:
		{
//			CommSend(dNum);
			WriteHIDOutputReport();		// 向HID发送数据

			//命令标志、传输buffer清零
			RegFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(RegBuf,0,sizeof(RegBuf));

			SendHIDRead();		// 读取HID返回数据

			break;
		}
	default:
		AfxMessageBox(_T("Please Send Register Flag Correctly"));
		break;

	}

	return 0;
}

LRESULT CPCRProjectDlg::OnGraDlg(WPARAM wParam, LPARAM lParam)
{
	switch(GraFlag)
	{
	case sendgramsg:
		{
//			CommSend(dNum);		//向串口发送数据
			WriteHIDOutputReport();		// 向HID发送数据

			//命令标志、传输buffer清零
			GraFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(GraBuf,0,sizeof(GraBuf));

			SendHIDRead();		// 读取HID返回数据
			break;
		}
	case sendpagemsg:
		{
			WriteHIDOutputReport();		// 向HID发送数据

			//命令标志、传输buffer清零
			GraFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(GraBuf,0,sizeof(GraBuf));

			while(Gra_pageFlag)
				ReadHIDInputReport();
			break;
		}
	case sendvideomsg:
		{
			if (Gra_videoFlag)
				SetTimer(1,500,NULL);
			else
				KillTimer(1);

			break;
		}

	default:
		AfxMessageBox(_T("Please Send Graphic Flag Correctly"));
		break;

	}

	return 0;
}

LRESULT CPCRProjectDlg::OnTrimDlg(WPARAM wParam, LPARAM lParam)
{
	switch(TrimFlag)
	{
	case sendtrimmsg:
		{
//			CommSend(dNum);		//向串口发送数据
			WriteHIDOutputReport();		// 向HID发送数据

			//命令标志、传输buffer清零
			TrimFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(TrimBuf,0,sizeof(TrimBuf));

			SendHIDRead();		// 读取HID返回数据
			break;
		}
	default:
		AfxMessageBox(_T("Please Send Trim Flag Correctly"));
		break;

	}

	return 0;
}


void CPCRProjectDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;

	CRect tabRect;    // 标签控件客户区的Rect     

	// 获取标签控件客户区Rect，并对其调整，以适合放置标签页     
	m_tab.GetClientRect(&tabRect);     
	tabRect.left += 1;     
	tabRect.right -= 1;     
	tabRect.top += 25;     
	tabRect.bottom -= 1;     

	switch (m_tab.GetCurSel())     
	{     
		// 如果标签控件当前选择标签为“register”，则显示m_regdlg对话框，隐藏m_gradlg对话框     
/*	case 0:     
		m_RegDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);     
		m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
		m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
		break;     
		// 如果标签控件当前选择标签为“graphic”，则隐藏m_regdlg对话框，显示m_gradlg对话框     
*/	case 0:     
//		m_RegDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);     
		m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW); 
		m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
		break;
	case 1:
//		m_RegDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);     
		m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
		m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
	default:     
		break;     
	}
}

void CPCRProjectDlg::GetCom()
{
	HANDLE  hCom;
	int i;
	CString str;
	BOOL flag;

	flag = FALSE;	//串口是否找到标志

	for (i = 1;i <= 16;i++)
	{//此程序支持16个串口，从1到16依次查找

		str.Format(_T("\\\\.\\COM%d"),i);		//设置要查找的串口地址

		hCom = CreateFile(str, 0, 0, 0, 
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);	//打开指定地址的串口
		if(INVALID_HANDLE_VALUE != hCom )
		{//能打开该串口，则添加该串口
			CloseHandle(hCom);
			str = str.Mid(4);		//以字符串形式取出串口的名字COMX
			m_StaticOpenComm = str;	//将取到的串口字符串名字赋值给文本控件
			flag = TRUE;
		}
		else
			{//不能打开将打开标示设成false
				if (flag == TRUE)
					flag = TRUE;
				else
					flag = FALSE;
			}
	}

	if (flag == TRUE)	//找到端口
	{
		CString str1;

		str1 = m_StaticOpenComm.GetAt(3);	//将串口字符串名字（COMX）中的数字X取出
		pnum = atoi(str1);	//转成十进制数值

//		m_StaticOpenComm = _T("Connect ") + m_StaticOpenComm + str1;
//		UpdateData(FALSE);
	}
	else	//没找到端口
		AfxMessageBox(_T("Can't find the comm port"));	
}

void CPCRProjectDlg::OnBnClickedBtnOpencomm()
{
	// TODO: Add your control notification handler code here

	m_StaticOpenComm="";
	GetCom();

	if (m_mscomm.get_PortOpen())  
		m_mscomm.put_PortOpen(FALSE); //关闭串口  

	m_mscomm.put_CommPort(pnum);     //设定串口为COM4  


	m_mscomm.put_Settings(_T("9600,n,8,1"));  //设定波特率9600，无奇偶校验，8位数据位，1位停止位  
	m_mscomm.put_InputMode(1);    //设定数据接收模式，1为二进制，0为文本  
	m_mscomm.put_InputLen(ONCOMNUM);     //设定当前接收区数据长度，0表示全部读取  
	m_mscomm.put_InBufferSize(1024);  //设定输入缓冲区大小为1024 byte  
	m_mscomm.put_OutBufferSize(1024); //设定输出缓冲区大小为1024 byte  
	m_mscomm.put_RThreshold(ONCOMNUM);   //每接收到59个字符时，触发OnComm事件  
	m_mscomm.put_SThreshold(0);   //设定每发送多少个字符触发OnComm事件，0表示不触发OnComm事件  

	if (!m_mscomm.get_PortOpen())  
	{
		m_mscomm.put_PortOpen(TRUE);  //打开串口 
		m_StaticOpenComm = _T("Connect the ") + m_StaticOpenComm;
		UpdateData(FALSE);
	}
	else  
		AfxMessageBox(_T("can't open the comm port"));
}


void CPCRProjectDlg::OnBnClickedBtnOpenhid()
{
	// TODO: Add your control notification handler code here
	if(CPCRProjectDlg::FindTheHID()) m_TrimDlg.ResetTrim();
}


void CPCRProjectDlg::OnBnClickedBtnSendhid()
{
	// TODO: Add your control notification handler code here
	ReadAndWriteToDevice();
}


void CPCRProjectDlg::OnBnClickedBtnReadhid()
{
	// TODO: Add your control notification handler code here
}


void CPCRProjectDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here

	//初始化读报告时用的Overlapped结构体
	//偏移量设置为0
	ReadOverlapped.Offset=0;
	ReadOverlapped.OffsetHigh=0;
	//创建一个事件，提供给ReadFile使用，当ReadFile完成时，
	//会设置该事件为触发状态。
	ReadOverlapped.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);

	//创建一个读报告的线程（处于挂起状态）
	pReadReportThread=AfxBeginThread(ReadReportThread,
		this,
		THREAD_PRIORITY_NORMAL,
		0,
		CREATE_SUSPENDED,
		NULL);
	//如果创建成功，则恢复该线程的运行
	if(pReadReportThread!=NULL)
	{
		pReadReportThread->ResumeThread();
	}
}


void CPCRProjectDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	switch(nIDEvent)
	{
	case 1:		// 持续video命令发送、数据读取
		{
			WriteHIDOutputReport();		// 向HID发送数据

			//命令标志、传输buffer清零
			GraFlag = 0;
			//				memset(TxData,0,sizeof(TxData));
			//				memset(GraBuf,0,sizeof(GraBuf));

			while(Gra_pageFlag)
				ReadHIDInputReport();
			Gra_pageFlag = true;

			break;
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}
