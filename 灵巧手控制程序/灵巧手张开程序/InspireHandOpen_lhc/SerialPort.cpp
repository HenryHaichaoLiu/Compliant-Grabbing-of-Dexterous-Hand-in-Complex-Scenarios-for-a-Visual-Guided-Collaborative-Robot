#include "StdAfx.h"    
#include "SerialPort.h"    
#include <process.h>    
#include <iostream>    
#include "InspireHand.h"   
using  namespace std;
/** �߳��˳���־ */
bool CSerialPort::s_bExit = false;
/** ������������ʱ,sleep���´β�ѯ�����ʱ��,��λ:�� */
const UINT SLEEP_TIME_INTERVAL = 5;

CSerialPort::CSerialPort(void)
	: m_hListenThread(INVALID_HANDLE_VALUE)
{
	m_hComm = INVALID_HANDLE_VALUE;
	m_hListenThread = INVALID_HANDLE_VALUE;
	InitializeCriticalSection(&m_csCommunicationSync);
}
CSerialPort::CSerialPort(UINT portNo /*= 1*/, UINT baud /*= CBR_9600*/)
	: m_hListenThread(INVALID_HANDLE_VALUE)
{
	m_hComm = INVALID_HANDLE_VALUE;
	m_hListenThread = INVALID_HANDLE_VALUE;
	InitializeCriticalSection(&m_csCommunicationSync);
	InitPort(portNo, baud);

}
CSerialPort::~CSerialPort(void)
{
	CloseListenTread();
	ClosePort();
	DeleteCriticalSection(&m_csCommunicationSync);
}
bool CSerialPort::InitPort(UINT portNo /*= 1*/, UINT baud /*= CBR_9600*/, char parity /*= 'N'*/,
	UINT databits /*= 8*/, UINT stopsbits /*= 1*/, DWORD dwCommEvents /*= EV_RXCHAR*/)
{

	/** ��ʱ����,���ƶ�����ת��Ϊ�ַ�����ʽ,�Թ���DCB�ṹ */
	char szDCBparam[50];
	sprintf_s(szDCBparam, "baud=%d parity=%c data=%d stop=%d", baud, parity, databits, stopsbits);

	/** ��ָ������,�ú����ڲ��Ѿ����ٽ�������,�����벻Ҫ�ӱ��� */
	if (!openPort(portNo))
	{
		return false;
	}
	/** �����ٽ�� */
	EnterCriticalSection(&m_csCommunicationSync);

	/** �Ƿ��д����� */
	BOOL bIsSuccess = TRUE;

	/** �ڴ˿���������������Ļ�������С,���������,��ϵͳ������Ĭ��ֵ.
	*  �Լ����û�������Сʱ,Ҫע�������Դ�һЩ,���⻺�������
	*/
	if (bIsSuccess )
	{
	bIsSuccess = SetupComm(m_hComm,500,500);
	}

	/** ���ô��ڵĳ�ʱʱ��,����Ϊ0,��ʾ��ʹ�ó�ʱ���� */
	COMMTIMEOUTS  CommTimeouts;
	CommTimeouts.ReadIntervalTimeout = 0;
	CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	CommTimeouts.ReadTotalTimeoutConstant = 0;
	CommTimeouts.WriteTotalTimeoutMultiplier = 0;
	CommTimeouts.WriteTotalTimeoutConstant = 0;
	if (bIsSuccess)
	{
		bIsSuccess = SetCommTimeouts(m_hComm, &CommTimeouts);
	}

	DCB  dcb;
	if (bIsSuccess)
	{
		// ��ANSI�ַ���ת��ΪUNICODE�ַ���    
		DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, NULL, 0);
		wchar_t* pwText = new wchar_t[dwNum];
		if (!MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, pwText, dwNum))
		{
			bIsSuccess = TRUE;
		}

		/** ��ȡ��ǰ�������ò���,���ҹ��촮��DCB���� */
		bIsSuccess = GetCommState(m_hComm, &dcb) && BuildCommDCB(pwText, &dcb);
		/** ����RTS flow���� */
		dcb.fRtsControl = RTS_CONTROL_ENABLE;

		/** �ͷ��ڴ�ռ� */
		delete[] pwText;
	}

	if (bIsSuccess)
	{
		/** ʹ��DCB�������ô���״̬ */
		bIsSuccess = SetCommState(m_hComm, &dcb);
	}

	/**  ��մ��ڻ����� */
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	/** �뿪�ٽ�� */
	LeaveCriticalSection(&m_csCommunicationSync);

	return bIsSuccess == TRUE;
}

