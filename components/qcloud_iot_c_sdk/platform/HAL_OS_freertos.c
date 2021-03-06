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

#include "FreeRTOS.h"
#include "qcloud_iot_import.h"

//TODO platform dependant
void HAL_SleepMs(_IN_ uint32_t ms)
{
  TickType_t ticks = ms / portTICK_PERIOD_MS;

  vTaskDelay(ticks ? ticks : 1);          /* Minimum delay = 1 tick */

  return ;
}

int HAL_random(void *p_rng, unsigned char *output, size_t output_len)
{
	int i;
	for(i = 0; i < output_len; i ++)	{
		output[i] = xTaskGetTickCount( )&0xff;
	}
	
	return 0;
}

int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int rc;

    va_start(args, fmt);
    rc = vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

int HAL_Vsnprintf(_IN_ char *str, _IN_ const int len, _IN_ const char *format, va_list ap)
{
    return vsnprintf(str, len, format, ap);
}


void *HAL_Malloc(_IN_ uint32_t size)
{
	return pvPortMalloc( size);
}

void HAL_Free(_IN_ void *ptr)
{
    if (ptr)
        vPortFree(ptr);
}

void* HAL_Calloc(size_t nelements, size_t elementSize)
{
	size_t size;
	void *ptr = NULL;

	size = nelements * elementSize;
	ptr = pvPortMalloc(size);

	if(ptr)
		memset(ptr, 0, size);

	return ptr;
}

void *HAL_MutexCreate(void)
{
#ifdef MULTITHREAD_ENABLED
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    if (NULL == mutex) {
        HAL_Printf("%s: xSemaphoreCreateMutex failed\n", __FUNCTION__);
        return NULL;
    }

    return mutex;
#else
    return (void *)0xFFFFFFFF;
#endif
}

void HAL_MutexDestroy(_IN_ void *mutex)
{
#ifdef MULTITHREAD_ENABLED
    if (xSemaphoreTake(mutex, 0) != pdTRUE) {
        HAL_Printf("%s: xSemaphoreTake failed\n", __FUNCTION__);
    }

    vSemaphoreDelete(mutex);
#else
    return;
#endif
}

void HAL_MutexLock(_IN_ void *mutex)
{
#ifdef MULTITHREAD_ENABLED
    if (!mutex) {
        HAL_Printf("%s: invalid mutex\n", __FUNCTION__);
        return;
    }

    if (xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) {
        HAL_Printf("%s: xSemaphoreTake failed\n", __FUNCTION__);
        return;
    }
#else
    return;
#endif
}

int HAL_MutexTryLock(_IN_ void *mutex)
{
#ifdef MULTITHREAD_ENABLED
    if (!mutex) {
        HAL_Printf("%s: invalid mutex\n", __FUNCTION__);
        return -1;
    }
    
    if (xSemaphoreTake(mutex, 0) != pdTRUE) {
        HAL_Printf("%s: xSemaphoreTake failed\n", __FUNCTION__);
        return -1;
    }
    
    return 0;
#else
    return 0;
#endif
}


void HAL_MutexUnlock(_IN_ void *mutex)
{
#ifdef MULTITHREAD_ENABLED
    if (!mutex) {
        HAL_Printf("%s: invalid mutex\n", __FUNCTION__);
        return ;
    }

    if (xSemaphoreGive(mutex) != pdTRUE) {
        HAL_Printf("%s: xSemaphoreGive failed\n", __FUNCTION__);
        return;
    }
#else
    return;
#endif
}

#ifdef MULTITHREAD_ENABLED

// platform-dependant thread routine/entry function
static void _HAL_thread_func_wrapper_(void *ptr)
{
    ThreadParams *params = (ThreadParams *)ptr;

    params->thread_func(params->user_arg);

    vTaskDelete(NULL);
}

// platform-dependant thread create function
int HAL_ThreadCreate(ThreadParams *params)
{
    if (params == NULL)
        return QCLOUD_ERR_INVAL;

    if (params->thread_name == NULL) {
        HAL_Printf("thread name is required for FreeRTOS platform!\n");
        return QCLOUD_ERR_INVAL;
    }

    int ret = xTaskCreate(_HAL_thread_func_wrapper_, params->thread_name, params->stack_size, (void *)params,
                          params->priority, (void *)&params->thread_id);
    if (ret != pdPASS) {
        HAL_Printf("%s: xTaskCreate failed: %d\n", __FUNCTION__, ret);
        return QCLOUD_ERR_FAILURE;
    }

    return QCLOUD_RET_SUCCESS;
}

#endif
