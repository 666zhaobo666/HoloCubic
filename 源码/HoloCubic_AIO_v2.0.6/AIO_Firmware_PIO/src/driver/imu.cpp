#include "imu.h"
#include "common.h"

const char *active_type_info[] = {"TURN_RIGHT", "RETURN",
                                  "TURN_LEFT", "UP",
                                  "DOWN", "GO_FORWORD",
                                  "HIBERNATE", "SHAKE", "UNKNOWN"};

IMU::IMU()
{
    action_info.isValid = false;
    action_info.active = ACTIVE_TYPE::UNKNOWN;
    action_info.long_time = true;
    // 初始化数据
    for (int pos = 0; pos < ACTION_HISTORY_BUF_LEN; ++pos)
    {
        // act_info_history.push_back(UNKNOWN);
        act_info_history[pos] = UNKNOWN;
    }
    act_info_history_ind = ACTION_HISTORY_BUF_LEN - 1;
    this->order = 0; // 表示方位
}

void IMU::init(uint8_t order, uint8_t auto_calibration,
               SysMpuConfig *mpu_cfg)
{
    this->setOrder(order); // 设置方向
    Wire.begin(IMU_I2C_SDA, IMU_I2C_SCL);
    Wire.setClock(400000);
    unsigned long timeout = 5000;
    unsigned long preMillis = millis();
    Serial.print("mpu6050 id is 0x");
    Serial.println(mpu.getDeviceID(),HEX);
    Serial.println("we need id is 0x0C ,default is 0x38");
    // mpu = MPU6050(0x68, &Wire);
    mpu = MPU6050(0x68);
    Serial.print(F("Unable to connect to 4.\n"));
    while (!mpu.testConnection() && !doDelayMillisTime(timeout, &preMillis, false))
        ;
    Serial.print(F("Unable to connect to 5.\n"));

    // if (!mpu.testConnection())
    // {
    //     Serial.print(F("Unable to connect to MPU6050.\n"));
    //     return;
    // }

    if (mpu.getDeviceID() != 0x38 && mpu.getDeviceID() != 0)
    {
        Serial.print(F("MPU6050 ID is false ... but alse try to connect mpu.\n"));
     }

    Serial.print(F("Initialization MPU6050 now, Please don't move.\n"));
    mpu.initialize();

    if (auto_calibration == 0)
    {
        Serial.print(F("MPU6050 User_calibration now\n"));
        // supply your own gyro offsets here, scaled for min sensitivity
        mpu.setXGyroOffset(mpu_cfg->x_gyro_offset);
        mpu.setYGyroOffset(mpu_cfg->y_gyro_offset);
        mpu.setZGyroOffset(mpu_cfg->z_gyro_offset);
        mpu.setXAccelOffset(mpu_cfg->x_accel_offset);
        mpu.setYAccelOffset(mpu_cfg->y_accel_offset);
        mpu.setZAccelOffset(mpu_cfg->z_accel_offset); // 1688 factory default for my test chip
    }
    else
    {
        Serial.print(F("MPU6050 auto_calibration now\n"));
        // 启动自动校准
        // 7次循环自动校正
        mpu.CalibrateAccel(7);
        mpu.CalibrateGyro(7);
        mpu.PrintActiveOffsets();

        mpu_cfg->x_gyro_offset = mpu.getXGyroOffset();
        mpu_cfg->y_gyro_offset = mpu.getYGyroOffset();
        mpu_cfg->z_gyro_offset = mpu.getZGyroOffset();
        mpu_cfg->x_accel_offset = mpu.getXAccelOffset();
        mpu_cfg->y_accel_offset = mpu.getYAccelOffset();
        mpu_cfg->z_accel_offset = mpu.getZAccelOffset();
    }

    Serial.print(F("Initialization MPU6050 success.\n"));
}

void IMU::setOrder(uint8_t order) // 设置方向
{
    this->order = order; // 表示方位
}

