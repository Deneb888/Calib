// TrimDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"
#include "TrimDlg.h"
#include "IntTimeDlg.h"
#include "afxdialogex.h"
#include "TrimReader.h"

//*****************************************************************
//Constant definition
//*****************************************************************
// #define TxNum 50			// the number of the buffer for sent data to COMX
// #define RxNum 200		// the number of the buffer for received data from COMX


//***************************************************************
//Global variable definition
//***************************************************************
int TrimFlag;	// register dialog flag

BYTE TrimBuf[trimdatanum];	// trim dialog transmitted data buffer

BYTE TimCom;		//command下拉框的取值buffer
BYTE TimType;		//type下拉框的取值buffer

//*****************************************************************
//External variable definition
//*****************************************************************
extern BYTE TxData[TxNum];		// the buffer of sent data to COMX
extern BYTE RxData[RxNum];		// the buffer of received data from COMX

//****************************************************
//External function 
//****************************************************
extern unsigned char AsicConvert (unsigned char i, unsigned char j);				//ASIC convert to HEX
extern int ChangeNum (CString str, int length);										//十六进制字符串转十进制整型
extern  char* EditDataCvtChar (CString strCnv,  char * charRec);					//编辑框取值转字符变量

int gain_mode = 0;		// zd add, 0 high gain, 1 low gain
float int_time = 1.0;	// zd add, in ms
//extern float max_int_time;

extern BOOL g_DeviceDetected;
char g_TxBin = 0x0;

#define PRELOAD_RPGTRIM

extern CTrimReader g_TrimReader;

// CTrimDlg dialog

IMPLEMENT_DYNAMIC(CTrimDlg, CDialog)

CTrimDlg::CTrimDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTrimDlg::IDD, pParent)
{

}

CTrimDlg::~CTrimDlg()
{
}

void CTrimDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_Rampgen, m_ComboxRampgen);
	//	DDX_Control(pDX, IDC_COMBO_Rampgen2, m_ComboxRange);
	DDX_Control(pDX, IDC_COMBO_RANGE, m_ComboxRange);
	DDX_Control(pDX, IDC_COMBO_AmuxSel, m_ComboxAmux);
	//  DDX_Control(pDX, IDC_COMBO_CMD, m_ComboxcOM);
	DDX_Control(pDX, IDC_COMBO_IPIX, m_ComboxIpix);
	DDX_Control(pDX, IDC_COMBO_Switch, m_ComboxSwitch);
	DDX_Control(pDX, IDC_COMBO_TestADC, m_ComboxTestADC);
	DDX_Control(pDX, IDC_COMBO_TxBin, m_ComboxTX);
	//  DDX_Control(pDX, IDC_COMBO_Type, m_ComboxType);
	DDX_Control(pDX, IDC_COMBO_V15, m_ComboxV15);
	DDX_Control(pDX, IDC_COMBO_V20, m_ComboxV20);
	DDX_Control(pDX, IDC_COMBO_V24, m_ComboxV24);
	DDX_Control(pDX, IDC_EDIT_REDATA, m_EditRecData);
	DDX_Control(pDX, IDC_EDIT_SENDDATA, m_EditData);
}


BEGIN_MESSAGE_MAP(CTrimDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_REREAD, &CTrimDlg::OnBnClickedBtnReread)
	ON_CBN_SELCHANGE(IDC_COMBO_Rampgen, &CTrimDlg::OnCbnSelchangeComboRampgen)
	ON_CBN_SELCHANGE(IDC_COMBO_RANGE, &CTrimDlg::OnCbnSelchangeComboRange)
	ON_CBN_SELCHANGE(IDC_COMBO_V24, &CTrimDlg::OnCbnSelchangeComboV24)
	ON_CBN_SELCHANGE(IDC_COMBO_V20, &CTrimDlg::OnCbnSelchangeComboV20)
	ON_CBN_SELCHANGE(IDC_COMBO_V15, &CTrimDlg::OnCbnSelchangeComboV15)
	ON_CBN_SELCHANGE(IDC_COMBO_IPIX, &CTrimDlg::OnCbnSelchangeComboIpix)
	ON_CBN_SELCHANGE(IDC_COMBO_Switch, &CTrimDlg::OnCbnSelchangeComboSwitch)
	ON_CBN_SELCHANGE(IDC_COMBO_TxBin, &CTrimDlg::OnCbnSelchangeComboTxbin)
	ON_CBN_SELCHANGE(IDC_COMBO_AmuxSel, &CTrimDlg::OnCbnSelchangeComboAmuxsel)
	ON_CBN_SELCHANGE(IDC_COMBO_TestADC, &CTrimDlg::OnCbnSelchangeComboTestadc)
	ON_BN_CLICKED(IDC_BTN_SENDCMD, &CTrimDlg::OnBnClickedBtnSendcmd)