bool CSerialPort::InitPort(UINT portNo, const LPDCB& plDCB)
{
	/** ��ָ������,�ú����ڲ��Ѿ����ٽ�������,�����벻Ҫ�ӱ��� */
	if (!openPort(portNo))
	{
		return false;
	}

	/** �����ٽ�� */
	EnterCriticalSection(&m_csCommunicationSync);

	/** ���ô��ڲ��� */
	if (!SetCommState(m_hComm, plDCB))
	{
		return false;
	}

	/**  ��մ��ڻ����� */
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	/** �뿪�ٽ�� */
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}

void CSerialPort::ClosePort()
{
	/** ����д��ڱ��򿪣��ر��� */
	if (m_hComm != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
	}
}

bool CSerialPort::openPort(UINT portNo)
{
	/** �����ٽ�� */
	EnterCriticalSection(&m_csCommunicationSync);

	/** �Ѵ��ڵı��ת��Ϊ�豸�� */
	char szPort[50];
	sprintf_s(szPort, "COM%d", portNo);

	/** ��ָ���Ĵ��� */
	m_hComm = CreateFileA(szPort,  /** �豸��,COM1,COM2�� */
		GENERIC_READ | GENERIC_WRITE, /** ����ģʽ,��ͬʱ��д */
		0,                            /** ����ģʽ,0��ʾ������ */
		NULL,                         /** ��ȫ������,һ��ʹ��NULL */
		OPEN_EXISTING,                /** �ò�����ʾ�豸�������,���򴴽�ʧ�� */
		0,                            /** �ò���0��ͬ����ʽ��ReadFile��WriteFile����Ϊ����ʽ */
		NULL);

	/** �����ʧ�ܣ��ͷ���Դ������ */
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}

	/** �˳��ٽ��� */
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}

bool CSerialPort::OpenListenThread()
{
	/** ����߳��Ƿ��Ѿ������� */
	if (m_hListenThread != INVALID_HANDLE_VALUE)
	{
		/** �߳��Ѿ����� */
		return false;
	}

	s_bExit = false;
	/** �߳�ID */
	UINT threadId;
	/** �����������ݼ����߳� */
	m_hListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, this, 0, &threadId);
	if (!m_hListenThread)
	{
		return false;
	}
	/** �����̵߳����ȼ�,������ͨ�߳� */
	if (!SetThreadPriority(m_hListenThread, THREAD_PRIORITY_ABOVE_NORMAL))
	{
		return false;
	}

	return true;
}

bool CSerialPort::CloseListenTread()
{
	if (m_hListenThread != INVALID_HANDLE_VALUE)
	{
		/** ֪ͨ�߳��˳� */
		s_bExit = true;

		/** �ȴ��߳��˳� */
		Sleep(10);

		/** ���߳̾����Ч */
		CloseHandle(m_hListenThread);
		m_hListenThread = INVALID_HANDLE_VALUE;
	}
	return true;
}
/** ��ȡ���ڻ���������δ����ȡ�ֽ���
*  @return: UINT  �����Ƿ�ɹ�
*  @note:   �����ڻ�������������ʱ,����0
*  @see:
*/
UINT CSerialPort::GetBytesInCOM()
{
	DWORD dwError = 0;  /** ������ */
	COMSTAT  comstat;   /** COMSTAT�ṹ��,��¼ͨ���豸��״̬��Ϣ */
	memset(&comstat, 0, sizeof(COMSTAT));
	UINT BytesInQue = 0;
	/** �ڵ���ReadFile��WriteFile֮ǰ,ͨ�������������ǰ�����Ĵ����־
	*	����ȡͨѶ�豸��״̬
	*/
	if (ClearCommError(m_hComm, &dwError, &comstat))
	{
		BytesInQue = comstat.cbInQue; /** ��ȡ�����뻺�����е��ֽ��� */
	}
	return BytesInQue;
}

