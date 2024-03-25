//头文件
#ifndef M_PI
#define M_PI (3.141592f)
#endif
typedef struct
{
    float ts;       //采样周期(s)
    float fc;       //截至频率(hz)
    float lastYn;   //上一次滤波值
    float alpha;    //滤波系数
} low_pass_filter_t;
//初始化滤波系数
void Init_lowPass_alpha(low_pass_filter_t* const filter,const float ts, const float fc);
//低通滤波
float Low_pass_filter(low_pass_filter_t* const filter, const float data);

//源文件
/*******************************************************************************
 * @fn Init_lowPass_alpha
 * @brief 初始化低通滤波器滤波系数
 * @param filter 滤波器
 * @param ts 采用周期 单位s
 * @return fc 截至频率 单位hz
 ******************************************************************************/
void Init_lowPass_alpha(low_pass_filter_t* const filter,const float ts, const float fc)
{
  float b=2*M_PI*fc*ts;
  filter->ts=ts;
  filter->fc=fc;
  filter->lastYn=0;
  filter->alpha=b/(b+1);
}

/*******************************************************************************
 * @fn Low_pass_filter
 * @brief 低通滤波函数
 * @param data 采样数据
 * @return 滤波结果
 ******************************************************************************/
float Low_pass_filter(low_pass_filter_t* const filter, const float data)
{
  float tem=filter->lastYn+(filter->alpha*(data-filter->lastYn));
  filter->lastYn=tem;
  return tem;
  
}
