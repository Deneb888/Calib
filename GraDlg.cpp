// GraDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"
#include "GraDlg.h"
#include "afxdialogex.h"
#include <fstream>
#include "TrimReader.h"

using namespace std;

//*****************************************************************
//Constant definition
//*****************************************************************
// #define TxNum 50		// the number of the buffer for sent data to COMX
// #define RxNum 200		// the number of the buffer for received data from COMX

#define RowNum24 24		// display row number
#define ColNum24 24		// display column number
#define pixelsize24 10	// display size for one pixel
#define RowNum12 12		// display row number
#define ColNum12 12		// display column number
#define pixelsize12 20	// display size for one pixel

#define dprow12 0x01		// display one line with 12 pixel
#define dppage12 0x02		// display one page with 12 pixel
#define dpvideo12 0x03		// display video with 12 pixel
#define spiread	0x04		// only SPI read
#define adccvt	0x05		// only ADC conversion and SPI read
#define ndread	0x06		// non-destructive read
#define dprow24	0x07		// display one line with 24 pixel
#define dppage24 0x08		// display one page with 24 pixel
#define heatstr 0x09		// start auto-heating
#define heatstop 0x0a		// stop auto-heating
#define dpvideo24 0x0b		// display video with 24 pixel


//***************************************************************
//Global variable definition
//***************************************************************
int GraFlag = 0;			// graphic dialog flag
bool Gra_pageFlag = false;		// graphic dialog 画页循环标志
bool Gra_videoFlag = false;		// graphic dialog video循环标志
BYTE GraBuf[gradatanum];	// graphic dialog transmitted data buffer


//*****************************************************************
//External variable definition
//*****************************************************************
extern BYTE TxData[TxNum];		// the buffer of sent data to COMX
extern BYTE RxData[RxNum];		// the buffer of received data from COMX

extern CString RegRecStr;				//接收数据字符串buffer 十六进制
extern CString Dec_RegRecStr;			//接收数据字符串buffer 十进制
extern CString Valid_RegRecStr;			//有效接收数据字符串buffer 十六进制
extern CString Valid_Dec_RegRecStr;		//有效接收数据字符串buffer 十进制


//*****************************************************************
//External function
//*****************************************************************
extern unsigned char AsicConvert (unsigned char i, unsigned char j);		//ASIC convert to HEX
extern int ChangeNum (CString str, int length);								//字符串转10进制

extern float int_time; // zd add, in ms, this is the current int time, for read only.
extern int gain_mode;  // zd add, 0 high gain, 1 low gain
int adc_result[24];    // zd add

#define NUM_FPN  36

int fpn_data_l[NUM_FPN + 1][12];    // 24 + 1, 1 based index
int fpn_data_h[NUM_FPN + 1][12];    // 24 + 1, 1 based index

float fpn_avg_l[12];
float fpn_avg_h[12];

float fpn_rms_l[12];
float fpn_rms_h[12];

BYTE rdata[401][26];

float aerr, cerr;			// aerr is the signed average error. cerr is the RMS error.

extern BOOL g_DeviceDetected;

//=====HDR Support==========

//int pdata_0[24][24];	// first pass
//int pdata_1[24][24];	// second pass
//int hdr_phase = 0;

int contrast = 16;

//==========================

extern CTrimDlg *g_pTrimDlg;
// extern char g_TxBin;

unsigned int videoElapseTime = 100;
unsigned int autoRepeatCounter = 0;
unsigned int autoRepeatMaxCount = 400;	// 400 points fixed. 
// float max_int_time = 200;				// adjustable through GUI in CTrimDlg
unsigned int repeat_per_int = 2;		// 400 / 2 = 200 points

int searchCounter = 0;
int optrpg = 0x88;

int optv15 = 0x8;

//int searchMaxCount = 8;

float opt_int_time = 100;

int step = 0;		// 1: Rampgen sel rnd 1; 2: V15 sel; 3 Rampgen sel rnd 2; 4 ADC data sweep, and gen report;  
BOOL chain = false;		// chain multi-steps

CTrimReader g_TrimReader;

// BOOL subFPN = false;

BOOL kb_loaded = false;

extern CString g_ChipID;

// Dark management	/////////////// zd add

//	const char auto_v20_lg = 0x08;
//	const char auto_v20_hg = 0x0a;

// #define ADC_CORRECT
// #define DARK_MANAGE

#define DARK_LEVEL 100

//------------------

// CGraDlg dialog

IMPLEMENT_DYNAMIC(CGraDlg, CDialogEx)

CGraDlg::CGraDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGraDlg::IDD, pParent)
	, m_GainMode(0)
	, m_FrameSize(0)
	, m_OrigOut(TRUE)
	, m_subFPN(FALSE)
	, m_PixOut(FALSE)
{

	m_EditReadRow = _T("5");
	m_ShowAllDataInt = 1;
	m_ReadHexInt = 0;
	m_PixelData = _T("");
	m_ADCRecdata = _T("");

//	m_GainMode = 0;
//	m_FrameSize = 0;
//	m_PixOut = false;
//	m_OrigOut = false;

	for(int i=0; i<24; i++) {
		adc_result[i] = 0;
	}

	g_TrimReader.SetCalib2(1);			// disable dark subtraction.
}

CGraDlg::~CGraDlg()
{
}

void CGraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_READROW, m_EditReadRow);
	DDX_Control(pDX, IDC_RADIO_SHOWALLDATA, m_ShowAllData);
	DDX_Radio(pDX, IDC_RADIO_SHOWALLDATA, m_ShowAllDataInt);
	DDX_Control(pDX, IDC_RADIO_READHEX, m_ReadHex);
	DDX_Radio(pDX, IDC_RADIO_READHEX, m_ReadHexInt);
	DDX_Control(pDX, IDC_RADIO_READDEC, m_ReadDec);
	DDX_Control(pDX, IDC_RADIO_ADCDATA, m_ADCData);
	DDX_Control(pDX, IDC_RADIO_SHOWVALIDDATA, m_ShowValidData);
	DDX_Text(pDX, IDC_EDIT_RecData, m_PixelData);
	DDX_Text(pDX, IDC_EDIT_ADCDATA, m_ADCRecdata);
	DDX_Radio(pDX, IDC_RADIOLOWGAIN, m_GainMode);
	DDX_Radio(pDX, IDC_RADIO_12, m_FrameSize);
	DDX_Check(pDX, IDC_CHECK_PIXOUT, m_PixOut);
	DDX_Check(pDX, IDC_CHECK_ORIGOUT, m_OrigOut);
	DDX_Check(pDX, IDC_CHECK_SUBFPN, m_subFPN);
}