//	ON_CBN_KILLFOCUS(IDC_COMBO_CMD, &CTrimDlg::OnKillfocusComboCmd)
//	ON_CBN_KILLFOCUS(IDC_COMBO_Type, &CTrimDlg::OnKillfocusComboType)
ON_BN_CLICKED(IDC_BTN_ITGTIM, &CTrimDlg::OnBnClickedBtnItgtim)
ON_BN_CLICKED(IDC_BTN_ITGTIM2, &CTrimDlg::OnBnClickedBtnItgtim2)
ON_BN_CLICKED(IDC_BTN_SHOWDATA, &CTrimDlg::OnBnClickedBtnShowdata)
ON_BN_CLICKED(IDC_BUTTON_TRIMTEST, &CTrimDlg::OnBnClickedButtonTrimtest)
ON_MESSAGE(UM_TRIMPROCESS, &CTrimDlg::OnTrimProcess)
ON_BN_CLICKED(IDC_BUTTON_WEEP, &CTrimDlg::OnBnClickedButtonWeep)
ON_BN_CLICKED(IDC_BUTTON_UPDATETEMP, &CTrimDlg::OnBnClickedButtonUpdatetemp)
END_MESSAGE_MAP()


// CTrimDlg message handlers

int myint = 13;

void CTrimDlg::OnBnClickedBtnReread()
{
	// TODO: Add your control notification handler code here

/*	TrimFlag = sendtrimmsg; 

	TxData[0] = 0xaa;			//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x01;			//data length
	TxData[3] = 0x05;	//data type, dat edit first byte
	TxData[4] = 0x00;	//real data, data edit second byte
	TxData[5] = 0x08;	//real data, data edit third byte
	TxData[6] = 0x17;		//check sum
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
*/

	// TODO: Add your control notification handler code here

	ResetTrim();

/*	IntTimeDlg intDlg;

	intDlg.m_IntTime = myint;

	if (intDlg.DoModal() == IDOK)
	{
	myint = intDlg.m_IntTime;
	}
*/
}

void CTrimDlg::TrimCalMainMsg()
{
	WPARAM a = 8;
	LPARAM b = 9;
	HWND hwnd = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	::SendMessage(hwnd,WM_TrimDlg_event,a,b);
}

