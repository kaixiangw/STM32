#include "software_iic.h"

void delay_us(uint16_t nus)
{ 
	__HAL_TIM_SET_COUNTER(DLY_TIM_Handle, 0);  // set the counter value a 0
	__HAL_TIM_ENABLE(DLY_TIM_Handle);          // start the counter
  // wait for the counter to reach the us input in the parameter
	while (__HAL_TIM_GET_COUNTER(DLY_TIM_Handle) < nus);
	__HAL_TIM_DISABLE(DLY_TIM_Handle);         // stop the counter
}

void IIC_Init(void)
{
  // ��ʼ��SCL��SDAΪ��©���
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  IIC_SDA_GPIO_CLK_ENABLE();
  IIC_SCL_GPIO_CLK_ENABLE(); 

  GPIO_InitStruct.Pin = IIC_SCL_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(IIC_SCL_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = IIC_SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(IIC_SDA_PORT, &GPIO_InitStruct);

  // ��ʼ��SCL��SDAΪ�ߵ�ƽ
  IIC_SCL_WRITE_UP();
  IIC_SDA_WRITE_UP();
}

// SDA���ģʽ
void IIC_SDA_OutputMode(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  IIC_SDA_GPIO_CLK_ENABLE();

  GPIO_InitStruct.Pin = IIC_SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(IIC_SDA_PORT, &GPIO_InitStruct);
}

// SDA����ģʽ
void IIC_SDA_InputMode(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  IIC_SDA_GPIO_CLK_ENABLE();

  GPIO_InitStruct.Pin = IIC_SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;

  HAL_GPIO_Init(IIC_SDA_PORT, &GPIO_InitStruct);
}

// IIC��ʼ�ź�
void IIC_StartSignal(void)
{
  IIC_SDA_OutputMode();  // ����SDAΪ���ģʽ

  // ȷ��SCL��SDA���Ǹߵ�ƽ 
  IIC_SCL_WRITE_UP();
  IIC_SDA_WRITE_UP();

  // ����SDA������һ���½���
  // һ�㳣�õ�IIC���߱�׼����Ϊ100kHz����ÿ��ʱ������Ϊ10us����SDA�͵�ƽӦ����5us
  IIC_SDA_WRITE_DOWN();  // SDA����
  delay_us(5);

 
  IIC_SCL_WRITE_DOWN();   // ����SCL����ʾ׼��ͨ��
}

// IICֹͣ�ź�
void IIC_StopSignal(void)
{
  IIC_SDA_OutputMode();  // ����SDAΪ���ģʽ

  // ȷ��SCL��SDA���ǵ͵�ƽ
  IIC_SCL_WRITE_DOWN();
  IIC_SDA_WRITE_DOWN();

  // ����SCL������һ��������
  // һ�㳣�õ�IIC���߱�׼����Ϊ100kHz����ÿ��ʱ������Ϊ10us����SCL�ߵ�ƽӦ����5us
  IIC_SCL_WRITE_UP();
  delay_us(5);
  
  IIC_SDA_WRITE_UP(); // ����SDA����ʾͨ�Ž���
  delay_us(5);        // ȷ��SDA�ĵ�ƽ���Ա�����������⵽
}

// ������������
void IIC_SendBytes(uint8_t Data)
{
  uint8_t i = 0;

  IIC_SDA_OutputMode();  // ����SDAΪ���ģʽ
      
  // ȷ��SCL��SDA���ǵ͵�ƽ
  IIC_SCL_WRITE_DOWN();
  IIC_SDA_WRITE_DOWN();

  // ��ʼ����8λ����
  for (i = 0; i < 8; i++)
  {
    // SCL�͵�ƽ�ڼ�����׼������
    if (Data & (1 << (7 - i))) // �ж����ݵĵ�7-iλ�Ƿ�Ϊ1
    {
      IIC_SDA_WRITE_UP();  // ���Ϊ1��SDA����
    }
    else
    {
      IIC_SDA_WRITE_DOWN();// ���Ϊ0��SDA����
    }
    delay_us(5); // ���ˣ�����׼�����

    // ����SCL��������������
    IIC_SCL_WRITE_UP();    
    delay_us(5); // ���ˣ����ݷ������

    // ����SCL��׼��������һ������
    IIC_SCL_WRITE_DOWN();
    delay_us(5);
  }
}

// ������������
uint8_t IIC_ReadBytes(void)
{
  uint8_t i = 0;
  uint8_t Data = 0; // ���ڴ洢���յ�������

  IIC_SDA_InputMode();  // ����SDAΪ����ģʽ

  IIC_SCL_WRITE_DOWN(); // ȷ��SCLΪ�͵�ƽ

  // ��ʼ����8λ����
  for (i = 0; i < 8; i++)
  {
    // ����SCL������׼����������
    IIC_SCL_WRITE_UP();
    delay_us(5);  // ���ˣ��ӻ�����׼����ϣ�������ʼ����

    if (IIC_SDA_READ() == 1)   // �����յ�1
    {
      Data |= (1 << (7 - i));  // ���յ���1�洢��Data�ĵ�7-iλ
    }

    /* ����Data��ʼ��Ϊ0000 0000�����Բ���Ҫelse���
    else // �յ�0
    {
      Data &= ~(1 << (7 - i)); // ���յ���0�洢��Data�ĵ�7-iλ
    }
    */
    
    // ����SCL������׼��������һ������
    IIC_SCL_WRITE_DOWN();
    delay_us(5);
  }
  return Data;  // ���ؽ��յ�������
}

// �����������ݣ��ӻ�����Ӧ��
uint8_t IIC_WaitACK(void)
{
  uint8_t ack;
  IIC_SDA_InputMode();  // ����SDAΪ����ģʽ

  IIC_SCL_WRITE_DOWN(); // ȷ��SCL�ǵ͵�ƽ
  delay_us(5); 

  IIC_SCL_WRITE_UP();  // ����SCL������׼�����մӻ���Ӧ���ź�
  delay_us(5);         // ���ˣ��ӻ�Ӧ���ź�׼����ϣ�������ʼ����

  // ����ӻ�Ӧ���ź�Ϊ0����ʾ�ӻ����յ�����
  if (IIC_SDA_READ() == 0)
  {
    ack = 0;
  }
  else // ����ӻ�Ӧ���ź�Ϊ1����ʾ�ӻ�û�н��յ�����
  {
    ack = 1;
  }
  
  IIC_SCL_WRITE_DOWN(); // ����SCL��������������
  delay_us(5);

  return ack;  // ���شӻ���Ӧ���ź�
}

// �ӻ��������ݣ���������Ӧ��0��ʾӦ��1��ʾ��Ӧ��
void IIC_MasterACK(uint8_t ack)
{
  IIC_SDA_OutputMode();  // ����SDAΪ���ģʽ

  // ȷ��SCL��SDA���ǵ͵�ƽ
  IIC_SCL_WRITE_DOWN();
  IIC_SDA_WRITE_DOWN();

  if (ack == 0) // ���ackΪ0����ʾ����Ӧ��
  {
    IIC_SDA_WRITE_DOWN(); // SDA����
  }
  else // ���ackΪ1����ʾ������Ӧ��
  {
    IIC_SDA_WRITE_UP();   // SDA����
  }
  delay_us(5);  //  ���ˣ�Ӧ���ź�׼�����

  // ����SCL����������Ӧ���ź�
  IIC_SCL_WRITE_UP();
  delay_us(5);  // ���ˣ�Ӧ���źŷ������

  // ����SCL���ӻ���������
  IIC_SCL_WRITE_DOWN();
  delay_us(5);
}
