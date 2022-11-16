
#pragma once
#include "IO.h"
#include "windows.h"
#include "time.h"
#include "SerialPort.h"
//ͨѶ���ֺ궨��
#define FRAME_HEAD1  				0xEB
#define FRAME_HEAD2  				0x90
#define FRAME_HEAD1_RT  			0x90
#define FRAME_HEAD2_RT  			0xEB
#define CMD_HANDG3_READ  												0x11 	//���������ڲ��Ĵ���
#define CMD_HANDG3_WRITE  												0x12	//д�������ڲ��Ĵ���
#define CMD_HANDG3_RETURN_DRVSTATUS										0x13	//�������Զ�����������״̬
#define GET_HAND_ANGLE_ACT  	       0x01
#define GET_HAND_FORCE_ACT  	       0x02
#define GET_HAND_CURRENT  	           0x03
#define GET_HAND_STATUS  	           0x04
#define GET_HAND_ERRORCODE  	       0x05
//�����ڲ��Ĵ�����ַ��
#define SYS_PARATABLE_FLAG_2B 											0
#define SYS_SAVE_TO_FLASH_B 											2
#define SYS_RESTORE_FACTORY_B 											3
#define SYS_SOFTWARE_VERSION_2B											14
#define SYS_PRODUCT_SN_8B			    								16
#define SYS_FINGER_FORCE_CONFIG_B(m)			    					(24+m)
#define SYS_FINGER_MID_DRVPOS_2B(m)										(30+m*2)
#define SYS_FINGER_MIN_DRVPOS_2B(m)										(42+m*2)
#define SYS_FINGER_MAX_DRVPOS_2B(m)										(54+m*2)
#define SYS_FORCE_RATIO_MUL10_2B(m)										(66+m*2)
#define SYS_YBPDLT_BENDED_NOLOAD_2B(m)									(78+m*2)
#define SYS_YBPBASE_NOLOAD_2B(m)										(90+m*2)
#define SYS_MOVE_K_0_13_2B(k,ID)										(102+k*12+ID*2)
//CMDMAP
#define CMD_ID_B 														1000
#define CMD_REDU_RATIO_B 												1001
#define CMD_AUTO_RUTRUNSTATUS_CONFIG_B			    					1002
#define CMD_POWER_ON_CONFIG_B			    							1003
#define CMD_CLEAR_ERROR_B												1004
#define CMD_SAVE_TO_FLASH_B 											1005
#define CMD_RESTORE_FACTORY_B 											1006
#define CMD_IDENTIFYING_CODE_2B 										1007
#define CMD_MOVE_K_B													1008
#define CMD_FORCE_CLB_B													1009
#define CMD_PWMMAX_B(m)												    (1014+m)    //PWM������
#define CMD_CURLMT_2B(m)												(1020+m*2)  //��ָ��������ֵ
#define CMD_DEFAULT_SPEED_2B(m)											(1032+m*2)
#define CMD_DEFAULT_FORCE_2B(m)											(1044+m*2)
#define CMD_MOVE_K_14_2B(k,ID)											(1066+(k-14)*12+ID*2)

#define CMD_TEMP_2B 											        1470
#define CMD_VOLTAGE_2B 											        1472
#define CMD_FINGER_DRVPOS_SET_2B(m)										(1474+m*2)
#define CMD_FINGER_ANGLE_SET_2B(m)										(1486+m*2)
#define CMD_FINGER_FORCE_SET_2B(m)										(1498+m*2)
#define CMD_FINGER_ADCDLT_SET_2B(m)										(1510+m*2)
#define CMD_FINGER_SPEED_SET_2B(m)										(1522+m*2)
#define CMD_FINGER_DRVPOS_2B(m)											(1534+m*2)
#define CMD_FINGER_ANGLE_2B(m)											(1546+m*2)
#define CMD_FINGER_ANGLE_X_2B(m)										(1558+m*2)
#define CMD_FINGER_YBPDLT_2B(m)											(1570+m*2)
#define CMD_FINGER_FORCEACT_2B(m)										(1582+m*2)
#define CMD_FINGER_CURRENT_2B(m)										(1594+m*2)
#define CMD_FINGER_ERROR_B(m)											(1606+m)
#define CMD_FINGER_STATUS_B(m)											(1612+m)
#define CMD_FINGER_TEMP_B(m)									    	(1618+m)