BOOL CTrimDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	//Rampgen add string
	CString sRamp;
	for (unsigned int i=0; i<256; i++) // this is strange
	{
		sRamp.Format("%.2X",i);
		m_ComboxRampgen.AddString(sRamp);
	}

	//Range add string
	m_ComboxRange.AddString("0x00");
	m_ComboxRange.AddString("0x01");
	m_ComboxRange.AddString("0x02");
	m_ComboxRange.AddString("0x03");
	m_ComboxRange.AddString("0x04");
	m_ComboxRange.AddString("0x05");
	m_ComboxRange.AddString("0x06");
	m_ComboxRange.AddString("0x07");
	m_ComboxRange.AddString("0x08");
	m_ComboxRange.AddString("0x09");
	m_ComboxRange.AddString("0x0a");
	m_ComboxRange.AddString("0x0b");
	m_ComboxRange.AddString("0x0c");
	m_ComboxRange.AddString("0x0d");
	m_ComboxRange.AddString("0x0e");
	m_ComboxRange.AddString("0x0f");

	//V24 add string
	m_ComboxV24.AddString("0x00");
	m_ComboxV24.AddString("0x01");
	m_ComboxV24.AddString("0x02");
	m_ComboxV24.AddString("0x03");
	m_ComboxV24.AddString("0x04");
	m_ComboxV24.AddString("0x05");
	m_ComboxV24.AddString("0x06");
	m_ComboxV24.AddString("0x07");
	m_ComboxV24.AddString("0x08");
	m_ComboxV24.AddString("0x09");
	m_ComboxV24.AddString("0x0a");
	m_ComboxV24.AddString("0x0b");
	m_ComboxV24.AddString("0x0c");
	m_ComboxV24.AddString("0x0d");
	m_ComboxV24.AddString("0x0e");
	m_ComboxV24.AddString("0x0f");

	//V20 add string
	m_ComboxV20.AddString("0x00");
	m_ComboxV20.AddString("0x01");
	m_ComboxV20.AddString("0x02");
	m_ComboxV20.AddString("0x03");
	m_ComboxV20.AddString("0x04");
	m_ComboxV20.AddString("0x05");
	m_ComboxV20.AddString("0x06");
	m_ComboxV20.AddString("0x07");
	m_ComboxV20.AddString("0x08");
	m_ComboxV20.AddString("0x09");
	m_ComboxV20.AddString("0x0a");
	m_ComboxV20.AddString("0x0b");
	m_ComboxV20.AddString("0x0c");
	m_ComboxV20.AddString("0x0d");
	m_ComboxV20.AddString("0x0e");
	m_ComboxV20.AddString("0x0f");

	//V15 add string
	m_ComboxV15.AddString("0x00");
	m_ComboxV15.AddString("0x01");
	m_ComboxV15.AddString("0x02");
	m_ComboxV15.AddString("0x03");
	m_ComboxV15.AddString("0x04");
	m_ComboxV15.AddString("0x05");
	m_ComboxV15.AddString("0x06");
	m_ComboxV15.AddString("0x07");
	m_ComboxV15.AddString("0x08");
	m_ComboxV15.AddString("0x09");
	m_ComboxV15.AddString("0x0a");
	m_ComboxV15.AddString("0x0b");
	m_ComboxV15.AddString("0x0c");
	m_ComboxV15.AddString("0x0d");
	m_ComboxV15.AddString("0x0e");
	m_ComboxV15.AddString("0x0f");

	//Ipix add string
	m_ComboxIpix.AddString("0x00");
	m_ComboxIpix.AddString("0x01");
	m_ComboxIpix.AddString("0x02");
	m_ComboxIpix.AddString("0x03");
	m_ComboxIpix.AddString("0x04");
	m_ComboxIpix.AddString("0x05");
	m_ComboxIpix.AddString("0x06");
	m_ComboxIpix.AddString("0x07");
	m_ComboxIpix.AddString("0x08");
	m_ComboxIpix.AddString("0x09");
	m_ComboxIpix.AddString("0x0a");
	m_ComboxIpix.AddString("0x0b");
	m_ComboxIpix.AddString("0x0c");
	m_ComboxIpix.AddString("0x0d");
	m_ComboxIpix.AddString("0x0e");
	m_ComboxIpix.AddString("0x0f");

	//Switch add string
	m_ComboxSwitch.AddString("0 high gain");
	m_ComboxSwitch.AddString("1 low gain");

	//TX add string
	m_ComboxTX.AddString("0x00");
	m_ComboxTX.AddString("0x01");
	m_ComboxTX.AddString("0x02");
	m_ComboxTX.AddString("0x03");
	m_ComboxTX.AddString("0x04");
	m_ComboxTX.AddString("0x05");
	m_ComboxTX.AddString("0x06");
	m_ComboxTX.AddString("0x07");
	m_ComboxTX.AddString("0x08");
	m_ComboxTX.AddString("0x09");
	m_ComboxTX.AddString("0x0a");
	m_ComboxTX.AddString("0x0b");
	m_ComboxTX.AddString("0x0c");
	m_ComboxTX.AddString("0x0d");
	m_ComboxTX.AddString("0x0e");
	m_ComboxTX.AddString("0x0f");

	//Amux add string
	m_ComboxAmux.AddString("Vbgr");
	m_ComboxAmux.AddString("Vcm");
	m_ComboxAmux.AddString("V24");
	m_ComboxAmux.AddString("V15");
	m_ComboxAmux.AddString("V20");

	//Test ADC add string
	m_ComboxTestADC.AddString("TestADC");
	m_ComboxTestADC.AddString("None");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTrimDlg::ResetTrim(void)
{
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	TrimFlag = sendtrimmsg; 

	TxData[0] = 0xaa;			//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x03;			//data length
	TxData[3] = 0x0F;	//data type, dat edit first byte
	TxData[4] = 0x00;	//real data, data edit second byte
	TxData[5] = 0x00;	//real data, data edit third byte
	TxData[6] = 0x13;		//check sum
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();

	m_ComboxV20.SetCurSel(8);
	m_ComboxV15.SetCurSel(8);
	m_ComboxRampgen.SetCurSel(0x88);
	m_ComboxRange.SetCurSel(8);
	m_ComboxV24.SetCurSel(8);
	m_ComboxIpix.SetCurSel(8);
	m_ComboxTX.SetCurSel(8);
	m_ComboxSwitch.SetCurSel(0);
	m_ComboxAmux.SetCurSel(0);
	m_ComboxTestADC.SetCurSel(0);

	// zd add initalize

#ifdef PRELOAD_RPGTRIM

	SetRampgen(0x88);
	SetRange(0x0f);
	m_ComboxRampgen.SetCurSel(0x88);
	m_ComboxRange.SetCurSel(0x0f);

	SetTXbin(0xf);
	m_ComboxTX.SetCurSel(0xf);

#endif

	CString kstg;
//	float kfl;

	kstg.Format("%f", int_time);
	SetDlgItemText(IDC_EDIT_IGTIME,kstg);	

//	kstg.Format("%f", max_int_time);
//	SetDlgItemText(IDC_EDIT_IGTIME2,kstg);	

	OnBnClickedButtonUpdatetemp();

//=======================
}