UINT WINAPI CSerialPort::ListenThread(void* pParam)
{
	/** �õ������ָ�� */
	CSerialPort* pSerialPort = reinterpret_cast<CSerialPort*>(pParam);
	// �߳�ѭ��,��ѯ��ʽ��ȡ��������    
	while (!pSerialPort->s_bExit)
	{
		UINT BytesInQue = pSerialPort->GetBytesInCOM();
		/** ����������뻺������������,����Ϣһ���ٲ�ѯ */
		if (BytesInQue == 0)
		{
			Sleep(SLEEP_TIME_INTERVAL);
			continue;
		}
		/** ��ȡ���뻺�����е����ݲ������ʾ */
		char cRecved = 0x00;
		int count = 0;
		if(BytesInQue > 0)
		{
			if (pSerialPort->ReadChar(BytesInQue) == true)
			{
				pSerialPort->Uart_Rx_Pro(BytesInQue);
			}
		} 
	}
	return 0;
}
//�Խ������ݵ�Э�����
void CSerialPort::Uart_Rx_Pro(DWORD Bytes_int)
{
	UCHAR sum_check = 0;
	int length = str_rx_buffer[3] + 5;
	static int Status_Rec = 0;
	static int Index = 0;
	static int Frame_length = 0;
	for (int i = 0; i < (int)Bytes_int; i++)
	{
		switch (Status_Rec)
		{
		case 0:
			{
				if ((UCHAR)str_rx_buffer[i] == 0x90 )
				{					
					str_rx[Index] = str_rx_buffer[i];
					Index = 1;
					Status_Rec = 1;
				}
				break;
			}
		case 1:
		{
			if ((UCHAR)str_rx_buffer[i] == 0xeb)
			{
				str_rx[Index] = str_rx_buffer[i];
				Index = 2;
				Status_Rec = 2;
			}
			break;
		}
		case 2:  //ID
		{
			str_rx[Index] = str_rx_buffer[i];
			Index = 3;
			Status_Rec = 3;
			break;
		}
		case 3:  //����
		{
			str_rx[Index] = str_rx_buffer[i];
			Frame_length = (str_rx[Index]&0x00FF) + 5;
			Index = 4;
			Status_Rec = 4;
			break;
		}
		case 4://
		{
			str_rx[Index] = str_rx_buffer[i];
			if (Index == Frame_length-1)//����һ֡�ѽ���
			{
				for (int x = 0; x < (Frame_length - 3); x++)
				{
					sum_check = sum_check + str_rx[2 + x];
				}
				if (sum_check == (UCHAR)str_rx[Index])//У����ȡ
				{
					m_hand.id = str_rx[2];
					short Rcv_Prameter_Index = ((short)str_rx[6] << 8) + (0x00FF & (short)str_rx[5]);					
					if ((UCHAR)str_rx[4] == CMD_HANDG3_READ)
					{

						if (Rcv_Prameter_Index == CMD_FINGER_ANGLE_2B(0))
						{
							m_hand.rcv_type = GET_STATE;
							for (int m = 0; m < 6; m++)
							{
								m_hand.finger[m].Angle_Act = (str_rx[8 + 2 * m] << 8) + str_rx[7 + 2 * m];
								m_hand.finger[m].Force_Act = (str_rx[8 + 24 + 2 * m] << 8) + str_rx[7 + 24 + 2 * m];
								m_hand.finger[m].Current = (str_rx[8 + 36 + 2 * m] << 8) + str_rx[7 + 36 + 2 * m];
								m_hand.finger[m].ErrorCode = str_rx[8 + 36+24 +m];
							}
						}
					}
					if ((UCHAR)str_rx_buffer[4] == CMD_HANDG3_WRITE)
					{
						if (Rcv_Prameter_Index == CMD_FINGER_ANGLE_SET_2B(0))
						{
							m_hand.rcv_type = SET_ANGLE;
						}
						if (Rcv_Prameter_Index == CMD_FINGER_SPEED_SET_2B(0))
						{
							m_hand.rcv_type = SET_SPEED;
						}
						if (Rcv_Prameter_Index == CMD_FINGER_FORCE_SET_2B(0))
						{
							m_hand.rcv_type = SET_FORCE;
						}
					}
					Index = 0;
					Status_Rec = 0;
				}
				else
				{
					Index = 0;
					Status_Rec = 0;
				}
				Index = 0;
				Status_Rec = 0;
			}
			Index++;		
		}
		default:break;
		}
	}

}
bool CSerialPort::ReadChar(DWORD int_byte)
{
	BOOL  bResult = TRUE;
	DWORD BytesRead = 0;
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	/** �ٽ�������(���û��Ᵽ֤�̰߳�ȫ) */
	EnterCriticalSection(&m_csCommunicationSync);
	/** �ӻ�������ȡһ���ֽڵ����� */
	bResult = ReadFile(m_hComm, str_rx_buffer, int_byte, &BytesRead, NULL);
	if ((!bResult))
	{
		/** ��ȡ������,���Ը��ݸô�����������ԭ�� */
		DWORD dwError = GetLastError();
		/** ��մ��ڻ����� */
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}
	/** �뿪�ٽ��� */
	LeaveCriticalSection(&m_csCommunicationSync);
	return true;
}
bool CSerialPort::WriteData(unsigned int length)
{
	BOOL   bResult = TRUE;
	DWORD  BytesToSend = 0;
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	/** �ٽ������� */
	EnterCriticalSection(&m_csCommunicationSync);
	/** �򻺳���д��ָ���������� */
	bResult = WriteFile(m_hComm, str_tx_buffer, length, &BytesToSend, NULL);
	if (!bResult)
	{
		DWORD dwError = GetLastError();
		/** ��մ��ڻ����� */
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}
	/** �뿪�ٽ��� */
	LeaveCriticalSection(&m_csCommunicationSync);
	m_hand.rcv_type = NO_RESPONSE;
	return true;
}