BEGIN_MESSAGE_MAP(CGraDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_READROW, &CGraDlg::OnClickedBtnReadrow)
	ON_MESSAGE(UM_GRAPROCESS,OnGraProcess)
	ON_BN_CLICKED(IDC_BTN_DPROW24, &CGraDlg::OnBnClickedBtnDprow24)
	ON_BN_CLICKED(IDC_BTN_DPPAGE12, &CGraDlg::OnBnClickedBtnDppage12)
	ON_BN_CLICKED(IDC_BTN_DPPAGE24, &CGraDlg::OnBnClickedBtnDppage24)
	ON_BN_CLICKED(IDC_BTN_DPVEDIO, &CGraDlg::OnBnClickedBtnDpvedio)
	ON_BN_CLICKED(IDC_BTN_STOPVIDEO, &CGraDlg::OnBnClickedBtnStopvideo)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CGraDlg::OnBnClickedBtnClear)
	ON_BN_CLICKED(IDC_BTN_ADCCONVERT, &CGraDlg::OnBnClickedBtnAdcconvert)
	ON_BN_CLICKED(IDC_BTN_CLEARADC, &CGraDlg::OnBnClickedBtnClearadc)
	ON_BN_CLICKED(IDC_BUTTON1, &CGraDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CGraDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BTN_DPVIDEO24, &CGraDlg::OnBnClickedBtnDpvideo24)
	ON_BN_CLICKED(IDC_RADIO_ADCDATA, &CGraDlg::OnBnClickedRadioAdcdata)
//	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER1, &CGraDlg::OnThumbposchangingSlider1)
//	ON_WM_HSCROLL()
//	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CGraDlg::OnNMCustomdrawSlider1)
ON_WM_HSCROLL()
ON_BN_CLICKED(IDC_RADIOLOWGAIN, &CGraDlg::OnClickedRadiolowgain)
//ON_UPDATE_COMMAND_UI(IDC_RADIOHDR, &CGraDlg::OnUpdateRadiohdr)
//ON_UPDATE_COMMAND_UI(IDC_RADIOHIGHGAIN, &CGraDlg::OnUpdateRadiohighgain)
ON_COMMAND(IDC_RADIOHIGHGAIN, &CGraDlg::OnRadiohighgain)
ON_COMMAND(IDC_RADIOHDR, &CGraDlg::OnRadiohdr)
ON_BN_CLICKED(IDC_RADIO_12, &CGraDlg::OnClickedRadio12)
ON_COMMAND(IDC_RADIO_24, &CGraDlg::OnRadio24)
ON_BN_CLICKED(IDC_BUTTON_CAPTURE, &CGraDlg::OnBnClickedButtonCapture)
ON_BN_CLICKED(IDC_CHECK_PIXOUT, &CGraDlg::OnClickedCheckPixout)
ON_BN_CLICKED(IDC_CHECK_ORIGOUT, &CGraDlg::OnClickedCheckOrigout)
ON_BN_CLICKED(IDC_BTN_READSPI, &CGraDlg::OnBnClickedBtnReadspi)
ON_WM_TIMER()
ON_BN_CLICKED(IDC_BUTTON_CPYCB, &CGraDlg::OnBnClickedButtonCpycb)
ON_BN_CLICKED(IDC_BUTTON_STEP1, &CGraDlg::OnBnClickedButtonStep1)
ON_BN_CLICKED(IDC_BUTTON_STEP2, &CGraDlg::OnBnClickedButtonStep2)
ON_BN_CLICKED(IDC_BUTTON_STEP3, &CGraDlg::OnBnClickedButtonStep3)
ON_BN_CLICKED(IDC_BUTTON_STEP4, &CGraDlg::OnBnClickedButtonStep4)
ON_BN_CLICKED(IDC_BUTTON_STEP5, &CGraDlg::OnBnClickedButtonStep5)
ON_BN_CLICKED(IDC_CHECK_SUBFPN, &CGraDlg::OnBnClickedCheckSubfpn)
ON_BN_CLICKED(IDC_BUTTON_STEP6, &CGraDlg::OnBnClickedButtonStep6)
ON_BN_CLICKED(IDC_BUTTON_STEP0, &CGraDlg::OnBnClickedButtonStep0)
END_MESSAGE_MAP()


//*********************************************************************
//Own function
//*********************************************************************

//调用主对话框对应消息处理函数
void CGraDlg::GraCalMainMsg()
{
	WPARAM a = 8;
	LPARAM b = 9;
	HWND hwnd = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	::SendMessage(hwnd,WM_GraDlg_event,a,b);
}