void CTrimDlg::OnCbnSelchangeComboRampgen()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxRampgen.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxRampgen.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer

	char* dataEdit="";
	dataEdit = EditDataCvtChar(strText, dataEdit);	//将获取的字符串转成16进制字符

	SetRampgen(dataEdit[0]);
}


void CTrimDlg::OnCbnSelchangeComboRange()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxRange.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxRange.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);
	SetRange(dataEdit[1]);
}


void CTrimDlg::OnCbnSelchangeComboV24()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxV24.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxV24.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x03;		//data type, date edit first byte
	TxData[4] = dataEdit[1];	//real data, date edit second byte
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = sendtrimmsg;

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::OnCbnSelchangeComboV20()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxV20.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxV20.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer

	char* dataEdit="";
	dataEdit = EditDataCvtChar(strText, dataEdit);

	SetV20(dataEdit[1]);
}


void CTrimDlg::OnCbnSelchangeComboV15()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	int nIndex = m_ComboxV15.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxV15.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);

	SetV15(dataEdit[1]);
}


void CTrimDlg::OnCbnSelchangeComboIpix()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxIpix.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxIpix.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x06;		//data type, date edit first byte
	TxData[4] = dataEdit[1];	//real data, date edit second byte
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = sendtrimmsg;

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::OnCbnSelchangeComboSwitch()
{
	// TODO: Add your control notification handler code here

	TrimFlag = sendtrimmsg;

	int nIndex = m_ComboxSwitch.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxSwitch.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中

	if (strText == "0 high gain")
	{
		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x01;		//command
		TxData[2] = 0x02;		//data length
		TxData[3] = 0x07;		//data type, date edit first byte
		TxData[4] = 0x00;	//real data, date edit second byte
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
		TrimCalMainMsg();
		gain_mode = 0;
	}
	else	
	{	
		if (strText == "1 low gain")
		{
			TxData[0] = 0xaa;		//preamble code
			TxData[1] = 0x01;		//command
			TxData[2] = 0x02;		//data length
			TxData[3] = 0x07;		//data type, date edit first byte
			TxData[4] = 0x01;	//real data, date edit second byte
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
			TrimCalMainMsg();
			gain_mode = 1;
		}
		else
			MessageBox("please check the right item");
	}	
}


void CTrimDlg::OnCbnSelchangeComboTxbin()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}


	int nIndex = m_ComboxTX.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxTX.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);

	SetTXbin(dataEdit[1]);
}


