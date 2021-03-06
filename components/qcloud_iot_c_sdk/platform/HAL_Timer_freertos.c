/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "FreeRTOS.h"
#include "qcloud_iot_import.h"

#include "SEGGER_SYSVIEW.h"

uint32_t HAL_GetTimeMs(void)
{
	return xTaskGetTickCount() * (1000/configTICK_RATE_HZ);
}

/*Get timestamp*/
long HAL_Timer_current_sec(void)
{
	return HAL_GetTimeMs()/1000;
}

char *HAL_Timer_current(char *time_str)
{
	long uiTime = 0;
	uiTime = HAL_Timer_current_sec();
    snprintf(time_str, TIME_FORMAT_STR_LEN, "%ld", uiTime);
    return time_str;

}

bool HAL_Timer_expired(Timer *timer)
{
    uint32_t now_ts;

	now_ts	= HAL_GetTimeMs();

	SEGGER_SYSVIEW_PrintfHost("now ms: %u, end ms: %u, expired: %d",
		now_ts, timer->end_time, (now_ts > timer->end_time) ? 1 : 0);

    return (now_ts > timer->end_time)?true:false;
}

void HAL_Timer_countdown_ms(Timer *timer, unsigned int timeout_ms)
{
	timer->end_time = HAL_GetTimeMs();
    timer->end_time += timeout_ms;
}

void HAL_Timer_countdown(Timer *timer, unsigned int timeout)
{
    timer->end_time = HAL_GetTimeMs();
	timer->end_time += timeout*1000;
}

int HAL_Timer_remain(Timer *timer)
{
	 return (int)(timer->end_time - HAL_GetTimeMs());
}

void HAL_Timer_init(Timer *timer)
{
      timer->end_time = 0;
}