LRESULT CGraDlg::OnGraProcess(WPARAM wParam, LPARAM lParam)
{
	// 12 / 24 pixel distinguish
	BYTE type;
	type = RxData[4];	//从接收数据中取出type分支

	//数据处理及显示

	int pixelNum;

	// 根据返回的type区分是显示12列数据还是24列数据
	switch(type)
	{
	case dprow12:		// 12列画行
	case dppage12:		// 12列画页
	case dpvideo12:		// 12列画vedio
	case 0x0c:			// 积分时间超过10ms
		pixelNum = 12;
		break;

	case dprow24:		// 24列画行
	case dppage24:		// 24列画页
	case dpvideo24:		// 24列画vedio
		pixelNum = 24;
		break;

	default: break;
	}

	WORD * DecData = new WORD [pixelNum];
	int NumData = 0;
	int iDecVal;
	CString TmHex;
	CString sDecData;	// 最后显示的十进制字符串
	char sDecVal[10];
	BYTE lByte;
	CString sRownum;
	sRownum.Format(" %d",RxData[5]);

	// zd comment RxData[5] is the row number.
	int rn = RxData[5];

	for (NumData=0; NumData<pixelNum; NumData++)			//将每两个byte整合成一个word
	{
		lByte=RxData[NumData*2+6];				//取出低4位byte
		lByte = lByte<<4;						//将低4位byte左移4位
		DecData[NumData] = RxData[NumData*2+7];		//将高8位byte赋值给word变量
		DecData[NumData] <<= 8;						//word buffer左移8位，将高8位byte数据放置到高8位
		DecData[NumData] |= lByte;				//将低4位byte放到word buffer低8位
		DecData[NumData] >>= 4;						//将word buffer整体右移4位，变成有效12位数据
	}

	for (NumData=0; NumData<pixelNum; NumData++)			//将一行中的每个Pixel十六进制有效数据转成十进制
	{
		TmHex.Format("%2X",DecData[NumData]);		//将每个word buffer转成十六进制字符串
		iDecVal = ChangeNum(TmHex, TmHex.GetLength());		//将每个十六进制字符串转成有效十进制数
		gcvt(iDecVal,4,sDecVal);		//将十进制浮点数转换成字符串
		//第二个参数代表十进制数有多少位
		//第三个参数，必须为char *, 如上面对sDecVal的定义

		sDecData += sDecVal;				//将char* 赋值给CString，用来显示
		sDecData += "  ";					//每个数据间加空格
	}

	sDecData += sRownum;
	delete[] DecData;

	// 当返回命令为0x14时，只显示下述字符串
	if (RxData[2] == 0x14)
	{
		sDecData = "";
		sDecData.Format("the cycle number is %d",RxData[3]);
	}

//	int ioffset = 0; // ilbc, iresult;
	int /*lbc,*/ result;
//	int TemInt;
	char fstrbuf[9];
//	CString TemHex;
	CString sADCData;	// 最后显示的ADC数据字符串
//	int hb,lb;
//	int hbln,lbp,hbhn;
//	bool oflow,uflow;
//	BYTE LowByte,HighByte;
//	BYTE bTem;

//	CString soffset,slbc,shbln,slbp,shbhn,sresult;

	for (NumData = 0; NumData < pixelNum; NumData++)
	{
		int flag;

//		result = g_TrimReader.ADCCorrection(1, 0x0a, 0x91, 12, 1, gain_mode, &flag);

		result = g_TrimReader.ADCCorrection(NumData, RxData[NumData * 2 + 7], RxData[NumData * 2 + 6], pixelNum, 1, gain_mode, &flag);

		//高8位（1byte）数据处理，算offset(step1)、判断溢出(step2)、算result(step3)
//		HighByte = RxData[NumData * 2 + 7];
//		TemHex.Format("%2X", HighByte);
//		TemInt = ChangeNum(TemHex, TemHex.GetLength());
//		hb = TemInt;


#ifdef DARK_MANAGE

		if (!gain_mode) result += -(int)(FPN_hg[nd]) + DARK_LEVEL;
		else result += -(int)(FPN[nd]) + DARK_LEVEL;
#endif
		if (m_subFPN) {	
			
			int nd;
			if(pixelNum == 12) nd = NumData;
			else nd = NumData >> 1;

			if (!m_GainMode) {
				result += -(round)(fpn_avg_l[nd]) + DARK_LEVEL;
			}
			else {
				result += -(round)(fpn_avg_h[nd]) + DARK_LEVEL;
			}
		}

		if (result < 0) result = 0;

		adc_result[NumData] = result;		// 

		if (autoRepeatCounter > 0 && (step == 5 || step == 6)) {
			if (!m_GainMode) {
				fpn_data_l[autoRepeatCounter][NumData] = result;
			}
			else {
				fpn_data_h[autoRepeatCounter][NumData] = result;
			}
		}

		itoa(result, fstrbuf, 10);		//将结果转成字符串显示
		sADCData += fstrbuf;
		sADCData += " ";
	}

	sADCData += sRownum;

	// 当返回命令为0x14时，只显示下述字符串
	if (RxData[2] == 0x14)
	{
		sADCData = "";
		sADCData.Format("the cycle number is %d", RxData[3]);
	}

	if (m_OrigOut && m_ReadHex.GetCheck())			//以十六进制
	{
		if (m_ShowAllData.GetCheck())	//显示所有数据
		{
			//			m_PixelData = RegRecStr + m_PixelData;		//最新数据在编辑框第一行显示
			m_PixelData += (RegRecStr + "\r\n");					//最新数据在编辑框最后一行显示
			SetDlgItemText(IDC_EDIT_RecData, m_PixelData);

			//编辑框垂直滚动到底端
			POINT pt;
			GetDlgItem(IDC_EDIT_RecData)->GetScrollRange(SB_VERT, (LPINT)&pt.x, (LPINT)&pt.y);
			pt.x = 0;
			GetDlgItem(IDC_EDIT_RecData)->SendMessage(EM_LINESCROLL, pt.x, pt.y);
			RegRecStr = "";
		}

		if (m_ShowValidData.GetCheck())	//显示有效数据
		{
			//			m_PixelData = Valid_RegRecStr + m_PixelData;	//最新数据在编辑框第一行显示
			m_PixelData += (Valid_RegRecStr + "\r\n");					//最新数据在编辑框最后一行显示
			SetDlgItemText(IDC_EDIT_RecData, m_PixelData);

			//编辑框垂直滚动到底端
			POINT pt;
			GetDlgItem(IDC_EDIT_RecData)->GetScrollRange(SB_VERT, (LPINT)&pt.x, (LPINT)&pt.y);
			pt.x = 0;
			GetDlgItem(IDC_EDIT_RecData)->SendMessage(EM_LINESCROLL, pt.x, pt.y);
			Valid_RegRecStr = "";
		}
	}

	if (m_OrigOut && m_ReadDec.GetCheck())			//以十进制显示
	{
		//		m_PixelData = sTest + m_PixelData +"\r\n";			//每行数据将加回车
		//最新数据在编辑框第一行显示
		m_PixelData += (sDecData + "\r\n");
		SetDlgItemText(IDC_EDIT_RecData, m_PixelData);		//每行数据将加回车
		//最新数据在编辑框最后一行显示

		//编辑框垂直滚动到底端
		POINT pt;
		GetDlgItem(IDC_EDIT_RecData)->GetScrollRange(SB_VERT, (LPINT)&pt.x, (LPINT)&pt.y);
		pt.x = 0;
		GetDlgItem(IDC_EDIT_RecData)->SendMessage(EM_LINESCROLL, pt.x, pt.y);
	}

	if (m_PixOut && m_GainMode <= 1)		//ADC数据显示
	{
		m_ADCRecdata += (sADCData + "\r\n");
		SetDlgItemText(IDC_EDIT_ADCDATA, m_ADCRecdata);		//每行数据将加回车
		//最新数据在编辑框最后一行显示

		//编辑框垂直滚动到底端
		POINT pt;
		GetDlgItem(IDC_EDIT_ADCDATA)->GetScrollRange(SB_VERT, (LPINT)&pt.x, (LPINT)&pt.y);
		pt.x = 0;
		GetDlgItem(IDC_EDIT_ADCDATA)->SendMessage(EM_LINESCROLL, pt.x, pt.y);

	}

	// 2018 addition - save data for further analysis

	for (int k = 0; k < 26; k++) {
		rdata[autoRepeatCounter][k] = RxData[k + 6];
	}

	DisplayPattern();

	return 0;
}


//=======================================
//		Graphics draw 
//=======================================

// - RxData will be cleared here after use