void CTrimDlg::OnCbnSelchangeComboAmuxsel()
{
	// TODO: Add your control notification handler code here

	TrimFlag = sendtrimmsg;

	int nIndex = m_ComboxAmux.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxAmux.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer


	if (strText == "Vbgr")
	{
		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x01;		//command
		TxData[2] = 0x02;		//data length
		TxData[3] = 0x09;		//data type, date edit first byte
		TxData[4] = 0x00;	//real data, date edit second byte
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
		TrimCalMainMsg();
	}	
	else
	{
		if (strText == "Vcm")
		{
			TxData[0] = 0xaa;		//preamble code
			TxData[1] = 0x01;		//command
			TxData[2] = 0x02;		//data length
			TxData[3] = 0x09;		//data type, date edit first byte
			TxData[4] = 0x01;	//real data, date edit second byte
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
			TrimCalMainMsg();
		}
		else
		{
			if (strText == "V24")
			{
				TxData[0] = 0xaa;		//preamble code
				TxData[1] = 0x01;		//command
				TxData[2] = 0x02;		//data length
				TxData[3] = 0x09;		//data type, date edit first byte
				TxData[4] = 0x02;		//real data, date edit second byte
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
				TrimCalMainMsg();
			}
			else
			{
				if (strText == "V15")
				{
					TxData[0] = 0xaa;		//preamble code
					TxData[1] = 0x01;		//command
					TxData[2] = 0x02;		//data length
					TxData[3] = 0x09;		//data type, date edit first byte
					TxData[4] = 0x03;		//real data, date edit second byte
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
					TrimCalMainMsg();
				}
				else
				{
					if (strText == "V20")
					{
						TxData[0] = 0xaa;		//preamble code
						TxData[1] = 0x01;		//command
						TxData[2] = 0x02;		//data length
						TxData[3] = 0x09;		//data type, date edit first byte
						TxData[4] = 0x04;		//real data, date edit second byte
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
						TrimCalMainMsg();
					}
					else
						MessageBox("please check right item");
				}
			}
		}

	}
}


void CTrimDlg::OnCbnSelchangeComboTestadc()
{
	// TODO: Add your control notification handler code here

	TrimFlag = sendtrimmsg;

	int nIndex = m_ComboxTestADC.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxTestADC.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中


	if (strText == "TestADC")
	{
		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x01;		//command
		TxData[2] = 0x02;		//data length
		TxData[3] = 0x0a;		//data type, date edit first byte
		TxData[4] = 0x00;	//real data, date edit second byte
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
		TrimCalMainMsg();
	}
	else	
	{	
		if (strText == "None")
		{
			TxData[0] = 0xaa;		//preamble code
			TxData[1] = 0x01;		//command
			TxData[2] = 0x02;		//data length
			TxData[3] = 0x0a;		//data type, date edit first byte
			TxData[4] = 0x01;	//real data, date edit second byte
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
			TrimCalMainMsg();
		}
		else
			MessageBox("please check the right item");
	}
}


void CTrimDlg::OnBnClickedBtnSendcmd()
{
	// TODO: Add your control notification handler code here

	CString sData, sCmd, sType;
	char* cData ="";
	char* cCmd = "";
	char* cType = "";
	int d_Num;

	TrimFlag = sendtrimmsg;


	memset(TxData,0,sizeof(TxData));

	GetDlgItemText(IDC_EDIT_CMD,sCmd);
	d_Num = (sCmd.GetLength())/2;
	cCmd = EditDataCvtChar(sCmd,cCmd);

	GetDlgItemText(IDC_EDIT_TYPE,sType);
	d_Num = (sType.GetLength())/2;
	cType = EditDataCvtChar(sType,cType);

	GetDlgItemText(IDC_EDIT_Data,sData);
	d_Num = (sData.GetLength())/2;

	cData = EditDataCvtChar(sData,cData);

	TxData[0] = 0xaa;
	TxData[1]= cCmd[0];
	TxData[2]= d_Num+1;
	TxData[3]= cType[0];
	for(int i=0; i<d_Num; i++)
	{
		TxData[i+4] = cData[i];
	}
	for(int j=1; j<(d_Num+4); j++)
	{
		TxData[d_Num+4] += TxData[j];
	}
	TxData[d_Num+5] = 0x17;
	TxData[d_Num+6] = 0x17;

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::OnBnClickedBtnItgtim()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	TrimFlag = sendtrimmsg;

	IntTimeDlg intDlg;

	intDlg.m_IntTime = int_time;

	if (intDlg.DoModal() == IDOK)
	{
	int_time = intDlg.m_IntTime;
	}

	//取Intergrate Time编辑框的值传给主对话框
	CString kstg;
//	float kfl;

	kstg.Format("%0.2f", int_time);
	SetDlgItemText(IDC_EDIT_IGTIME,kstg);	

//	GetDlgItemText(IDC_EDIT_IGTIME,kstg);		//从编辑框获取数值字符串

//	kfl = (float)atof(kstg);		//将字符串转成浮点型数据

	unsigned char * hData = (unsigned char *) & int_time;	//将浮点数据转化为十六进制数据

	TrimBuf[0] = hData[0];	//存到窗口传递buffer
	TrimBuf[1] = hData[1];
	TrimBuf[2] = hData[2];
	TrimBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x20;		//data type, date edit first byte
	TxData[4] = TrimBuf[0];		//real data, date edit second byte
	TxData[5] = TrimBuf[1];
	TxData[6] = TrimBuf[2];
	TxData[7] = TrimBuf[3];
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[8] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7];		//check sum
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[9] = 0x17;		//back code
	TxData[10] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}

