
#include "stdafx.h"
#include "InspireHand.h"
#include "SerialPort.h"
CSerialPort mySerialPort(7, CBR_115200);//��ʼ����������󣬶˿ں�5��������115200��
CInspireHand myHand(&mySerialPort, 0);//��ʼ�������������ID=1
Hand_State hand_state = { NO_RESPONSE,0,{0} };//�ṹ��hand_state �洢������״̬��Ϣ
int main(int argc, _TCHAR* argv[])
{
	if (!mySerialPort.OpenListenThread())
	{
		printf("OpenListenThread fail !\n");
	}
	else
	{
		printf("OpenListenThread success !\n");
	}

	//myHand.Send_Frame_SetForce(20, 20, 20, 20, 20, 20);

	int time = 1;
	while (time < 2)
	{

		//myHand.Send_Frame_SetAngle(1000, 1000, 1000, 1000, 1000, 40);
		//myHand.Send_Frame_SetSpeed(300, 300, 300, 300, 300, 300);

		// myHand.Send_Frame_SetForce(60, 60, 60, 60, 60, 60);
		//myHand.Send_Frame_SetSpeed(100, 100, 100, 300, 300, 300);
		//myHand.Send_Frame_SetForce(20, 20, 20, 20, 20, 20);
		//myHand.Send_Frame_SetSpeed(500, 500, 500, 500, 500, 500);
		// myHand.Send_Frame_SetForce(50, 50, 50, 50, 120, 100);
		//Sleep(2000);
		//myHand.Send_Frame_SetForce(50, 50, 50, 50, 50, 50);
		myHand.Send_Frame_SetAngle(500, 500, 500, 500, 550, 40);
		myHand.Send_Frame_SetForce(100, 100, 100, 100, 100, 60);
		//myHand.Send_Frame_SetForce(30, 30, 30, 30, 30, 30);
		//myHand.Send_Frame_SetSpeed(500, 500, 500, 500, 500, 500);
		//myHand.Send_Frame_SetSpeed(100, 100, 100, 300, 300, 300);

		Sleep(1000);
		// myHand.Send_Frame_SetSpeed(200, 200, 200, 200, 200, 200);
		//myHand.Send_Frame_SetForce(50, 50, 50, 50, 50, 50);
		//Sleep(2000);
		time++;
	}
	//myHand.Send_Frame_SetSpeed(300, 300, 300, 300, 300, 300);
	//myHand.Send_Frame_SetAngle(1000, 1000, 1000, 1000, 1000, 40);
	//Sleep(2000);
	//myHand.Send_Frame_SetAngle(800, 800, 800, 800, 800, 40);
	//myHand.Send_Frame_SetForce(25, 25, 25, 25, 25, 25);
	//myHand.Send_Frame_SetSpeed(300, 300, 300, 300, 300, 300);
	//myHand.Send_Frame_SetAngle(500, 500, 500, 500, 500, 5); // (733, 819, 832, 815, 868, 0)
	if (myHand.Send_Frame_GetState_block(&hand_state))
	{
		printf("�ɹ���ȡID=%d������״̬\n", hand_state.id);
		printf("          �Ƕ�|����|����|����\n");
		printf("СĴָ    %d|%d|%d|%d\n", hand_state.finger[0].Angle_Act,
			hand_state.finger[0].Force_Act,
			hand_state.finger[0].Current,
			hand_state.finger[0].ErrorCode
		);
		printf("����ָ    %d|%d|%d|%d\n", hand_state.finger[1].Angle_Act,
			hand_state.finger[1].Force_Act,
			hand_state.finger[1].Current,
			hand_state.finger[1].ErrorCode
		);
		printf("��ָ      %d|%d|%d|%d\n", hand_state.finger[2].Angle_Act,
			hand_state.finger[2].Force_Act,
			hand_state.finger[2].Current,
			hand_state.finger[2].ErrorCode
		);
		printf("ʳָ      %d|%d|%d|%d\n", hand_state.finger[3].Angle_Act,
			hand_state.finger[3].Force_Act,
			hand_state.finger[3].Current,
			hand_state.finger[3].ErrorCode
		);
		printf("��Ĵָ    %d|%d|%d|%d\n", hand_state.finger[4].Angle_Act,
			hand_state.finger[4].Force_Act,
			hand_state.finger[4].Current,
			hand_state.finger[4].ErrorCode
		);
		printf("��Ĵָ��ת%d|%d|%d|%d\n", hand_state.finger[5].Angle_Act,
			hand_state.finger[5].Force_Act,
			hand_state.finger[5].Current,
			hand_state.finger[5].ErrorCode
		);
		printf("СĴָ%d|%d|%d|%d\n", hand_state.finger[0].Angle_Act,
			hand_state.finger[0].Force_Act,
			hand_state.finger[0].Current,
			hand_state.finger[0].ErrorCode
		);
	}
	/*while (TRUE)
	{
		myHand.Send_Frame_SetAngle(0, -1, -1, -1, 1000, 1000);
		Sleep(100);
		myHand.Send_Frame_SetAngle_block(-1, 0, -1, -1, 1000, 900);
		Sleep(100);
		myHand.Send_Frame_SetAngle_block(-1, -1, 0, -1, 1000, 1000);
		Sleep(100);
		myHand.Send_Frame_SetAngle_block(-1, -1, -1, 0, 1000, 900);
		Sleep(100);
		myHand.Send_Frame_SetAngle_block(0, -1, -1, -1, -1, 1000);
		if (myHand.Send_Frame_GetState_block(&hand_state))
		{
			printf("�ɹ���ȡID=%d������״̬\n", hand_state.id);
			printf("          �Ƕ�|����|����|����\n");
			printf("СĴָ    %d|%d|%d|%d\n", hand_state.finger[0].Angle_Act,
				hand_state.finger[0].Force_Act,
				hand_state.finger[0].Current,
				hand_state.finger[0].ErrorCode
			);
			printf("����ָ    %d|%d|%d|%d\n", hand_state.finger[1].Angle_Act,
				hand_state.finger[1].Force_Act,
				hand_state.finger[1].Current,
				hand_state.finger[1].ErrorCode
			);
			printf("��ָ      %d|%d|%d|%d\n", hand_state.finger[2].Angle_Act,
				hand_state.finger[2].Force_Act,
				hand_state.finger[2].Current,
				hand_state.finger[2].ErrorCode
			);
			printf("ʳָ      %d|%d|%d|%d\n", hand_state.finger[3].Angle_Act,
				hand_state.finger[3].Force_Act,
				hand_state.finger[3].Current,
				hand_state.finger[3].ErrorCode
			);
			printf("��Ĵָ    %d|%d|%d|%d\n", hand_state.finger[4].Angle_Act,
				hand_state.finger[4].Force_Act,
				hand_state.finger[4].Current,
				hand_state.finger[4].ErrorCode
			);
			printf("��Ĵָ��ת%d|%d|%d|%d\n", hand_state.finger[5].Angle_Act,
				hand_state.finger[5].Force_Act,
				hand_state.finger[5].Current,
				hand_state.finger[5].ErrorCode
			);
			printf("СĴָ%d|%d|%d|%d\n", hand_state.finger[0].Angle_Act,
				hand_state.finger[0].Force_Act,
				hand_state.finger[0].Current,
				hand_state.finger[0].ErrorCode
			);

		}
	}*/
	return 0;
}