bool IMU::Encoder_GetIsPush(void)
{
#ifdef PEAK
    return (digitalRead(CONFIG_ENCODER_PUSH_PIN) == LOW);
#else
    return false;
#endif
}

ImuAction *IMU::update(int interval)
{
    getVirtureMotion6(&action_info);
    // 原先判断的只是加速度，现在要加上陀螺仪
    if (millis() - last_update_time > interval)
    {
        if (!action_info.isValid)
        {
            if (action_info.v_ay > 4000)
            {
                encoder_diff--;
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::TURN_LEFT;
            }
            else if (action_info.v_ay < -4000)
            {
                encoder_diff++;
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::TURN_RIGHT;
            }
            else if (action_info.v_ay > 1000 || action_info.v_ay < -1000)
            {
                // 震动检测
                encoder_diff--;
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::SHAKE;
            }
            else
            {
                action_info.isValid = 0;
            }
        }

        if (!action_info.isValid)
        {
            if (action_info.v_ax > 5000)
            {
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::UP;
                delay(500);
                getVirtureMotion6(&action_info);
                if (action_info.v_ax > 5000)
                {
                    action_info.isValid = 1;
                    action_info.active = ACTIVE_TYPE::GO_FORWORD;
                    encoder_state = LV_INDEV_STATE_PR;
                }
            }
            else if (action_info.v_ax < -5000)
            {
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::DOWN;
                delay(500);
                getVirtureMotion6(&action_info);
                if (action_info.v_ax < -5000)
                {
                    action_info.isValid = 1;
                    action_info.active = ACTIVE_TYPE::RETURN;
                    encoder_state = LV_INDEV_STATE_REL;
                }
            }
            else if (action_info.v_ax > 1000 || action_info.v_ax < -1000)
            {
                // 震动检测
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::SHAKE;
            }
            else
            {
                action_info.isValid = 0;
            }
        }

        last_update_time = millis();
    }
    return &action_info;
}