void CTrimDlg::OnBnClickedBtnItgtim2()
{
	// TODO: Add your control notification handler code here
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

//	TrimFlag = sendtrimmsg;

//	IntTimeDlg intDlg;

//	intDlg.m_IntTime = max_int_time;

//	if (intDlg.DoModal() == IDOK)
//	{
//		max_int_time = intDlg.m_IntTime;
//		if(max_int_time < 40) {
	//		max_int_time = 40;
//			MessageBox("Saturation Int Time must be 40ms or more");
//		}
//	}

	//取Intergrate Time编辑框的值传给主对话框
//	CString kstg;
//	float kfl;

//	kstg.Format("%f", max_int_time);
//	SetDlgItemText(IDC_EDIT_IGTIME2,kstg);	
}

void CTrimDlg::SetRange(BYTE range)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x02;		//data type, date edit first byte
	TxData[4] = range;	//real data, date edit second byte
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = sendtrimmsg;

	//Call maindlg message
	TrimCalMainMsg();

	g_TrimReader.Node[0].range = (unsigned int)range;			// This is because range is selected by user.
}

void CTrimDlg::SetV20(char v20)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x04;		//data type, date edit first byte
	TxData[4] = v20;		//real data, date edit second byte
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = sendtrimmsg;

	//Call maindlg message
	TrimCalMainMsg();

	m_ComboxV20.SetCurSel(v20);
}

void CTrimDlg::SetV15(BYTE v)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x05;		//data type, date edit first byte
	TxData[4] = v;	//real data, date edit second byte
								//0x01 means send vedio data
								//0x00 means stop vedio data
	TxData[5] = TxData[1] + TxData[2] + TxData[3] + TxData[4];		//check sum
	if (TxData[5] == 0x17)
		TxData[5] = 0x18;
	else
		TxData[5] = TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = sendtrimmsg;

	//Call maindlg message
	TrimCalMainMsg();

	g_TrimReader.Node[0].auto_v15 = (unsigned int)v;			// This is because v15 can be modified by user.
}

void CTrimDlg::SetRampgen(BYTE rampgen)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x01;		//data type, date edit first byte
	TxData[4] = rampgen;	//real data, date edit second byte
	//0x01 means send video data
	//0x00 means stop video data
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = sendtrimmsg;

	//Call maindlg message
	TrimCalMainMsg();

	g_TrimReader.Node[0].rampgen = (unsigned int)rampgen;			// This is because rampgen can be modified by user.
}

void CTrimDlg::SetTXbin(char txbin)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x08;		//data type, date edit first byte
	TxData[4] = txbin;	//real data, date edit second byte
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	g_TxBin = txbin;

	TrimFlag = sendtrimmsg;

	//Call maindlg message
	TrimCalMainMsg();
}

void CTrimDlg::ResetTxBin(void)
{
		g_TxBin = 0x8;
		m_ComboxTX.SetCurSel(8);
}


