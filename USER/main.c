#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "uart4.h"
#include "timer.h"
#include "ads115.h"
#include "oled.h"
#include "dma.h"
#include "ADS1292.h"
#include "exti.h"
#include "mlx90614.h" 
#include "beep.h"
#include "adc.h"
#include "stdio.h"
#include "LCD.h"
#include "AD9954.h"
/************************************************

2020������ƾ���-TI��  �ۺϲ��Գ���
˵����������ֻ��2020 TI����Ҫ�õ��Ĵ��������ܲ��Գ���
���ߣ�����Ȼ
�汾��V1.1

					Ӳ��˵��
---------------------------------------------------
Ӳ������		|	���		|	����	|	��ע
---------------------------------------------------
USART		|	RXD			|	PA10	|
			|	TXD			|	PA9		|
---------------------------------------------------
UART4		|	RXD			|	PC11	|
			|	TXD			|	PC10	|
---------------------------------------------------
KEY			|	KEY_UP		|	PA4		|
			|	KEY_LEFT	|	PA5		|
			|	KEY_RIGHT	|	PA6		|
			|	KEY_DOWN	|	PA7		|
			|	KEY_SURE	|	PC4		|
			|	KEY_CANCEL	|	PC5		|
---------------------------------------------------
LED			|	LED_R		|	PC0		|
			|	LED_G		|	PC1		|
			|	LED_B		|	PC2		|
---------------------------------------------------
BEEP		|	BEEP		|	PC3		|
---------------------------------------------------
PWM			|	TIME4_CH1	|	PB6		|
			|	TIME4_CH2	|	PB7		|
			|	TIME4_CH3	|	PB8		|
			|	TIME4_CH4	|	PB9		|
---------------------------------------------------
ADC			|	ADC1_CH0	|	PA0		|
			|	ADC1_CH1	|	PA1		|
			|	ADC1_CH2	|	PA2		|
			|	ADC1_CH3	|	PA3		|
---------------------------------------------------
ADS1292		|	PWDN		|	PB10	|
			|	START		|	PB11	|
			|	DRDY		|	PC6		|
			|	CS0			|	PB12	|
			|	MOSI		|	PB15	|
			|	SCK			|	PB13	|
			|	MISO		|	PB14	|
---------------------------------------------------
MLX90614	|	SCL			|	PA8		|
			|	SDA			|	PC9		|
---------------------------------------------------
ADS1115 	|	SCL			|	PD2		|
			|	SDA			|	PC12	|
---------------------------------------------------
1.3 OLED	|	SCL			|	PB0		|
			|	SDA			|	PB1		|
---------------------------------------------------
FLASH		|	SCL			|	PC8		|
			|	SDA			|	PC7		|
---------------------------------------------------
USB    		|	USB_DP		|	PA12	|
			|	USB_DM		|	PA11	|


����������ļ������ݵ����壺
Code����ʾ������ռ�� FLASH �Ĵ�С��FLASH��
RO-data���� Read Only-data�� ��ʾ������ĳ������� const ���ͣ�FLASH��
RW-data���� Read Write-data�� ��ʾ�ѱ���ʼ����ȫ�ֱ�����SRAM��
ZI-data���� Zero Init-data�� ��ʾδ����ʼ����ȫ�ֱ���(SRAM)

************************************************/
extern unsigned char Plane_BMP[];   

