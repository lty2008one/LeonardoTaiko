//头文件
typedef struct
{
    float ts;       //采样周期(s)
    float fc;       //下限频率(hz)
    float lastYn;   //上一次滤波值
    float lastXn;   //上一次采样值
    float alpha;    //滤波系数
} high_pass_filter_t;
//初始化滤波系数
void Init_highPass_alpha(high_pass_filter_t* const filter,const float ts, const float fc);
//低通滤波
float High_pass_filter(high_pass_filter_t* const filter, const float data);

//源文件

/*******************************************************************************
 * @fn Init_hightPass_alpha
 * @brief 初始高通滤波器滤波系数
 * @param filter 滤波器
 * @param ts 采用周期 单位s
 * @return fc 截至频率 单位hz
 ******************************************************************************/
void Init_highPass_alpha(high_pass_filter_t* const filter,const float ts, const float fc)
{
  float b=2*M_PI*fc*ts;
  filter->ts=ts;
  filter->fc=fc;
  filter->lastYn=0;
  filter->lastXn=0;
  filter->alpha=1/(1+b);
}

/*******************************************************************************
 * @fn Hight_pass_filter
 * @brief 高通滤波函数
 * @param data 采样数据
 * @return 滤波结果
 ******************************************************************************/
float High_pass_filter(high_pass_filter_t* const filter, const float data)
{
  float tem=((filter->alpha)*(filter->lastYn))+((filter->alpha)*(data-(filter->lastXn)));
  filter->lastYn=tem;
  filter->lastXn=data;
  return tem;
  
}