void CGraDlg::DisplayPattern(void)
{
	BYTE type = RxData[4];

	CDC		*pDC;		//创建目标DC指针
	pDC = GetDlgItem(IDC_Bmp)->GetDC();

	CRect   rect;
	CBrush	brush[RowNum24][ColNum24];
	int		i,l;
	int		gray_level;

	for (i=0; i<ColNum24; i++)
	{
		gray_level = adc_result[i]/contrast;
		if(gray_level > 255) gray_level = 255;
		else if(gray_level < 0) gray_level = 0;

		brush[RxData[5]][i].CreateSolidBrush(RGB(gray_level, gray_level, gray_level));		// zd mod use corrected adc data with green tint
	}

	switch(type)
	{
	case dprow12:
		{

			CBitmap bmp;
			bmp.CreateCompatibleBitmap(pDC,RowNum12*pixelsize12,ColNum12*pixelsize12);

			CDC bDC; 
			bDC.CreateCompatibleDC(pDC);

			bDC.SelectObject(&bmp);


			for(l=0; l<ColNum12; l++)		// l代表列号；rxdata[5]中数据是行号
			{
				rect.SetRect(pixelsize12*l,pixelsize12*RxData[5],pixelsize12*(l+1),pixelsize12*(RxData[5]+1));
				bDC.Rectangle(rect);
				bDC.FillRect(&rect,&brush[RxData[5]][l]);
			}
			pDC->BitBlt(0,0,900, 380, &bDC, 0, 0, SRCCOPY);

			//数据传输buffer清零
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case dppage12:
		{
			for(l=0; l<ColNum12; l++)		// l代表列号
			{
				rect.SetRect(pixelsize12*l,pixelsize12*RxData[5],pixelsize12*(l+1),pixelsize12*(RxData[5]+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[RxData[5]][l]);
			}

			//数据传输buffer清零
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case 0x0c:		// 积分时间超过10ms
		{
			for(l=0; l<ColNum12; l++)		// l代表列号
			{
				rect.SetRect(pixelsize12*l,pixelsize12*RxData[5],pixelsize12*(l+1),pixelsize12*(RxData[5]+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[RxData[5]][l]);
			}

			//数据传输buffer清零
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case dprow24:
		{

			CBitmap bmp;
			bmp.CreateCompatibleBitmap(pDC,RowNum24*pixelsize24,ColNum24*pixelsize24);

			CDC bDC;  
			bDC.CreateCompatibleDC(pDC);

			bDC.SelectObject(&bmp);


			for(l=0; l<ColNum24; l++)
			{
				rect.SetRect(pixelsize24*l,pixelsize24*RxData[5],pixelsize24*(l+1),pixelsize24*(RxData[5]+1));
				bDC.Rectangle(rect);
				bDC.FillRect(&rect,&brush[RxData[5]][l]);
			}
			pDC->BitBlt(0,0,900, 380, &bDC, 0, 0, SRCCOPY);

			//数据传输buffer清零
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case dppage24:
		{
			for(l=0; l<ColNum24; l++)		// l代表列号
			{
				rect.SetRect(pixelsize24*l,pixelsize24*RxData[5],pixelsize24*(l+1),pixelsize24*(RxData[5]+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[RxData[5]][l]);
			}

			//数据传输buffer清零
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case dpvideo12:	//video12
		{
			for(l=0; l<ColNum12; l++)		// l代表列号
			{
				rect.SetRect(pixelsize12*l,pixelsize12*RxData[5],pixelsize12*(l+1),pixelsize12*(RxData[5]+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[RxData[5]][l]);
			}

			//数据传输buffer清零
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case dpvideo24:	//video24
		{
			for(l=0; l<ColNum24; l++)		// l代表列号
			{
				rect.SetRect(pixelsize24*l,pixelsize24*RxData[5],pixelsize24*(l+1),pixelsize24*(RxData[5]+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[RxData[5]][l]);
			}

			//数据传输buffer清零
			memset(RxData,0,sizeof(RxData));

			break;
		}

	default:
		break;

	}


}



// CGraDlg message handlers

void CGraDlg::OnClickedBtnReadrow()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	if(!m_GainMode) {
		SetGainMode(1);
//		if (m_subFPN) 
			g_pTrimDlg->SetV20(0x8);
	}
	else {
		SetGainMode(0);
//		if (m_subFPN) 
			g_pTrimDlg->SetV20(0xa);

	}

	GraFlag = sendgramsg;

	CString sRownum;
	GetDlgItemText(IDC_EDIT_READROW,sRownum);
	GraBuf[0] = atoi(sRownum);

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x01;		//data type, date edit first byte
	TxData[4] = GraBuf[0];		//real data
	TxData[5] = 0x00;		//预留位
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code


	//Send message to main dialog
	GraCalMainMsg();		//调用主对话框串口发送消息程序

}

void CGraDlg::OnBnClickedBtnDprow24()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	if (!m_GainMode) {
		SetGainMode(1);
		//		if (m_subFPN) 
		g_pTrimDlg->SetV20(0x8);
	}
	else {
		SetGainMode(0);
		//		if (m_subFPN) 
		g_pTrimDlg->SetV20(0xa);

	}

	GraFlag = sendgramsg;

	CString sRownum;
	GetDlgItemText(IDC_EDIT_READROW,sRownum);
	GraBuf[0] = atoi(sRownum);

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x07;		//data type, date edit first byte
	TxData[4] = GraBuf[0];		//real data
	TxData[5] = 0x00;		//预留位
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code


	//Send message to main dialog
	GraCalMainMsg();		//调用主对话框串口发送消息程序
}


void CGraDlg::OnBnClickedBtnDppage12()
{
	// TODO: Add your control notification handler code here
}


void CGraDlg::OnBnClickedBtnDppage24()
{
	// TODO: Add your control notification handler code here

	CString fn = "trim-";
	fn += g_ChipID;
	fn += ".dat";

	CFileDialog loadDlg(TRUE, ".dat", fn,
		OFN_HIDEREADONLY,
		"Data file (*.dat)|*.dat|");

	int e = 0;

	if (loadDlg.DoModal() == IDOK)
	{
		fn = loadDlg.GetPathName();
		LPTSTR lpszData = fn.GetBuffer(fn.GetLength());
		e = g_TrimReader.Load((TCHAR*)lpszData);
		fn.ReleaseBuffer(0);
	}

	//	delete[] lpszData;// don't forget to do this.

	if (e) {
		g_TrimReader.Parse();
		DebugLog("trim loaded...");
		g_TrimReader.CloseFile();
	}
	else {
		MessageBox("Trim File not found");
	}
}

//Start Video

// Repurposed to auto calib start

void CGraDlg::OnBnClickedBtnDpvedio()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	if (!m_OrigOut) {
		m_OrigOut = true;
		((CButton*)GetDlgItem(IDC_CHECK_ORIGOUT))->SetCheck(true);
	}

	if (m_PixOut) {
		m_PixOut = false;
		((CButton*)GetDlgItem(IDC_CHECK_PIXOUT))->SetCheck(false);
	}

	m_PixelData = "";
	SetDlgItemText(IDC_EDIT_RecData,m_PixelData);
						
//	int_time = 1.0;	
	if(g_pTrimDlg) g_pTrimDlg->SetIntTime(1.0);

	autoRepeatCounter = 0;

	//==== chainning =====

	chain = true;
	step = 0;

	//====================

	SetTimer(1,videoElapseTime,NULL);		// 
}

/*

On click step 1

step = 1;

set int time to 1, 
clear buffer

set timer


*/

// Repurposed to Auto Calib 3

void CGraDlg::OnBnClickedBtnDpvideo24()
{
	// TODO: Add your control notification handler code here
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	if(!kb_loaded) {
		MessageBox("KB Matrix not loaded yet");
		return;
	}

	if (m_subFPN) {
		m_subFPN = FALSE;
		((CButton*)GetDlgItem(IDC_CHECK_SUBFPN))->SetCheck(false);
	}

	// int_time = 1.0;
	if (g_pTrimDlg) {
		g_pTrimDlg->SetIntTime(3.0);
	}

	OnBnClickedButtonCapture();			// Precapture to clear buffer

	if (g_pTrimDlg) {
		g_pTrimDlg->SetIntTime(1.0);
		g_pTrimDlg->SetTXbin(0);
		g_pTrimDlg->m_ComboxTX.SetCurSel(0);
	}

	m_OrigOut = false;
	m_PixOut = true;
	((CButton*)GetDlgItem(IDC_CHECK_ORIGOUT))->SetCheck(false);
	((CButton*)GetDlgItem(IDC_CHECK_PIXOUT))->SetCheck(true);


	SetGainMode(1);		// low gain
	m_GainMode = 0;		// low gain

	((CButton*)GetDlgItem(IDC_RADIOLOWGAIN))->SetCheck(true);
	((CButton*)GetDlgItem(IDC_RADIOHIGHGAIN))->SetCheck(false);

	g_pTrimDlg->SetV20(0x8);

	chain = true;
	step = 5;

	//====================

	SetTimer(1, videoElapseTime, NULL);		// 
}


// Stop Video
// repurposed to stop auto calib

void CGraDlg::OnBnClickedBtnStopvideo()
{
	// TODO: Add your control notification handler code here
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	KillTimer(1);		// stop video

	chain = 0;
	step = 0;

	autoRepeatCounter = 0;
}


void CGraDlg::OnBnClickedBtnClear()
{
	// TODO: Add your control notification handler code here

	m_PixelData = "";
	SetDlgItemText(IDC_EDIT_RecData,m_PixelData);
}


void CGraDlg::OnBnClickedBtnAdcconvert()
{
	// TODO: Add your control notification handler code here
	g_TrimReader.WriteTrimData();

	CString fn = "trim-";
	fn += g_ChipID;
	fn += ".dat";

	CFileDialog saveDlg(FALSE, ".dat", fn, 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Data file (*.dat)|*.dat|",
		NULL, 0, TRUE);

	if (saveDlg.DoModal() == IDOK)
	{
		ofstream ofs(saveDlg.GetPathName());
		ofs << g_TrimReader.TrimDotData;
	
		DebugLog("Trim.dat written...");
		DebugLog(g_TrimReader.TrimDotData);
	}
}


void CGraDlg::OnBnClickedBtnClearadc()
{
	// TODO: Add your control notification handler code here

	m_ADCRecdata ="";
	SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);
}


void CGraDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CString str;
	// 创建另存对话框
	CFileDialog saveDlg(FALSE,".txt",
		NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text (*.txt)|*.txt|""Word doc (*.doc)|*.doc|""Excel doc (*.xls)|*.xls|""All Files(*.*)|*.*||",
		NULL,0,TRUE);

	//	saveDlg.m_ofn.lpstrInitialDir = "c:\\";		// 另存对话框默认路径为c盘

	// 进行保存动作
	if (saveDlg.DoModal() == IDOK)
	{
		ofstream ofs(saveDlg.GetPathName());
		CStatic * pst = (CStatic*)GetDlgItem(IDC_EDIT_RecData);	// 获取要保存编辑框控件内的数据
		// IDC_EDIT_FILE是编辑框控件句柄
		pst->GetWindowTextA(str);
		ofs << str;
	}
}

void CGraDlg::SaveRecData()
{
	// TODO: Add your control notification handler code here
	CString str = "recdata-";

	str += g_ChipID;
	str += ".txt";

//	if (1)
//	{
		ofstream ofs(str);
		CStatic * pst = (CStatic*)GetDlgItem(IDC_EDIT_RecData);	// 获取要保存编辑框控件内的数据
																// IDC_EDIT_FILE是编辑框控件句柄
		pst->GetWindowTextA(str);
		ofs << str;
//	}
}


void CGraDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here

	CString str;
	// 创建另存对话框
	CFileDialog saveDlg(FALSE,".txt",
		NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text(*.txt)|*.txt|""Word doc(*.doc)|*.doc|""All Files(*.*)|*.*||",
		NULL,0,TRUE);

	//	saveDlg.m_ofn.lpstrInitialDir = "c:\\";		// 另存对话框默认路径为c盘

	// 进行保存动作
	if (saveDlg.DoModal() == IDOK)
	{
		ofstream ofs(saveDlg.GetPathName());
		CStatic * pst = (CStatic*)GetDlgItem(IDC_EDIT_ADCDATA);	// 获取要保存编辑框控件内的数据
		// IDC_EDIT_FILE是编辑框控件句柄
		pst->GetWindowTextA(str);
		ofs << str;
	}
}



void CGraDlg::OnBnClickedRadioAdcdata()
{
	// TODO: Add your control notification handler code here
}



void CGraDlg::SetGainMode(int gain)
{
	GraFlag = sendgramsg;

	if (!gain)
	{
		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x01;		//command
		TxData[2] = 0x02;		//data length
		TxData[3] = 0x07;		//data type, date edit first byte
		TxData[4] = 0x00;		//real data, date edit second byte
		//0x01 means send vedio data
		//0x00 means stop vedio data
		TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
		if (TxData[5]==0x17)
			TxData[5]=0x18;
		else
			TxData[5]=TxData[5];
		TxData[6] = 0x17;		//back code
		TxData[7] = 0x17;		//back code

		//Call maindlg message
//		GraCalMainMsg();
		gain_mode = 0;
	}
	else
	{
			TxData[0] = 0xaa;		//preamble code
			TxData[1] = 0x01;		//command
			TxData[2] = 0x02;		//data length
			TxData[3] = 0x07;		//data type, date edit first byte
			TxData[4] = 0x01;		//real data, date edit second byte
			//0x01 means send vedio data
			//0x00 means stop vedio data
			TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
			if (TxData[5]==0x17)
				TxData[5]=0x18;
			else
				TxData[5]=TxData[5];
			TxData[6] = 0x17;		//back code
			TxData[7] = 0x17;		//back code

			//Call maindlg message
//			GraCalMainMsg();
			gain_mode = 1;
	}

	GraCalMainMsg();		//调用主对话框串口发送消息程序

#ifdef DARK_MANAGE

	if(g_pTrimDlg) {
		if(!gain) g_pTrimDlg->SetV20(auto_v20_hg);
		else g_pTrimDlg->SetV20(auto_v20_lg);
	}

#endif

}


void CGraDlg::DisplayHDR(void)
{
}


void CGraDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

//	if(m_GainMode <= 1) return; // only works for HDR mode.

	int pos;

/*	CSliderCtrl* pSlider = reinterpret_cast<CSliderCtrl*>(pScrollBar);  

    // Check which slider sent the notification  
    if (pSlider == &c_Slider1)  
    {  
    }
    else if (pSlider == &c_Slider2)  
    {  
    }  
*/
    // Check what happened  
    switch(nSBCode)
    {
    case TB_LINEUP:  
    case TB_LINEDOWN:  
    case TB_PAGEUP:  
    case TB_PAGEDOWN:  
    case TB_THUMBPOSITION: 
		pos = nPos;
		contrast = 16 - pos / 7;
		if(!m_FrameSize)
			DisplayHDR();
		else
			DisplayHDR24();
		break;
    case TB_TOP:  
    case TB_BOTTOM:  
    case TB_THUMBTRACK:  
    case TB_ENDTRACK:  
    default:  
        break;  
    }


	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CGraDlg::DisplayHDR24(void)
{
}


void CGraDlg::OnClickedRadiolowgain()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
}


void CGraDlg::OnRadiohighgain()
{
	// TODO: Add your command handler code here
		UpdateData(true);
}


void CGraDlg::OnRadiohdr()
{
	// TODO: Add your command handler code here
		UpdateData(true);
}


void CGraDlg::OnClickedRadio12()
{
	// TODO: Add your control notification handler code here
			UpdateData(true);
}


void CGraDlg::OnRadio24()
{
	// TODO: Add your command handler code here
			UpdateData(true);
}


void CGraDlg::OnBnClickedButtonCapture()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

//	m_PixelData = "";
//	SetDlgItemText(IDC_EDIT_RecData,m_PixelData);
//	OnBnClickedBtnClearadc();

	if(!m_FrameSize) {
		if(m_GainMode <= 1) {
			CaptureFrame12();
		}
		else {
			OnBnClickedBtnDppage12();		// HDR mode not supported
		}
	}
	else {
		if(m_GainMode <= 1) {
			CaptureFrame24();
		}
		else {
			OnBnClickedBtnDppage24();		// HDR mode not supported
		}

		if(g_pTrimDlg) g_pTrimDlg->ResetTxBin();	// after coming back to 12X12 mode, bin will be 0x8.
	}
}


void CGraDlg::CaptureFrame12(void)
{
	if(!m_GainMode) {
		SetGainMode(1);
//		if (m_subFPN) 
			g_pTrimDlg->SetV20(0x8);
	}
	else {
		SetGainMode(0);
//		if (m_subFPN) 
			g_pTrimDlg->SetV20(0xa);
	}

	GraFlag = sendpagemsg;
	Gra_pageFlag = TRUE;	// 开始循环

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x02;		//data type, date edit first byte
	TxData[4] = 0xff;		//real data
	TxData[5] = 0x00;		//预留位
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code

	//Send message to main dialog
	GraCalMainMsg();		//调用主对话框串口发送消息程序
}


void CGraDlg::CaptureFrame24(void)
{
	if(!m_GainMode) {
		SetGainMode(1);
//		if (m_subFPN) 
			g_pTrimDlg->SetV20(0x8);
	}
	else {
		SetGainMode(0);
//		if (m_subFPN) 
			g_pTrimDlg->SetV20(0xa);
	}

	if(g_pTrimDlg) g_pTrimDlg->ResetTxBin();

	GraFlag = sendpagemsg;
	Gra_pageFlag = TRUE;	// 开始循环

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x08;		//data type, date edit first byte
	TxData[4] = 0xff;		//real data
	TxData[5] = 0x00;		//预留位
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code

	//Send message to main dialog
	GraCalMainMsg();		//调用主对话框串口发送消息程序
}


void CGraDlg::OnClickedCheckPixout()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
}


void CGraDlg::OnClickedCheckOrigout()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
}


// Repurposed to load KB matrix

void CGraDlg::OnBnClickedBtnReadspi()
{
	// TODO: Add your control notification handler code here

	CString path = "kbmat-";
//	path = g_CurrentDirectory;
	path += g_ChipID;
	path += ".dat";

	LPTSTR lpszData = path.GetBuffer(path.GetLength());
	int e = g_TrimReader.Load((TCHAR*)lpszData);

	path.ReleaseBuffer(0);

	//	delete[] lpszData;// don't forget to do this.

	if (e) {
		g_TrimReader.JustParseMatrix();
		DebugLog("KB matrix loaded...");
		g_TrimReader.CloseFile();
		kb_loaded = true;
	}
	else {
		MessageBox("KB Matrix File not found");
	}
}


void CGraDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	switch(nIDEvent)
	{
	case 1:
		TimerISR();
		break;	
	default:
		break;
	}		// end of switch (nIDEvent)

	CDialogEx::OnTimer(nIDEvent);
}