int main(void)
{	
  /////////////////////////��FPGAͨ�Ų��ֱ���	
	int t_FPGA; 
  int FPGA_val;	 
	u16 len_1;	
	u16 times=0;
//	int res_FPGA_data[16];
	/////////////////////////
	u16 result;         	//����ADS1115�ı���
	
	float result_val;

	u16 adcx;           	//����MCU_ADC�ı���

	u16 len,t;          	//���ڴ���4�ı���

	u8 key;             	//���ڰ����ı���

	u8 test_flag = 0;   	//������ѡ�񰴼�

	float temp;         	//LMT70�¶ȱ���

	u8 i,sum;	    	//����ADS1292�ı���
	u8 data_to_send[60];	//���ڷ��ͻ���
	u32 cannle[2];	    	//�洢����ͨ��������
	s32	p_Temp[2];	    	//���ݻ���
	
  char  LCD_temp1[10];  //�¶ȴ�����
	char  LCD_temp2[10];  //�¶ȴ�����+ADS1115
  float temp1;

	data_to_send[0]=0xAA;  	//��λ��������������ͷ
	data_to_send[1]=0xAA;
	data_to_send[2]=0xF1;	
	data_to_send[3]=8;	 
	 
	delay_init();	    	 		//��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 	//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	     		//����һ��ʼ��Ϊ115200
	uart4_init(115200);	     		//�����ĳ�ʼ��Ϊ115200
	LED_Init();			     		//LED�˿ڳ�ʼ��
	KEY_Init();  					//������ʼ��
	TIM3_Int_Init(4999,7199);				//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms  
	ADS1115_Init();    						//ADS1115��ʼ��
  DMA_Config(DMA1_Channel4,(u32)&USART1->DR,(u32)data_to_send);	//����1DMA����
	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); 					//DMAʹ��	
	ADS1292_Init();	        				//��ʼ��ads1292
	EXTIX_Init();         					//��ʼ���ⲿ�ж����� 
	Adc_Init();		  						//ADC��ʼ��
	SMBus_Init();           				//MLX90614��ʼ��
  AD9954_Init();	
	LCD_Init();
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	delay_ms(1000);
	POINT_COLOR=BLUE;//��������
 	while(1)
	{
		////////////////////////////////////////////////////////////////////////�¶ȴ�������ʾ����
//		adcx=Get_Adc_Average(ADC_Channel_1,10);		//�õ�ADC1��ͨ��1
//    result=lvbo(0xe3,0xc2);
//    temp1=(float)(3.3/4096)*adcx;
//		POINT_COLOR=BLUE;//��������Ϊ��ɫ
//		sprintf(LCD_temp1,"%5.5f",temp1);
////  	LCD_ShowNum(250,500,temp1,12,12);
//    LCD_ShowString(152,150,200,16,16,LCD_temp1); //ֱ�����ַ���ʾ����
//    printf("ADC:%f\r\n",temp1);
///////////////////////////////////////////////////////////////////////ADS1115+�¶ȴ���������
//    result=lvbo(0xe3,0xc2);                 	//�õ�ͨ��һ��ֵ
//		result_val=4.096*2*result/65535*1000;     	//ת���ɵ�ѹ(��λmv)
//		POINT_COLOR=BLUE;//��������Ϊ��ɫ
//		sprintf(LCD_temp2,"%5.5f",result_val);
////  	LCD_ShowNum(250,500,temp1,12,12);
//    LCD_ShowString(152,150,200,16,16,LCD_temp2); //ֱ�����ַ���ʾ����
//		printf("ADC:%f\r\n",result_val);	
//		}
	 ////////////////////////////////////////////////////////DDS����
//		AD9954_SETFRE(1000);
	 ////////////////////////////////////////////////////////��FPGA����ͨ�Ų���
//		for(t_FPGA=0;t_FPGA<=100;t_FPGA++)
//	 {
//	  USART_SendData(USART1,0xcd);//�򴮿�1��������
//		LCD_ShowNum(200,500,USART_RX_BUF[0],16,16);
//		 if(t_FPGA==100)
//		 {
//		  t_FPGA=0;
//		 }
//	 }
///////////////////////////////////////////////////////////////�������
//     temp = Get_GY906();							//�õ��¶�ֵ
//     POINT_COLOR=RED;//��������Ϊ��ɫ
//     LCD_ShowNum(150,50,temp,16,16);
//   
	}	 

}
