#pragma once
#include "afxwin.h"

//***************************************************************
//Constant definition
//***************************************************************
#define UM_TRIMPROCESS WM_USER + 205	//Trim dialog自定义消息序号

#define sendtrimmsg		1		// ReadRow button
#define sendeeprommsg	2		// Read EEPROM
#define trimdatanum		200		// graphic dialog transmitted data number

extern BYTE TrimBuf[trimdatanum];	// trim dialog transmitted data buffer

// CTrimDlg dialog

class CTrimDlg : public CDialog
{
	DECLARE_DYNAMIC(CTrimDlg)

public:
	CTrimDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTrimDlg();

// Dialog Data
	enum { IDD = IDD_TRIM_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg LRESULT OnTrimProcess(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnReread();
	void TrimCalMainMsg();
	CComboBox m_ComboxRampgen;
	virtual BOOL OnInitDialog();
	CComboBox m_ComboxRange;
	CComboBox m_ComboxAmux;
	CComboBox m_ComboxIpix;
	CComboBox m_ComboxSwitch;
	CComboBox m_ComboxTestADC;
	CComboBox m_ComboxTX;
	CComboBox m_ComboxV15;
	CComboBox m_ComboxV20;
	CComboBox m_ComboxV24;
	CEdit m_EditRecData;
	CEdit m_EditData;
	afx_msg void OnCbnSelchangeComboRampgen();
	afx_msg void OnCbnSelchangeComboRange();
	afx_msg void OnCbnSelchangeComboV24();
	afx_msg void OnCbnSelchangeComboV20();
	afx_msg void OnCbnSelchangeComboV15();
	afx_msg void OnCbnSelchangeComboIpix();
	afx_msg void OnCbnSelchangeComboSwitch();
	afx_msg void OnCbnSelchangeComboTxbin();
	afx_msg void OnCbnSelchangeComboAmuxsel();
	afx_msg void OnCbnSelchangeComboTestadc();
	afx_msg void OnBnClickedBtnSendcmd();
	afx_msg void OnBnClickedBtnItgtim();
	void ResetTrim(void);
	void SetRange(BYTE range);
	void SetV20(char v20);
	void SetRampgen(BYTE rampgen);
	void SetTXbin(char txbin);
	void ResetTxBin(void);
	void SetIntTime(float);
	afx_msg void OnBnClickedBtnItgtim2();
	void SetV15(BYTE v15);
	afx_msg void OnBnClickedBtnShowdata();
	afx_msg void OnBnClickedButtonTrimtest();
	void GetTemp();

	void EEPROMWrite(int chan, int total_packets, int packet_index, BYTE* packet_data, int packet_size);
	void EEPROMRead(int chan);

	afx_msg void OnBnClickedButtonWeep();

	afx_msg void OnBnClickedButtonUpdatetemp();
};