void CGraDlg::TimerISR()
{
	int done = 0;

	switch (step)
	{
	case 0:
		done = Step0();
		break;

	case 1:
		done = Step1();
		break;

	case 2:
		done = Step2();
		break;

	case 3:
		done = Step3();
		break;

	case 4:
		done = Step4();
		break;

	case 5:
		done = Step5();
		break;

	case 6:
		done = Step6();
		break;

	default:
		break;
	}


	if (done) {
		if (!chain) {
			KillTimer(1);
		}
		else {
			if (step == 4 || step == 6) {		// end of chain
				KillTimer(1);
			}
			else {
				step++;

				if (step == 1) {
					optv15 = 0xf;				// start high to low
					g_pTrimDlg->SetV15(optv15);
					g_pTrimDlg->m_ComboxV15.SetCurSel((int)optv15);
				}
				else if (step == 2) {
					opt_int_time = 10;
					if (g_pTrimDlg) g_pTrimDlg->SetIntTime(opt_int_time);
				}
				else if (step == 3) {
//					int_time = 1.0;
					if (g_pTrimDlg) 
						g_pTrimDlg->SetIntTime(1.0);

					SetMaxRepeatCount(80);
					searchCounter = 0;
				}
				else if (step == 4) {
					if (g_pTrimDlg)
						g_pTrimDlg->SetIntTime(1.0);

					SetMaxRepeatCount(400);
				}
				else if (step == 6) {			// int time and bin already set by this point
					SetGainMode(0);		// high gain
					m_GainMode = 1;		// high gain

					((CButton*)GetDlgItem(IDC_RADIOLOWGAIN))->SetCheck(false);
					((CButton*)GetDlgItem(IDC_RADIOHIGHGAIN))->SetCheck(true);

					g_pTrimDlg->SetV20(0xa);
				}

				m_PixelData = "";
				SetDlgItemText(IDC_EDIT_RecData, m_PixelData);
			}
		}
	}
}