void CTrimDlg::SetIntTime(float int_t)
{
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	TrimFlag = sendtrimmsg;

	int_time = int_t;

	//取Intergrate Time编辑框的值传给主对话框
	CString kstg;
//	float kfl;

	kstg.Format("%0.2f", int_time);
	SetDlgItemText(IDC_EDIT_IGTIME,kstg);	

//	GetDlgItemText(IDC_EDIT_IGTIME,kstg);		//从编辑框获取数值字符串

//	kfl = (float)atof(kstg);		//将字符串转成浮点型数据

	unsigned char * hData = (unsigned char *) & int_time;	//将浮点数据转化为十六进制数据

	TrimBuf[0] = hData[0];	//存到窗口传递buffer
	TrimBuf[1] = hData[1];
	TrimBuf[2] = hData[2];
	TrimBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x20;		//data type, date edit first byte
	TxData[4] = TrimBuf[0];		//real data, date edit second byte
	TxData[5] = TrimBuf[1];
	TxData[6] = TrimBuf[2];
	TxData[7] = TrimBuf[3];
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[8] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7];		//check sum
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[9] = 0x17;		//back code
	TxData[10] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}

extern CString RegRecStr;

void CTrimDlg::OnBnClickedBtnShowdata()
{
	// TODO: Add your control notification handler code here
			
	SetDlgItemText(IDC_EDIT_REDATA,RegRecStr);
}

void CTrimDlg::OnBnClickedButtonTrimtest()
{
	// TODO: Add your control notification handler code here

	int col, ecode;
	BYTE hb, lb;
	int result;
	CString str;

	GetDlgItemText(IDC_EDIT_HB, str);
	hb = (BYTE)_tcstol(str, NULL, 16);
	GetDlgItemText(IDC_EDIT_LB, str);
	lb = (BYTE)_tcstol(str, NULL, 16);
	GetDlgItemText(IDC_EDIT_COL, str);
	col = atoi(str);		// col 1 based

	result = g_TrimReader.ADCCorrection(col - 1, hb, lb, 12, 1, 1, &ecode);

	str.Format("%d, 0x%x (%d)", result, result, ecode);

	SetDlgItemText(IDC_EDIT_RESULT, str);
}

BYTE EepromBuff[NUM_EPKT][EPKT_SZ + 1];

float TempC_sum = 0;
#define T_REPEAT 2

afx_msg LRESULT CTrimDlg::OnTrimProcess(WPARAM wParam, LPARAM lParam)
{
	BYTE type;
	type = RxData[4];	//
	int index = RxData[7];		// For command type 2d EEPROM read command

								//	CString sTemp1,sTemp2;
								//	sTemp1.Empty();
								//	sTemp2.Empty();

								//	CString sTem;
								//	sTem.Empty();

	unsigned char cTem[4];
	float *fTem;

	switch (type)
	{
	case 0x11:		// PCR temp1
		{
		// get setup time
		cTem[0] = RxData[5];
		cTem[1] = RxData[6];
		cTem[2] = RxData[7];
		cTem[3] = RxData[8];
		fTem = (float *)cTem;

		TempC_sum += (*fTem);

		break;
		}
		/*		case 0x12:		// PCR temp2
		{
		// get setup time
		cTem[0] = RxData[5];
		cTem[1] = RxData[6];
		cTem[2] = RxData[7];
		cTem[3] = RxData[8];
		fTem = (float *)cTem;
		sTem.Format("%g",*fTem);

		sTemp2 = sTem;
		SetDlgItemTextA(IDC_TRIM_EDIT_PCRTEMP2,sTemp2);
		//清除数据传输buffer
		memset(RxData,0,sizeof(RxData));
		break;
		} 
		*/

	case 0x2d:		// EEPROM data, check parity here too.
	{
		BYTE eeprom_parity = 0;

		for (int i = 0; i < EPKT_SZ + 1; i++) {		// 
			EepromBuff[index][i] = RxData[8 + i];

			if (i < EPKT_SZ) {
				eeprom_parity += RxData[8 + i];
			} 
			else {
				if (eeprom_parity != RxData[8 + i]) {
					MessageBox(_T("Packet parity error!"));
				}
			}
		}

		memset(RxData, 0, sizeof(RxData));
		break;
	}
	default:
		break;
	}

	return 0;
}