/** ������ͨѶ��
*
*  ���ƺͼ���������
*/
class CInspireHand
{
public:
	/** ��������Ĺ��캯��
	*
	*  @param:  CSerialPort* para_p_SerialPort ������Ķ���ָ��
	*  @param: int para_id �����ֵ�ID��
	*  @note:   �ú����ڷ�����ָ��֡����������
	*  @see:
	*/
	CInspireHand(CSerialPort* para_p_SerialPort, int para_id);
	~CInspireHand(void);
	/*��������*/
	CSerialPort* p_SerialPort;
	Hand_State state;
	int id;
public:
	/** ���Ͷ�ȡ������״̬��Ϣ��ָ��֡����������
	*  @param:  NULL
	*  @return: NULL
	*  @note:   �ú����ڷ�����ָ��֡����������   
	*/
	/** Send command frame to read the hand state information.This functoin is non blocking functoin.
	*  @param:  NULL
	*  @return: NULL
	*  @note:   This function returns immediately after sending the instruction frame
	*/
	void Send_Frame_GetState(void);
	/**  Send command frame  to read the hand state information, wait for the hand to reply, 
		 and store state information in Hand_State structure pointed by para_state.This functoin is blocking functoin.
	*  @param:  Hand_State* para_state,Pointer for storing state information structure of the hand
	*  @return: bool, has the hand returned status information
	*  @note:   After sending command frame, this function returns true after receiving response frame from the hand. If the hand does not reply within 300ms, it returns false.
	*  @see:
	*/
	/** ���Ͷ�ȡ������״̬��Ϣ��ָ��֡�������������ȴ������ֻظ�������������״̬��Ϣ��ŵ�para_stateָ���Hand_State�ṹ����
	*  @param:  Hand_State* para_state ���������״̬��Ϣ�ṹ���ָ��
	*  @return: bool�������Ƿ񷵻�״̬��Ϣ
	*  @note:   �ú����ڷ�����ָ��֡�󣬵ȴ������ֻظ��󷵻� true�����300ms��û���յ������ֵĻظ�������false
	*  @see:
	*/
	bool Send_Frame_GetState_block(Hand_State* para_state);
	/** Send command frame to set finger angle of the hand.This functoin is non blocking functoin.
	*  @param:  Val0 Set angle of little finger;Numerical range is -1~1000;-1��the finger finger ignores this command��0��the finger are completely bent��1000��the fingers are completely straight;
	*  @param:  Val1 Set angle of ring finger;Numerical range is -1~1000;-1��the finger finger ignores this command��0��the finger are completely bent��1000��the fingers are completely straight;
	*  @param:  Val2 Set angle of middle finger;Numerical range is -1~1000;-1��the finger finger ignores this command��0��the finger are completely bent��1000��the fingers are completely straight;
	*  @param:  Val3 Set angle of index finger;Numerical range is -1~1000;-1��the finger finger ignores this command��0��the finger are completely bent��1000��the fingers are completely straight;
	*  @param:  Val4 Set angle of thumb;Numerical range is -1~1000;-1��the finger finger ignores this command��0��the finger are completely bent��1000��the fingers are completely straight;
	*  @param:  Val5 Set angle of Thumb rotation;Numerical range is -1~1000;-1��the finger finger ignores this command��0��the finger are completely bent��1000��the fingers are completely straight;
	*  @return: NULL
	*  @note:   This function returns immediately after sending the instruction frame
	*/
	/** ����������������ָ�Ƕȵ�ָ��֡����������
	*  @param:  Val0 СĴָ���ýǶȡ���Χ-1~1000��-1����Ӧ��ָ���Դ�ָ�0����ָ���������̶ȣ�1000����ָ��ֱ�����̶�
	*  @param:  Val1 ����ָ���ýǶȡ���Χ-1~1000��-1����Ӧ��ָ���Դ�ָ�0����ָ���������̶ȣ�1000����ָ��ֱ�����̶�
	*  @param:  Val2 ��ָ���ýǶȡ���Χ-1~1000��-1����Ӧ��ָ���Դ�ָ�0����ָ���������̶ȣ�1000����ָ��ֱ�����̶�
	*  @param:  Val3 ʳָ���ýǶȡ���Χ-1~1000��-1����Ӧ��ָ���Դ�ָ�0����ָ���������̶ȣ�1000����ָ��ֱ�����̶�
	*  @param:  Val4 ��Ĵָ�������ýǶȡ���Χ-1~1000��-1����Ӧ��ָ���Դ�ָ�0����ָ���������̶ȣ�1000����ָ��ֱ�����̶�
	*  @param:  Val5 ��Ĵָ��ת���ýǶȡ���Χ-1~1000��-1����Ӧ��ָ���Դ�ָ�0����ָ���������̶ȣ�1000����ָ��ֱ�����̶�
	*  @return: NULL
	*  @note:   �ú����ڷ�����ָ��֡����������
	*/
	void Send_Frame_SetAngle(short Val0, short Val1, short Val2, short Val3, short Val4, short Val5);
	/** Send command frame to set finger angle of the hand,and wait for dexterous hand to reply.This functoin is blocking functoin.
	*  @param:  Val0 Set angle of little finger;Numerical range is -1~1000;-1��the finger finger ignores this command��0��the finger are completely bent��1000��the fingers are completely straight;
	*  @param:  Val1 Set angle of ring finger;Numerical range is -1~1000;-1��the finger finger ignores this command��0��the finger are completely bent��1000��the fingers are completely straight;
	*  @param:  Val2 Set angle of middle finger;Numerical range is -1~1000;-1��the finger finger ignores this command��0��the finger are completely bent��1000��the fingers are completely straight;
	*  @param:  Val3 Set angle of index finger;Numerical range is -1~1000;-1��the finger finger ignores this command��0��the finger are completely bent��1000��the fingers are completely straight;
	*  @param:  Val4 Set angle of thumb;Numerical range is -1~1000;-1��the finger finger ignores this command��0��the finger are completely bent��1000��the fingers are completely straight;
	*  @param:  Val5 Set angle of Thumb rotation;Numerical range is -1~1000;-1��the finger finger ignores this command��0��the finger are completely bent��1000��the fingers are completely straight;
	*  @return: bool, has the hand returned status information
	*  @note:   After sending command frame, this function returns true after receiving response frame from the hand. If the hand does not reply within 300ms, it returns false.
	*/
	/** ���Ͷ�ȡ������������ָ�Ƕȵ�ָ��֡�������������ȴ������ֻظ�
	*  @param:  Val0 СĴָ���ýǶȡ���Χ-1~1000��-1����Ӧ��ָ���Դ�ָ�0����ָ���������̶ȣ�1000����ָ��ֱ�����̶�
	*  @param:  Val1 ����ָ���ýǶȡ���Χ-1~1000��-1����Ӧ��ָ���Դ�ָ�0����ָ���������̶ȣ�1000����ָ��ֱ�����̶�
	*  @param:  Val2 ��ָ���ýǶȡ���Χ-1~1000��-1����Ӧ��ָ���Դ�ָ�0����ָ���������̶ȣ�1000����ָ��ֱ�����̶�
	*  @param:  Val3 ʳָ���ýǶȡ���Χ-1~1000��-1����Ӧ��ָ���Դ�ָ�0����ָ���������̶ȣ�1000����ָ��ֱ�����̶�
	*  @param:  Val4 ��Ĵָ�������ýǶȡ���Χ-1~1000��-1����Ӧ��ָ���Դ�ָ�0����ָ���������̶ȣ�1000����ָ��ֱ�����̶�
	*  @param:  Val5 ��Ĵָ��ת���ýǶȡ���Χ-1~1000��-1����Ӧ��ָ���Դ�ָ�0����ָ���������̶ȣ�1000����ָ��ֱ�����̶�
	*  @return: bool �������Ƿ��յ�������ȷ����Ϣ
	*  @note:   �ú����ڷ�����ָ��֡�󣬵ȴ������ֻظ��󷵻� true�����300ms��û���յ������ֵĻظ�������false
	*/
	bool Send_Frame_SetAngle_block(short Val0, short Val1, short Val2, short Val3, short Val4, short Val5);
	/** Send command frame to set finger speed of the hand.This functoin is non blocking functoin.
	*  @param:  Val0 Set speed of little finger;Numerical range is 1~1000;
	*  @param:  Val1 Set speed of ring finger;Numerical range is 1~1000;
	*  @param:  Val2 Set speed of middle finger;Numerical range is 1~1000;
	*  @param:  Val3 Set speed of index finger;Numerical range is 1~1000;
	*  @param:  Val4 Set speed of thumb;Numerical range is 1~1000;
	*  @param:  Val5 Set speed of Thumb rotation;Numerical range is 1~1000;
	*  @return: NULL
	*  @note:   This function returns immediately after sending the instruction frame
	*/
	/** ���Ͷ�ȡ������������ָ�ٶȵ�ָ��֡����������
	*  @param:  Val0 СĴָ�����ٶȡ���Χ1~1000��1������ٶȣ�1000������ٶ�
	*  @param:  Val1 ����ָ�����ٶȡ���Χ1~1000��1������ٶȣ�1000������ٶ�
	*  @param:  Val2 ��ָ�����ٶȡ���Χ1~1000��1������ٶȣ�1000������ٶ�
	*  @param:  Val3 ʳָ�����ٶȡ���Χ1~1000��1������ٶȣ�1000������ٶ�
	*  @param:  Val4 ��Ĵָ���������ٶȡ���Χ1~1000��1������ٶȣ�1000������ٶ�
	*  @param:  Val5 ��Ĵָ��ת�����ٶȡ���Χ1~1000��1������ٶȣ�1000������ٶ�
	*  @return: NULL
	*  @note:   �ú����ڷ�����ָ��֡����������
	*/
	void Send_Frame_SetSpeed(short Val0, short Val1, short Val2, short Val3, short Val4, short Val5);
	/** Send command frame to set finger speed of the hand,and wait for dexterous hand to reply.This functoin is blocking functoin.
	*  @param:  Val0 Set speed of little finger;Numerical range is 1~1000;
	*  @param:  Val1 Set speed of ring finger;Numerical range is 1~1000;
	*  @param:  Val2 Set speed of middle finger;Numerical range is 1~1000;
	*  @param:  Val3 Set speed of index finger;Numerical range is 1~1000;
	*  @param:  Val4 Set speed of thumb;Numerical range is 1~1000;
	*  @param:  Val5 Set speed of Thumb rotation;Numerical range is 1~1000;
	*  @return: bool, has the hand returned status information
	*  @note:   After sending command frame, this function returns true after receiving response frame from the hand. If the hand does not reply within 300ms, it returns false.
	*/
	/** ���Ͷ�ȡ������������ָ�ٶȵ�ָ��֡�������������ȴ������ֻظ�
	*  @param:  Val0 СĴָ�����ٶȡ���Χ1~1000��1������ٶȣ�1000������ٶ�
	*  @param:  Val1 ����ָ�����ٶȡ���Χ1~1000��1������ٶȣ�1000������ٶ�
	*  @param:  Val2 ��ָ�����ٶȡ���Χ1~1000��1������ٶȣ�1000������ٶ�
	*  @param:  Val3 ʳָ�����ٶȡ���Χ1~1000��1������ٶȣ�1000������ٶ�
	*  @param:  Val4 ��Ĵָ���������ٶȡ���Χ1~1000��1������ٶȣ�1000������ٶ�
	*  @param:  Val5 ��Ĵָ��ת�����ٶȡ���Χ1~1000��1������ٶȣ�1000������ٶ�
	*  @return: bool �������Ƿ��յ�������ȷ����Ϣ
	*  @note:   �ú����ڷ�����ָ��֡�󣬵ȴ������ֻظ��󷵻� true�����300ms��û���յ������ֵĻظ�������false
	*/
	bool Send_Frame_SetSpeed_block(short Val0, short Val1, short Val2, short Val3, short Val4, short Val5);
	/** Send command frame to set finger force of the hand.This functoin is non blocking functoin.
	*  @param:  Val0 Set force  of little finger;Numerical range is 1~1000;
	*  @param:  Val1 Set force  of ring finger;Numerical range is 1~1000;
	*  @param:  Val2 Set force  of middle finger;Numerical range is 1~1000;
	*  @param:  Val3 Set force  of index finger;Numerical range is 1~1000;
	*  @param:  Val4 Set force  of thumb;Numerical range is 1~1000;
	*  @param:  Val5 Set force  of Thumb rotation;Numerical range is 1~1000;
	*  @return: NULL
	*  @note:   This function returns immediately after sending the instruction frame
	*/
	/** ���Ͷ�ȡ������������ָ���ȵ�ָ��֡����������
	*  @param:  Val0 СĴָ�������ȡ���Χ1~1000��1����С���ȣ�1000����С����
	*  @param:  Val1 ����ָ�������ȡ���Χ1~1000��1����С���ȣ�1000����С����
	*  @param:  Val2 ��ָ�������ȡ���Χ1~1000��1����С���ȣ�1000����С����
	*  @param:  Val3 ʳָ�������ȡ���Χ1~1000��1����С���ȣ�1000����С����
	*  @param:  Val4 ��Ĵָ�����������ȡ���Χ1~1000��1����С���ȣ�1000����С����
	*  @param:  Val5 ��Ĵָ��ת�������ȡ���Χ1~1000��1����С���ȣ�1000����С����
	*  @return: NULL
	*  @note:   �ú����ڷ�����ָ��֡����������
	*  @see:
	*/
	void Send_Frame_SetForce(short Val0, short Val1, short Val2, short Val3, short Val4, short Val5);
	/** Send command frame to set finger force of the hand,and wait for dexterous hand to reply.This functoin is blocking functoin.
	*  @param:  Val0 Set force of little finger;Numerical range is 1~1000;
	*  @param:  Val1 Set force of ring finger;Numerical range is 1~1000;
	*  @param:  Val2 Set force of middle finger;Numerical range is 1~1000;
	*  @param:  Val3 Set force of index finger;Numerical range is 1~1000;
	*  @param:  Val4 Set force of thumb;Numerical range is 1~1000;
	*  @param:  Val5 Set force of Thumb rotation;Numerical range is 1~1000;
	*  @return: bool, has the hand returned status information
	*  @note:   After sending command frame, this function returns true after receiving response frame from the hand. If the hand does not reply within 300ms, it returns false.
	*/
	/** ���Ͷ�ȡ������������ָ���ȵ�ָ��֡�������������ȴ������ֻظ�
	*  @param:  Val0 СĴָ�������ȡ���Χ1~1000��1����С���ȣ�1000����С����
	*  @param:  Val1 ����ָ�������ȡ���Χ1~1000��1����С���ȣ�1000����С����
	*  @param:  Val2 ��ָ�������ȡ���Χ1~1000��1����С���ȣ�1000����С����
	*  @param:  Val3 ʳָ�������ȡ���Χ1~1000��1����С���ȣ�1000����С����
	*  @param:  Val4 ��Ĵָ�����������ȡ���Χ1~1000��1����С���ȣ�1000����С����
	*  @param:  Val5 ��Ĵָ��ת�������ȡ���Χ1~1000��1����С���ȣ�1000����С����
	*  @return: bool �������Ƿ��յ�������ȷ����Ϣ
	*  @note:   �ú����ڷ�����ָ��֡�󣬵ȴ������ֻظ��󷵻� true�����300ms��û���յ������ֵĻظ�������false
	*  @see:
	*/
	bool Send_Frame_SetForce_block(short Val0, short Val1, short Val2, short Val3, short Val4, short Val5);
	void Get_State(Hand_State *para_state);
};
