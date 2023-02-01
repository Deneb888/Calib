#define TRIM_IMAGER_SIZE 12
#define MAX_TRIMBUFF 256

#define EPKT_SZ  51					// Not include parity
#define NUM_EPKT 4

class CTrimNode {

public:

	double kb[TRIM_IMAGER_SIZE][6];		// New (auto) calib method with hump factor
	double fpn[2][TRIM_IMAGER_SIZE];	// 0 - lg, 1 - hg

	unsigned int rampgen;
	unsigned int range;
	unsigned int auto_v20[2];
	unsigned int auto_v15;
	unsigned int version;

	double tempcal[TRIM_IMAGER_SIZE];

	int kbi[TRIM_IMAGER_SIZE][6];		// New (auto) calib method with hump factor, all are int
	int fpni[2][TRIM_IMAGER_SIZE];		// 0 - lg, 1 - hg

	CString name;

	BYTE	trim_buff[MAX_TRIMBUFF];
	int		tbuff_size;
	int		tbuff_rptr;

public:

	CTrimNode();
	
private:

	void Initialize();
};

#define TRIM_MAX_NODE 4
#define TRIM_MAX_WORD 640

class CTrimReader {

protected:

	CFile InFile;

	CString WordBuf[TRIM_MAX_WORD];
	int WordIndex;
	int MaxWord;
	CString CurWord;
	int calib2;
//	int version;

public:

	CTrimNode Node[TRIM_MAX_NODE];
	CTrimNode *curNode;
	int NumNode;
	CString TrimDotData;

public:

	CTrimReader();
	~CTrimReader();

	int Load(TCHAR*);
	void CloseFile();
	void Parse();
	void ParseNode();

	int GetNumNode() {
		return NumNode;
	}

	int ADCCorrection(int NumData, BYTE HighByte, BYTE LowByte,  int pixelNum, int PCRNum, int gain_mode, int *flag);
	int ADCCorrectionf(int NumData, BYTE HighByte, BYTE LowByte, int pixelNum, int PCRNum, int gain_mode, int *flag);
	int ADCCorrectioni(int NumData, BYTE HighByte, BYTE LowByte, int pixelNum, int PCRNum, int gain_mode, int *flag);
	int ADCCorrectionOld(int NumData, BYTE HighByte, BYTE LowByte, int pixelNum, int PCRNum, int gain_mode, int *flag);

	void SetCalib2(int c) {
		calib2 = c;
	}

	void JustParseMatrix();
	void WriteTrimDatai();
	void WriteTrimData();

	int Add2TrimBuff(int i, int);
	int Add2TrimBuff(int i, BYTE);
	int WriteTrimBuff(int i);
	int TrimBuff2Int(int i);
	BYTE TrimBuff2Byte(int i);
	void RestoreTrimBuff(int k);
	void CopyEepromBuff(int k);

	void Convert2Int();

 protected:

	BOOL fileLoaded;

	void ParseMatrix();
	void ParseArray(int);
	void ParseValue(int);
	void ParseMatrixOld();

private:

	int GetWord();
	int Match(CString);
};