void CTrimDlg::GetTemp()
{
	TrimFlag = sendtrimmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x04;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x11;		//data type
	TxData[4] = 0x00;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1] + TxData[2] + TxData[3] + TxData[4] + TxData[5];		//check sum
	if (TxData[6] == 0x17)
		TxData[6] = 0x18;
	else
		TxData[6] = TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

							//Call maindlg message
	TrimCalMainMsg();
}

float g_juncTemp = 0;

void CTrimDlg::OnBnClickedButtonUpdatetemp()
{
	if (!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached", "ULVision", MB_ICONWARNING);
		return;
	}

	TempC_sum = 0;
	for (int i = 0; i<T_REPEAT; i++) {
		GetTemp();
	}

	float TempC;
	CString sTemp;

	sTemp.Empty();

	TempC = TempC_sum / T_REPEAT;
	sTemp.Format("%.3f", TempC);
	SetDlgItemTextA(IDC_EDIT_JUNCTEMP, sTemp);

	//Clear buffer
	memset(RxData, 0, sizeof(RxData));

	g_juncTemp = TempC;
}



//=============== EEPROM write =======================

// Use EPKT_SZ, NUM_EPKT

void CTrimDlg::EEPROMWrite(int chan, int total_packets, int packet_index, BYTE* packet_data, int packet_size)
{
	if (packet_size > 53) return;		// Do nothing if packet size too big.

/*	Total HID packet is 64, minus 6 (header, tail) is 58, 
	-4 protocol header, 54, minus 1 parity, data packet max size is 53.
*/

	BYTE eeprom_parity = 0;

	TrimFlag = sendtrimmsg;

	int len = packet_size + 4 + 1;		// 4: protocol header, Add a EEPROM parity byte
	BYTE check_sum = 0;
	int i;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = len;		//data length
	TxData[3] = 0x2d;		//data type, date edit first byte
	TxData[4] = (BYTE)chan;		//real data, date edit second byte
	TxData[5] = (BYTE)total_packets;
	TxData[6] = (BYTE)packet_index;

	for (i = 0; i < packet_size; i++) {
		TxData[7 + i] = packet_data[i];
		eeprom_parity += packet_data[i];
	}

	TxData[7 + packet_size] = eeprom_parity;

	for (i = 1; i < 8 + packet_size; i++) {
		check_sum += TxData[i];
	}

	TxData[8 + packet_size] = check_sum;		//check sum

	if (TxData[8 + packet_size] == 0x17) {
		TxData[8 + packet_size] = 0x18;
	}

	TxData[9 + packet_size] = 0x17;				//back code
	TxData[10 + packet_size] = 0x17;			//back code

	TrimCalMainMsg();							//Call maindlg message
}

void CTrimDlg::EEPROMRead(int chan)
{
	TrimFlag = sendeeprommsg;			// only the eeprom read use this message type

	TxData[0] = 0xaa;					//preamble code
	TxData[1] = 0x04;					//command
	TxData[2] = 0x02;					//data length
	TxData[3] = 0x2d;					//data type
	TxData[4] = (BYTE)chan;			//	real data
									//	TxData[5] = 0x00;
	TxData[5] = TxData[1] + TxData[2] + TxData[3] + TxData[4];		//check sum
	if (TxData[5] == 0x17)
		TxData[5] = 0x18;
	else
		TxData[5] = TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimCalMainMsg();		//Call maindlg message
}

void CTrimDlg::OnBnClickedButtonWeep()
{
	if (!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	// Write to EEPROM from trim buff

	g_TrimReader.Convert2Int();	
	BYTE *pdata = g_TrimReader.curNode->trim_buff;
	int tsize = g_TrimReader.WriteTrimBuff(0);

	for (int i = 0; i < NUM_EPKT; i++) {
		EEPROMWrite(0, NUM_EPKT, i, pdata + EPKT_SZ * i, EPKT_SZ);
	}

	// Now read and verify

	EEPROMRead(0);		

	int num_err = 0;

	for (int i = 0; i < NUM_EPKT; i++) {
		for (int j = 0; j < EPKT_SZ; j++) {
			if (pdata[i * EPKT_SZ + j] != EepromBuff[i][j]) {
				num_err++;
			}
		}
	}

	if (num_err > 0) {
		MessageBox("EEPROM program failed");
	}
	else {
		MessageBox("EEPROM program success!");
	}
}