void CGraDlg::OnBnClickedButtonCpycb()
{
	// TODO: Add your control notification handler code here
	CEdit *myEdit;

	myEdit = (CEdit*)GetDlgItem(IDC_EDIT_RecData);

	myEdit->SetSel(0, -1, FALSE);
	myEdit->Copy();
}

// Called by step3

void CGraDlg::CalcErrs()			
{
	BYTE hb, lb, mod;
	int diff;

	float sum[12];
	float avg = 0;
	cerr = 0;

	for(int j=0; j<12; j++) {
		sum[j] = 0;
		for(unsigned int i=1; i<=autoRepeatMaxCount; i++) {
			lb = rdata[i][2*j];
			hb = rdata[i][2*j + 1];
			mod = hb % 16;
			diff = (int)(mod * 16 + 7 - lb);

			avg += (float)diff;
			sum[j] += (float)diff * (float)diff;
		}
		cerr += sqrt(sum[j]/autoRepeatMaxCount);
	}
	aerr = avg / autoRepeatMaxCount;
}

// pre-select rampgen to make the test adc high byte to 0x7f

int CGraDlg::Step0()
{
	int done = 0;

	autoRepeatCounter++;
	OnClickedBtnReadrow();						// read a row

	int thb = rdata[autoRepeatCounter][25];		// cast to int, high byte of the test ADC output.

	int inc = (thb - 0x7f) / 2;

	if (abs(inc) < 1)
		done = 1;
	else
		optrpg += inc;

	if (optrpg < 10) optrpg = 10;
	else if (optrpg > 250) optrpg = 250;

	g_pTrimDlg->SetRampgen(optrpg);
	g_pTrimDlg->m_ComboxRampgen.SetCurSel((int)optrpg);

	if (autoRepeatCounter >= 20) {
		done = 1;
	}

	if (done) {
		autoRepeatCounter = 0;

		CString str;
		str.Format("TestADC HighByte: 0x%x, Rampgen: 0x%x", thb, optrpg);
//		MessageBox(str);
		DebugLog(str);

	}

	return done;
}

