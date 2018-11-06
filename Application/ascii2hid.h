
#ifndef __ASCII2HID_H__
#define __ASCII2HID_H__

#include "BSP.h"

//��ý�����
#define	next_track_hid		0		//��һ��
#define	prev_track_hid		1		//��һ��
#define	stop_hid			2		//ֹͣ
#define	play_pause_hid		3		//����/��ͣ
#define	mute_hid			4		//����
#define	media_hid			5		//��ý��
#define	volume_dec_hid		6		//������
#define	volume_add_hid		7		//������

#define	www_web_hid			8		//��ҳ
#define	www_back_hid		9		//����
#define	www_forward			10		//ǰ��
#define	www_stop_hid		11		//ֹͣ
#define	www_refresh_hid		12		//ˢ��
#define	www_favorite_hid	13		//�ղؼ�
#define	www_search_hid		14		//����
#define	mail_hid			15		//�ʼ�

#define	my_computer_hid		16		//�ҵĵ���
#define	calculator_hid		17		//������

#define	power_hid			24		//�ػ�
#define	sleep_hid			25		//˯��
#define	wake_up_hid			26		//����


//��׼����
#define	hid_k42				0x32
#define	hid_k45				0x64
#define	hid_enter			0x28
#define	hid_esc				0x29
#define	hid_semicolon		0x33	//�ֺ�" ; "
#define	hid_quotation_mark	0x34	//���� " '",
#define	hid_comma			0x36	//����
#define	hid_full_stop		0x37	//���
#define	hid_solidus_r		0x38	//��б��"/"
#define	hid_space			0x2c
#define	hid_scroll			0x47
#define	hid_print			0x46
#define	hid_pause			0x48
#define	hid_wave_line		0x35	//������" `( ~ )"		

#define	hid_back			0x2a
#define	hid_tab				0x2b
#define	hid_underline		0x2d	//�»���"-(_)"
#define	hid_equal_mark		0x2e	//�Ⱥ�"=(+)"
#define	hid_braces_l		0x2f	//��������
#define	hid_braces_r		0x30	//�Ұ������
#define	hid_solidus_l		0x31	//��б��"\"
#define	hid_caps			0x39
#define	hid_f1				0x3a
#define	hid_f2				0x3b
#define	hid_f3				0x3c
#define	hid_f4				0x3d
#define	hid_f5				0x3e
#define	hid_f6				0x3f
#define	hid_f7				0x40
#define	hid_f8				0x41
#define	hid_f9				0x42
#define	hid_f10				0x43
#define	hid_f11				0x44
#define	hid_f12				0x45
#define	hid_ins				0x49
#define	hid_home			0x4a
#define	hid_page_up			0x4b
#define	hid_del				0x4c
#define	hid_end				0x4d
#define	hid_page_down		0x4e
#define	hid_right			0x4f
#define	hid_left			0x50
#define	hid_down			0x51
#define	hid_up				0x52
#define	hid_menu			0x65	//�˵���


//���ּ���(С������)
#define	hid_nk_num			0x53
#define	hid_nk_div			0x54	//����"/"
#define	hid_nk_asterisk		0x55	//�˺�,�Ǻż�"*"
#define	hid_nk_sub			0x56	//����"-"
#define	hid_nk_plus			0x57	//�Ӻ�"+"
#define	hid_nk_enter		0x58
#define	hid_nk_1			0x59
#define	hid_nk_2			0x5a
#define	hid_nk_3			0x5b
#define	hid_nk_4			0x5c
#define	hid_nk_5			0x5d
#define	hid_nk_6			0x5e
#define	hid_nk_7			0x5f
#define	hid_nk_8			0x60
#define	hid_nk_9			0x61
#define	hid_nk_0			0x62
#define	hid_nk_del			0x63

//Ӣ����ĸ��
#define	hid_a				0x04
#define	hid_b				0x5
#define	hid_c				0x6
#define	hid_d				0x7
#define	hid_e				0x8
#define	hid_f				0x9
#define	hid_g				0xA
#define	hid_h				0xb
#define	hid_i				0xc
#define	hid_j				0xd
#define	hid_k				0xe
#define	hid_l				0xf
#define	hid_m				0x10
#define	hid_n				0x11
#define	hid_o				0x12
#define	hid_p				0x13
#define	hid_q				0x14
#define	hid_r				0x15
#define	hid_s				0x16
#define	hid_t				0x17
#define	hid_u				0x18
#define	hid_v				0x19
#define	hid_w				0x1a
#define	hid_x				0x1b
#define	hid_y				0x1c
#define	hid_z				0x1d

//���ּ���
#define	hid_1				0x1e
#define	hid_2				0x1f
#define	hid_3				0x20
#define	hid_4				0x21
#define	hid_5				0x22
#define	hid_6				0x23
#define	hid_7				0x24
#define	hid_8				0x25
#define	hid_9				0x26
#define	hid_0				0x27

//��ϼ�
#define	hid_ctrl_l			0x01
#define	hid_shift_l			0x02
#define	hid_alt_l			0x04
#define	hid_win_l			0x08
#define	hid_ctrl_r			0x10
#define	hid_shift_r			0x20
#define	hid_alt_r			0x40
#define	hid_win_r			0x80


//#define	hid_ctrl_l			0xe0
//#define	hid_shift_l			0xE1
//#define	hid_alt_l			0xe2
//#define	hid_win_l			0xe3
//#define	hid_ctrl_r			0xe4
//#define	hid_shift_r			0xE5
//#define	hid_alt_r			0xe6
//#define	hid_win_r			0xe7


//type:���̹���
#define		_USA				0		//����
#define		_Turkey_Q			1		//������Q
#define 	_Spain				2		//������
#define 	_Portugal			3		//������
#define 	_France				4		//����
#define 	_Germany			5		//�¹�
#define 	_Italy				6		//�����
#define 	_UK					7		//Ӣ��


typedef struct
{
	unsigned char hid;
	unsigned char func;
}CONVERT, LANG;

typedef CONVERT CONVERT_p;

//extern const CONVERT_p *lang_ptr[];

/*******************************************************************************
* ������		: USB_KeySend.��ֻ���������ַ���
* ���� 		: ׼������USB��ֵ��
* ���� 		: Strs: ���յ��ַ���.
* ��� 		: None.
* ����ֵ		: 0��1.
*******************************************************************************/
extern uint8_t USB_KeySend( uint8_t *strs, uint8_t length );

/*******************************************************************************
* ������	: USB_KeySend.������������ԣ�
* ���� 	: ����USB��ֵ���������ַ����������������Լ���������
* ���� 	: 	send_buff: ���յ��ַ���.
			length:�ַ�������.
			type:���̹���
* ����ֵ	: 0��1.
* ע��	: �����������ʱ�����̹��ұ���ѡ��Ϊ_USA(��������)��
*******************************************************************************/
extern uint8_t USB_Key_Send(uint8_t *send_buff, uint16_t length, uint8_t type);

#endif /* __ASCII2HID_H__ */