ImuAction *IMU::getAction(void)
{
    // 基本方法: 通过对近来的动作数据简单的分析，确定出动作的类型
    ImuAction tmp_info;
    getVirtureMotion6(&tmp_info);

    // Serial.printf("ax = %d \t ay = %d \r\n",tmp_info.v_ax,tmp_info.v_ay);

    tmp_info.active = ACTIVE_TYPE::UNKNOWN;

    // 原先判断的只是加速度，现在要加上陀螺仪
    if (ACTIVE_TYPE::UNKNOWN == tmp_info.active)
    {
        if (tmp_info.v_ay > 4000)
        {
            tmp_info.active = ACTIVE_TYPE::TURN_LEFT;
        }
        else if (tmp_info.v_ay < -4000)
        {
            tmp_info.active = ACTIVE_TYPE::TURN_RIGHT;
        }
        else if (tmp_info.v_ay > 1000 || tmp_info.v_ay < -1000)
        {
            // 震动检测
            tmp_info.active = ACTIVE_TYPE::SHAKE;
        }
    }

    if (ACTIVE_TYPE::UNKNOWN == tmp_info.active)
    {
        if (tmp_info.v_ax > 5000)
        {
            tmp_info.active = ACTIVE_TYPE::UP;
        }
        else if (tmp_info.v_ax < -5000)
        {
            tmp_info.active = ACTIVE_TYPE::DOWN;
        }
        else if (action_info.v_ax > 1000 || action_info.v_ax < -1000)
        {
            // 震动检测
            tmp_info.active = ACTIVE_TYPE::SHAKE;
        }
    }

    // 储存当前检测的动作数据到动作缓冲区中
    act_info_history_ind = (act_info_history_ind + 1) % ACTION_HISTORY_BUF_LEN;
    int index = act_info_history_ind;
    act_info_history[index] = tmp_info.active;

    // 本次流程的动作识别
    if (!action_info.isValid)
    {
        bool isHoldDown = false;// 长按的标志位
        bool isHibernate = false;//休眠标志位
        // 本次流程的动作识别
        int historical_state[50];
        for(int i=0;i<50;i++) historical_state[i] = (index + ACTION_HISTORY_BUF_LEN - 1 - i) % ACTION_HISTORY_BUF_LEN;
        // int second = (index + ACTION_HISTORY_BUF_LEN - 1) % ACTION_HISTORY_BUF_LEN;
        // int third = (index + ACTION_HISTORY_BUF_LEN - 2) % ACTION_HISTORY_BUF_LEN;
        // 先识别"短按" （注：不要写成else if）
        if (ACTIVE_TYPE::UNKNOWN != tmp_info.active)
        {
            action_info.isValid = 1;
            action_info.active = tmp_info.active;
        }
        // 识别"长按","长按"相对"短按"高级（所以键值升级放在短按之后）
        if (act_info_history[index] == act_info_history[historical_state[0]] 
            && act_info_history[historical_state[0]] == act_info_history[historical_state[1]])
        {
            // 目前只识别前后的长按
            if (ACTIVE_TYPE::UP == tmp_info.active)
            {
                isHoldDown = true;
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::GO_FORWORD;
            }
            else if (ACTIVE_TYPE::DOWN == tmp_info.active)
            {
                isHoldDown = true;
                action_info.isValid = 1;
                action_info.active = ACTIVE_TYPE::RETURN;
            }
            // 如需左右的长按可在此处添加"else if"的逻辑

            if(isHoldDown)
            {
            // 本次识别为长按，则手动清除识别过的历史数据 避免对下次动作识别的影响
            // act_info_history[0] = ACTIVE_TYPE::UNKNOWN;
            // act_info_history[1] = ACTIVE_TYPE::UNKNOWN;
            }
        }
        //识别为超长按，多用于检测人已经不在情况
        for(int i=0;i<40;i++){
            if(act_info_history[i] == act_info_history[index] 
                && act_info_history[index] != ACTIVE_TYPE::SHAKE
                && act_info_history[index] != ACTIVE_TYPE::UNKNOWN) 
                isHibernate = true;
            else {
                isHibernate = false;
                break;
            }
        }
        if(isHibernate){
            action_info.isValid = 1;
            action_info.active = ACTIVE_TYPE::HIBERNATE;
        }
    }

    return &action_info;
}

void IMU::getVirtureMotion6(ImuAction *action_info)
{
    mpu.getMotion6(&(action_info->v_ax), &(action_info->v_ay),
                   &(action_info->v_az), &(action_info->v_gx),
                   &(action_info->v_gy), &(action_info->v_gz));

    fixOriginDeflection6(action_info,&Imu_origin);  //偏向修正

    if (order & X_DIR_TYPE)
    {
        action_info->v_ax = -action_info->v_ax;
        action_info->v_gx = -action_info->v_gx;
    }

    if (order & Y_DIR_TYPE)
    {
        action_info->v_ay = -action_info->v_ay;
        action_info->v_gy = -action_info->v_gy;
    }

    if (order & Z_DIR_TYPE)
    {
        action_info->v_az = -action_info->v_az;
        action_info->v_gz = -action_info->v_gz;
    }

    if (order & XY_DIR_TYPE)
    {
        int16_t swap_tmp;
        swap_tmp = action_info->v_ax;
        action_info->v_ax = action_info->v_ay;
        action_info->v_ay = swap_tmp;
        swap_tmp = action_info->v_gx;
        action_info->v_gx = action_info->v_gy;
        action_info->v_gy = swap_tmp;
    }
}

//函数：使用原点修正陀螺仪采样
void IMU::fixOriginDeflection6(ImuAction *action_info, Imu_data *origin_data)
{
    action_info->v_ax -= origin_data->ax;
    action_info->v_ay -= origin_data->ay;
    action_info->v_az -= origin_data->az;
    action_info->v_gx -= origin_data->gx;
    action_info->v_gy -= origin_data->gy;
    action_info->v_gz -= origin_data->gz;
}