// select v15 to make the minimum high byte to 0x5

int CGraDlg::Step1()
{
	int done = 0;

	autoRepeatCounter++;
	OnClickedBtnReadrow();						// read a row

	int mhb = MinHighByte(autoRepeatCounter);		// cast to int

	int inc;
	
	if(!m_GainMode) {
		inc = (0x05 - mhb) / 2;			// low gain
	}
	else {
		inc = (0x06 - mhb) / 2;			// high gain
	}

	if (abs(inc) < 1)
		done = 1;
	else
		optv15 += inc;

	if (optv15 < 3) optv15 = 3;
	else if (optv15 > 14) optv15 = 14;

	g_pTrimDlg->SetV15(optv15);
	g_pTrimDlg->m_ComboxV15.SetCurSel((int)optv15);

	if (autoRepeatCounter >= 12) {
		done = 1;
	}

	if (done) {
		autoRepeatCounter = 0;

		CString str;
		str.Format("Min HighByte: 0x%x, Optimal v15: 0x%x", mhb, optv15);
		DebugLog(str);

		if (mhb >= 0x13) {
			g_pTrimDlg->SetRange(0x03);
			g_pTrimDlg->m_ComboxRange.SetCurSel(0x03);

			str.Format("Range adjusted to: 0x%x", 0x03);
			DebugLog(str);
		}
		else if (mhb >= 0x10) {
			g_pTrimDlg->SetRange(0x08);
			g_pTrimDlg->m_ComboxRange.SetCurSel(0x08);

			str.Format("Range adjusted to: 0x%x", 0x08);
			DebugLog(str);
		}
		else if (mhb >= 0x0c) {
			g_pTrimDlg->SetRange(0x0c);
			g_pTrimDlg->m_ComboxRange.SetCurSel(0x0c);

			str.Format("Range adjusted to: 0x%x", 0x0c);
			DebugLog(str);
		}

//		g_TrimReader.Node[0].auto_v15 = optv15;

	}

	return done;
}

// Refine and select optimal rampgen

int CGraDlg::Step3()
{
	int done = 0;

	autoRepeatCounter++;
	OnClickedBtnReadrow();

	if (autoRepeatCounter % repeat_per_int == 0) {

		int_time += (opt_int_time * repeat_per_int) / autoRepeatMaxCount;
		if (g_pTrimDlg) 
			g_pTrimDlg->SetIntTime(int_time);
	}

	if (autoRepeatCounter >= autoRepeatMaxCount) {
		searchCounter++;
		CalcErrs();

		autoRepeatCounter = 0;
		int_time = 1.0;
		if (g_pTrimDlg) 
			g_pTrimDlg->SetIntTime(int_time);

		// search for better rampgen value

		int inc = (int)(aerr / 30);

		if (abs(inc) < 1)
			searchCounter = 8;
		else
			optrpg += inc;

		if (optrpg < 10) optrpg = 10;
		else if (optrpg > 250) optrpg = 250;

		g_pTrimDlg->SetRampgen(optrpg);
		g_pTrimDlg->m_ComboxRampgen.SetCurSel((int)optrpg);

		if (searchCounter >= 8) {
			done = 1;
		}
	}

	if (done) {
			CString str;
			str.Format("Code error: %.2f, Signed error: %.2f, Rampgen: %x", cerr, aerr, optrpg);
			DebugLog(str);

//			g_TrimReader.Node[0].rampgen = optrpg;
			g_TrimReader.Node[0].auto_v20[0] = 0x08;	// V20 is always auto set by GraDlg. 
			g_TrimReader.Node[0].auto_v20[1] = 0x0a;
//			g_TrimReader.Node[0].range = 0x0f;			// range set by ui input, assigned in TrimDlg.
	}

	return done;
}

int CGraDlg::Step4()
{
	int done = 0;

	autoRepeatCounter++;
	OnClickedBtnReadrow();

	if (autoRepeatCounter % repeat_per_int == 0) {
		int_time += (opt_int_time * repeat_per_int) / autoRepeatMaxCount;
		if (g_pTrimDlg) g_pTrimDlg->SetIntTime(int_time);
	}

	if (autoRepeatCounter >= autoRepeatMaxCount) {
		autoRepeatCounter = 0;		
		done = 1;
	}

	if (done) {
			CString str;
			str.Format("Finished sweeping through ADC: Total count %d", autoRepeatMaxCount);
			DebugLog(str);

			SaveRecData();
	}

	return done;
}

int CGraDlg::MinHighByte(int i)
{
	int min = rdata[i][1];

	for (int k = 1; k < 12; k++) {
		int hb = rdata[i][2*k+1];
		if (hb < min) min = hb;
	}

	return min;
}

int CGraDlg::MaxHighByte(int i)
{
	int max = rdata[i][1];

	for (int k = 0; k < 12; k++) {
		int hb = rdata[i][2*k+1];
		if (hb > max) max = hb;
	}

	return max;
}

