
// PCRProjectDlg.h : header file
//
//**************************************************************
//Message definition to Resource.h
//**************************************************************

#define WM_RegDlg_event		WM_USER + 101		
#define WM_GraDlg_event		WM_USER + 102	
#define WM_TrimDlg_event	WM_USER + 103
#define WM_ReadHID_event	WM_USER+105

//.......................................................................................

#if !defined(AFX_USBHIDIOCDLG_H__0B2AAA84_F5A9_11D3_9F47_0050048108EA__INCLUDED_)
#define AFX_USBHIDIOCDLG_H__0B2AAA84_F5A9_11D3_9F47_0050048108EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//....................................................................

//#pragma once
//#include "mscomm1.h"
#include "GraDlg.h"
//#include "RegDlg.h"
#include "TrimDlg.h"

//****************************************************
//Global variable definition
//****************************************************
#define TxNum 64		// the number of the buffer for sent data to COMX
#define RxNum 200		// the number of the buffer for received data from COMX
#define dNum 29			// 每次向串口发送的字节数
#define ONCOMNUM	59		// 下位机返回多少字节会出发COM口中断

#define HIDREPORTNUM 64		//	HID 每个report byte个数

#define GraCmd 0x02			// return 0x02 command 
#define ReadCmd 0x04		// 
#define TempCmd 0x10		// return 0x10 command
#define PidCmd 0x11			// return 0x11 command
#define PidReadCmd 0x12		// return 0x12 command

#define HIDBUFSIZE 12






// CPCRProjectDlg dialog
class CPCRProjectDlg : public CDialogEx
{
// Construction
public:
	CPCRProjectDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PCRPROJECT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
//	afx_msg LRESULT OnRegDlg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGraDlg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTrimDlg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReadHID(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

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
	void SendHIDRead();

public:
//	CMscomm1 m_mscomm;
	DECLARE_EVENTSINK_MAP()
	void OnCommMscomm1();
	CTabCtrl m_tab;
//	CRegDlg m_RegDlg;
	CGraDlg m_GraDlg;
	CTrimDlg m_TrimDlg;
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	void GetCom();
	CString m_StaticOpenComm;
	void CommSend (int num);
	afx_msg void OnBnClickedBtnOpencomm();
	afx_msg void OnBnClickedBtnOpenhid();
//	CString m_strBytesReceived;
//	CEdit m_BytesReceived;
	CListBox m_BytesReceived;
	CString m_strBytesReceived;
	afx_msg void OnBnClickedBtnSendhid();
	afx_msg void OnBnClickedBtnReadhid();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
//	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonChipid();
};

#endif // !defined(AFX_USBHIDIOCDLG_H__0B2AAA84_F5A9_11D3_9F47_0050048108EA__INCLUDED_)