void CGraDlg::OnBnClickedButtonStep1()
{
	// TODO: Add your control notification handler code here
	if (!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	m_PixelData = "";
	SetDlgItemText(IDC_EDIT_RecData, m_PixelData);

	int_time = 1.0;
	if (g_pTrimDlg) g_pTrimDlg->SetIntTime(int_time);

	optv15 = 0xf;				// start high to low
	g_pTrimDlg->SetV15(optv15);
	g_pTrimDlg->m_ComboxV15.SetCurSel((int)optv15);

	chain = false;
	step = 1;

	//====================

	SetTimer(1, videoElapseTime, NULL);		// 
}

void CGraDlg::OnBnClickedButtonStep2()
{
	// TODO: Add your control notification handler code here
	if (!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	m_PixelData = "";
	SetDlgItemText(IDC_EDIT_RecData, m_PixelData);

	opt_int_time = 10;
	if (g_pTrimDlg) g_pTrimDlg->SetIntTime(opt_int_time);

	chain = false;
	step = 2;

	//====================

	SetTimer(1, videoElapseTime, NULL);		// 
}

void CGraDlg::OnBnClickedButtonStep3()
{
	// TODO: Add your control notification handler code here
	if (!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	m_PixelData = "";
	SetDlgItemText(IDC_EDIT_RecData, m_PixelData);

	int_time = 1.0;
	if (g_pTrimDlg) g_pTrimDlg->SetIntTime(int_time);

	SetMaxRepeatCount(80);
	searchCounter = 0;

	chain = false;
	step = 3;

	//====================

	SetTimer(1, videoElapseTime, NULL);		// 
}

void CGraDlg::OnBnClickedButtonStep4()
{
	// TODO: Add your control notification handler code here
	if (!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	m_PixelData = "";
	SetDlgItemText(IDC_EDIT_RecData, m_PixelData);

	SetMaxRepeatCount(400);

	int_time = 1.0;
	if (g_pTrimDlg) g_pTrimDlg->SetIntTime(int_time);

	chain = false;
	step = 4;

	//====================

	SetTimer(1, videoElapseTime, NULL);		// 
}


void CGraDlg::OnBnClickedButtonStep5()
{
	// TODO: Add your control notification handler code here
	if (!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

//	int_time = 1.0;
	if (g_pTrimDlg) {
		g_pTrimDlg->SetIntTime(1.0);
		g_pTrimDlg->SetTXbin(0);
		g_pTrimDlg->m_ComboxTX.SetCurSel(0);
	}

	SetGainMode(1);		// low gain
	m_GainMode = 0;		// low gain

	((CButton*)GetDlgItem(IDC_RADIOLOWGAIN))->SetCheck(true);
	((CButton*)GetDlgItem(IDC_RADIOHIGHGAIN))->SetCheck(false);

	g_pTrimDlg->SetV20(0x8);

	chain = false;
	step = 5;

	//====================

	SetTimer(1, videoElapseTime, NULL);		// 
}


void CGraDlg::OnBnClickedButtonStep6()
{
	// TODO: Add your control notification handler code here
	if (!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

//	int_time = 1.0;
	if (g_pTrimDlg) {
		g_pTrimDlg->SetIntTime(1.0);
		g_pTrimDlg->SetTXbin(0);
		g_pTrimDlg->m_ComboxTX.SetCurSel(0);
	}

	SetGainMode(0);		// high gain
	m_GainMode = 1;		// high gain

	((CButton*)GetDlgItem(IDC_RADIOLOWGAIN))->SetCheck(false);
	((CButton*)GetDlgItem(IDC_RADIOHIGHGAIN))->SetCheck(true);

	g_pTrimDlg->SetV20(0xa);

	chain = false;
	step = 6;

	//====================

	SetTimer(1, videoElapseTime, NULL);		// 
}



void CGraDlg::DebugLog(CString str)
{
	m_dLog += (str + "\r\n");			// 
	SetDlgItemTextA(IDC_EDIT_DLOG, m_dLog);		// 

	POINT pt;
	GetDlgItem(IDC_EDIT_DLOG)->GetScrollRange(SB_VERT, (LPINT)&pt.x, (LPINT)&pt.y);
	pt.x = 0;
	GetDlgItem(IDC_EDIT_DLOG)->SendMessage(EM_LINESCROLL, pt.x, pt.y);
}

void CGraDlg::SetMaxRepeatCount(int cnt)
{
	autoRepeatMaxCount = cnt;	//  

	if (cnt > (int)opt_int_time) {
		repeat_per_int = 2;
	}
	else {
		repeat_per_int = 1;
	}
}


int CGraDlg::Step5()
{
	int done = 0;

	autoRepeatCounter++;
	OnClickedBtnReadrow();						// read a row

	if (autoRepeatCounter >= NUM_FPN) {
		done = 1;
	}

	if (done) {
		autoRepeatCounter = 0;

		CalcFPNL();

		CString str, sub;
		str.Format("FPN estimation low gain");
		DebugLog(str);

		str.Format("Rms: ");
		for(int i=0; i<12; i++) {
			sub.Format("%.1f, ", fpn_rms_l[i]);
			str.Append(sub);
		}
		DebugLog(str);

//		subFPN = true;
	}

	return done;
}

int CGraDlg::Step6()
{
	int done = 0;

	autoRepeatCounter++;
	OnClickedBtnReadrow();						// read a row

	if (autoRepeatCounter >= NUM_FPN) {
		done = 1;
	}

	if (done) {
		autoRepeatCounter = 0;

		CalcFPNH();

		CString str, sub;
		str.Format("FPN estimation high gain");
		DebugLog(str);

		str.Format("Rms: ");
		for(int i=0; i<12; i++) {
			sub.Format("%.1f, ", fpn_rms_h[i]);
			str.Append(sub);
		}
		DebugLog(str);

		//		subFPN = true;
	}

	return done;
}

void CGraDlg::CalcFPNL()
{
	float avg;
	int err;

	for (int j = 0; j < 12; j++) {
		avg = 0;
		for (int i = 1; i <= NUM_FPN; i++) {
			avg += (float)fpn_data_l[i][j];
		}
		fpn_data_l[0][j] = (int)round(avg / NUM_FPN);
		fpn_avg_l[j] = avg / NUM_FPN;
	}

	for (int j = 0; j < 12; j++) {
		avg = 0;
		for (int i = 1; i <= NUM_FPN; i++) {
			err = fpn_data_l[i][j] - fpn_data_l[0][j];
			avg += (float)(err * err);
		}
		fpn_rms_l[j] = sqrt(avg / NUM_FPN);
	}

	for(int j=0; j<12; j++) {
		g_TrimReader.Node[0].fpn[0][j] = (double)fpn_avg_l[j];
	}
}

void CGraDlg::CalcFPNH()
{
	float avg;
	int err;

	for (int j = 0; j < 12; j++) {
		avg = 0;
		for (int i = 1; i <= NUM_FPN; i++) {
			avg += (float)fpn_data_h[i][j];
		}
		fpn_data_h[0][j] = (int)round(avg / NUM_FPN);
		fpn_avg_h[j] = avg / NUM_FPN;
	}

	for (int j = 0; j < 12; j++) {
		avg = 0;
		for (int i = 1; i <= NUM_FPN; i++) {
			err = fpn_data_h[i][j] - fpn_data_h[0][j];
			avg += (float)(err * err);
		}
		fpn_rms_h[j] = sqrt(avg / NUM_FPN);
	}

	for(int j=0; j<12; j++) {
		g_TrimReader.Node[0].fpn[1][j] = (double)fpn_avg_h[j];
	}
}

void CGraDlg::OnBnClickedCheckSubfpn()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	if(m_subFPN) {
		g_pTrimDlg->SetTXbin(0xf);
		g_pTrimDlg->m_ComboxTX.SetCurSel(0xf);
	}
}

// select int time to make the max high byte to 0xf0

int CGraDlg::Step2()
{
	int done = 0;

	autoRepeatCounter++;
	OnClickedBtnReadrow();						// read a row

	int min_hb = MinHighByte(autoRepeatCounter);		// cast to int

	float inc = (float)(0xf0 - min_hb) * 0.2;			// slowly approach the opt int time to avoid saturation

	if (abs(inc) < 3)
		done = 1;
	else
		opt_int_time += inc;

	if (opt_int_time < 10) opt_int_time = 10;
	else if (opt_int_time > 800) opt_int_time = 800;

	if (g_pTrimDlg) 
		g_pTrimDlg->SetIntTime(opt_int_time);

	if (autoRepeatCounter >= 16) {
		done = 1;
	}

	if (done) {
		autoRepeatCounter = 0;

		CString str;
		str.Format("Min HighByte: 0x%x, Saturation int time: %0.2f", min_hb, opt_int_time);
		DebugLog(str);
	}

	return done;
}


void CGraDlg::OnBnClickedButtonStep0()
{
	// TODO: Add your control notification handler code here
	if (!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	m_PixelData = "";
	SetDlgItemText(IDC_EDIT_RecData, m_PixelData);

	int_time = 1.0;
	if (g_pTrimDlg) g_pTrimDlg->SetIntTime(int_time);

	autoRepeatCounter = 0;


	chain = false;
	step = 0;

	//====================

	SetTimer(1, videoElapseTime, NULL);		// 
}
