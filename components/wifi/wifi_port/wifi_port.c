#include "wifi_port.h"
#include "ln_kv_err.h"
#include "ln_kv_api.h"
#include "itypes.h"
#include "osal/osal.h"
#include "ln_nvds.h"
#include "ln_compiler.h"

/* hardware for MAC Interrupt */
void wlib_mac_isr_enable(void)
{
    NVIC_EnableIRQ(MAC_IRQn);
}

void wlib_mac_isr_disable(void)
{
    NVIC_DisableIRQ(MAC_IRQn);
}

/* hardware for EVM test, temp cal */
#include "hal/hal_timer.h"
#include "hal/hal_syscon.h"
#include "console.h"
#include <stdarg.h>

void wlib_hwtimer_init(void * timer_cb, uint32_t period_us)
{
    TIMER_InitTypeDef hwtimer_cfg;

    hwtimer_cfg.index            = TIMER_1;
    hwtimer_cfg.mask             = TIMER_MASKED_NO;
    hwtimer_cfg.mode             = TIMER_MODE_FREERUNNING;//TIMER_MODE_USERDEFINED
    hwtimer_cfg.user_freq        = 1000 * 1000;
	hwtimer_cfg.timer_cb.cb_func = (timer_cb_func_t)timer_cb;
	hwtimer_cfg.timer_cb.arg     = NULL;

    HAL_TIMER_Init(&hwtimer_cfg);
    HAL_TIMER_LoadCount_Set((TIMER_Index)hwtimer_cfg.index, (hwtimer_cfg.user_freq/1000000)*period_us);
}

void wlib_hwtimer_start(void)
{
    NVIC_EnableIRQ(TIMER_IRQn);
    HAL_TIMER_Enable(TIMER_1, TIMER_ENABLE);
}

void wlib_hwtimer_stop(void)
{
    //NVIC_DisableIRQ(TIMER_IRQn);
    HAL_TIMER_Enable(TIMER_1, TIMER_DISABLE);
}

void wlib_software_reset_core(void)
{
    HAL_SYSCON_SoftwareResetCore(SW_RST_CORE_ALL);
}

void wlib_pvtcmd_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    __sprintf(NULL, (stdio_write_fn)command_stdio_write, format, args);
    va_end(args);
}

void wlib_xtal40m_cap_set(uint8_t cap)
{
    HAL_SYSCON_Xtal40MCap_Set(cap);
}

void wlib_xtal40m_vol_set(uint8_t vol)
{
    HAL_SYSCON_XtalVol_Set(vol);
}

/* misc */
#include "utils/art_string.h"
uint8_t wlib_char2hex(char c)
{
    return ln_char2hex(c);
}

int wlib_str2bytes(uint8_t *bytes, const char *str)
{
    return ln_str2bytes(bytes, str);
}

int wlib_is_valid_mac_str(const char* mac_str)
{
    return ln_is_valid_mac_str(mac_str);
}

int wlib_is_valid_mac(const char *mac)
{
    return ln_is_valid_mac(mac);
}

int wlib_mac_str2hex(const uint8_t *str, uint8_t *hex)
{
    return ln_mac_str2hex(str, hex);
}

/* restore/store psk info */
int wlib_kv_set_psk_info_list(uint8_t *v_buf, uint32_t buf_len)
{
    if (KV_ERR_NONE != ln_kv_set((const char *)KV_PSK_INFO, v_buf, (size_t)buf_len)) {
        return BFALSE;
    }
    return BTRUE;
}

int wlib_kv_get_psk_info_list(uint8_t *v_buf, uint32_t buf_len)
{
    size_t len;
    if (KV_ERR_NONE != ln_kv_get((const char *)KV_PSK_INFO, v_buf, (size_t)buf_len, &len)) {
        return BFALSE;
    }
    return BTRUE;
}

int wlib_kv_has_psk_info_list(void)
{
    if (LN_TRUE == ln_kv_has_key((const char *)KV_PSK_INFO)) {
        return BTRUE;
    }
    return BFALSE;
}

/* tx power external compensation */
int wlib_get_tx_power_ext_comp_val(int8_t *val)
{
    uint8_t rd_val = 0;
    
    if (NVDS_ERR_OK != ln_nvds_get_tx_power_comp(&rd_val)) {
        return BTRUE;
    }
    
    if (rd_val == 0xFF) {
        return BTRUE;
    }
    
    *val = (int8_t)rd_val;
    return BTRUE;
}

/* heap memory manager */
void *wlib_malloc(uint32_t size)
{
    return OS_Malloc(size);
}

void wlib_free(void *ptr)
{
    OS_Free(ptr);
}

/* log print, hexdump */
void wlib_log_printf(uint8_t tag_en, uint8_t level, const char *format, ...)
{
    const char *tag = NULL;
    va_list args;

    va_start(args, format);
    if (tag_en) {
        switch(level) {
            case WLIB_LOG_E:
                tag = WLIB_LOG_E_TAG;
                break;
            case WLIB_LOG_I:
                tag = WLIB_LOG_I_TAG;
                break;
            case WLIB_LOG_W:
                tag = WLIB_LOG_W_TAG;
                break;
            case WLIB_LOG_D:
                tag = WLIB_LOG_D_TAG;
                break;
            default:
                break;
        }
    }

    if (level <= WLIB_LOG_LEVEL_CFG) {
        __sprintf(tag, (stdio_write_fn)log_stdio_write, format, args);
    }
    va_end(args);
}

void wlib_hexdump(const char *info, void *buf, uint32_t buf_len)
{
    hexdump(LOG_LVL_ERROR, (uint8_t *)info, buf, buf_len);
}

void wlib_assert(int expr, const char *fun, int line)
{
    if (!expr){
        __disable_irq();
        wlib_log_printf(0, WLIB_LOG_E, "[WLIB_ASSERT]fun:%s, line:%d\r\n", fun, line);
        __BKPT(0);
        while(1);
    }
}

/* sniffer pool buf */
#include "utils/ln_mem_pool.h"
#define SNIFFER_MEM_POOL_USE_DYNAMIC_MEM   (1)
#define SNIFFER_MEM_POOL_CHUNK_CNT         (200)
#define SNIFFER_MEM_POOL_CHUNK_BUF_SIZE    (30)
#define SNIFFER_MEM_POOL_CHUNK_SIZE        (MEM_POOL_CHUNK_INFO_SIZE + SNIFFER_MEM_POOL_CHUNK_BUF_SIZE)
static ln_mem_pool_t sniffer_mem_pool = {0};

int wlib_sniffer_mem_pool_init(void)
{
#if (defined(SNIFFER_MEM_POOL_USE_DYNAMIC_MEM) && SNIFFER_MEM_POOL_USE_SYNC_MEM)
    sniffer_mem_pool.mem_base       = (uint8_t  *)OS_Malloc(SNIFFER_MEM_POOL_CHUNK_CNT * SNIFFER_MEM_POOL_CHUNK_SIZE);
    sniffer_mem_pool.free_chunk_ptr = (uint8_t **)OS_Malloc(SNIFFER_MEM_POOL_CHUNK_CNT * sizeof(void *));
    if (!sniffer_mem_pool.mem_base || !sniffer_mem_pool.free_chunk_ptr) {
        WLIB_LOG(WLIB_LOG_E, "sniffer mem pool use dynamic mem(OS_Malloc) failed.\r\n");
        return LN_FALSE;
    }
#else
    static uint8_t  g_sniffer_mem_block[SNIFFER_MEM_POOL_CHUNK_CNT * SNIFFER_MEM_POOL_CHUNK_SIZE] = {0};
    static uint8_t *sniffer_mem_pool_chunk_ptr[SNIFFER_MEM_POOL_CHUNK_CNT] = {0};
    sniffer_mem_pool.mem_base         = g_sniffer_mem_block;
    sniffer_mem_pool.free_chunk_ptr   = (uint8_t **)sniffer_mem_pool_chunk_ptr;    
#endif

    sniffer_mem_pool.total_chunk_cnt  = SNIFFER_MEM_POOL_CHUNK_CNT;
    sniffer_mem_pool.chunk_size       = SNIFFER_MEM_POOL_CHUNK_SIZE;
    ln_mem_pool_init(&sniffer_mem_pool);

    return LN_TRUE;
}

void wlib_sniffer_mem_pool_deinit(void)
{
#if (defined(SNIFFER_MEM_POOL_USE_DYNAMIC_MEM) && SNIFFER_MEM_POOL_USE_SYNC_MEM)
    OS_Free(sniffer_mem_pool.mem_base);
    OS_Free(sniffer_mem_pool.free_chunk_ptr);
#endif
}

void *wlib_sniffer_mem_pool_alloc(void)
{
    return ln_mem_pool_alloc(&sniffer_mem_pool);
}

int wlib_sniffer_mem_pool_free(void *addr)
{
    return ln_mem_pool_free(&sniffer_mem_pool, addr);
}

uint16_t wlib_sniffer_mem_chunk_count_get(void)
{
    return SNIFFER_MEM_POOL_CHUNK_CNT;
}

uint16_t wlib_sniffer_mem_chunk_buf_size_get(void)
{
    return SNIFFER_MEM_POOL_CHUNK_BUF_SIZE;
}


/* cpu sleep voter register */
#include "hal/hal_sleep.h"
void wlib_cpu_sleep_voter_reg(void *vote_check, void *pre_sleep, void *post_sleep)
{
    hal_sleep_register(MOD_MAC, (sleep_condition_cb_t)vote_check, (sleep_processing_cb_t)pre_sleep, (sleep_processing_cb_t)post_sleep);
}


/* sdio host interface */
#include "hal/hal_sdio.h"
__WEAK__ void sdio_if_init(void *wlib_recv_from_sdio, void **wlib_send_to_sdio, void *pkt_buf_malloc){};
__WEAK__ void sdio_if_reset(void){};

void wlib_sdio_if_init(void *wlib_recv_from_sdio, void **wlib_send_to_sdio, void *pkt_buf_malloc)
{
    sdio_if_init(wlib_recv_from_sdio, wlib_send_to_sdio, pkt_buf_malloc);
}

void wlib_sdio_if_reset(void)
{
    sdio_if_reset();
}

/* os service */
wlib_thread_t wlib_os_thread_creat(const char *name, void *entry, void *arg, uint32_t stksize, uint32_t prio)
{
    WLIB_ASSERT(entry);
    OS_Thread_t *thread = OS_Malloc(sizeof(OS_Thread_t));
    WLIB_ASSERT(thread);
    memset(thread, 0, sizeof(OS_Thread_t));

    if (OS_OK != OS_ThreadCreate(thread, name, (OS_ThreadEntry_t)entry, arg, (OS_Priority)prio, stksize)) {
        WLIB_LOG(WLIB_LOG_E, "Create <%s> thread failed.\r\n", name);
        return NULL;
    }
    return (wlib_thread_t)thread;
}

void wlib_os_thread_delete(wlib_thread_t *pthread)
{
    WLIB_ASSERT(pthread);
    if(*pthread != NULL) {
        OS_ThreadDelete((OS_Thread_t *)*pthread);
        OS_Free(*pthread);
        *pthread = NULL;
    }
}

wlib_queue_t wlib_os_queue_creat(uint32_t item_count, uint32_t item_size)
{
    OS_Queue_t *queue = OS_Malloc(sizeof(OS_Queue_t));
    WLIB_ASSERT(queue);
    memset(queue, 0, sizeof(OS_Queue_t));

    if (OS_OK != OS_QueueCreate(queue, item_count, item_size)) {
        WLIB_LOG(WLIB_LOG_E, "Create queue failed.\r\n");
        return NULL;
    }
    return (wlib_queue_t)queue;
}

void wlib_os_queue_delete(wlib_queue_t *pqueue)
{
    WLIB_ASSERT(pqueue);
    if(*pqueue != NULL) {
        OS_QueueDelete((OS_Queue_t *)*pqueue);
        OS_Free(*pqueue);
        *pqueue = NULL;
    }
}

int wlib_os_queue_send(wlib_queue_t *pqueue, const void *item, uint32_t timeout_ms)
{
    if (OS_OK != OS_QueueSend((OS_Queue_t *)*pqueue, item, timeout_ms)) {
        return WIFI_ERR_OS_SERVICE;
    }
    return WIFI_ERR_NONE;
}

int wlib_os_queue_recv(wlib_queue_t *pqueue, void *item, uint32_t timeout_ms)
{
    if (OS_OK != OS_QueueReceive((OS_Queue_t *)*pqueue, item, timeout_ms)) {
        return WIFI_ERR_OS_SERVICE;
    }
    return WIFI_ERR_NONE;
}

wlib_mutex_t wlib_os_mutex_creat(void)
{
    OS_Mutex_t *mutex = OS_Malloc(sizeof(OS_Mutex_t));
    WLIB_ASSERT(mutex);
    memset(mutex, 0, sizeof(OS_Mutex_t));

    if (OS_OK != OS_MutexCreate(mutex)) {
        WLIB_LOG(WLIB_LOG_E, "Create mutex failed.\r\n");
        return NULL;
    }
    return (wlib_mutex_t)mutex;
}

void wlib_os_mutex_delete(wlib_mutex_t *pmutex)
{
    WLIB_ASSERT(pmutex);
    if(*pmutex != NULL) {
        OS_MutexDelete((OS_Mutex_t *)*pmutex);
        OS_Free(*pmutex);
        *pmutex = NULL;
    }
}

int wlib_os_mutex_lock(wlib_mutex_t *pmutex, uint32_t timeout_ms)
{
    if (OS_OK != OS_MutexLock((OS_Mutex_t *)*pmutex, timeout_ms)) {
        return WIFI_ERR_OS_SERVICE;
    }
    return WIFI_ERR_NONE;
}

int wlib_os_mutex_unlock(wlib_mutex_t *pmutex)
{
    if (OS_OK != OS_MutexUnlock((OS_Mutex_t *)*pmutex)) {
        return WIFI_ERR_OS_SERVICE;
    }
    return WIFI_ERR_NONE;
}

wlib_sem_t wlib_os_sem_create(uint32_t max_count, uint32_t init_count)
{
    OS_Semaphore_t *sem = OS_Malloc(sizeof(OS_Semaphore_t));
    WLIB_ASSERT(sem);
    memset(sem, 0, sizeof(OS_Semaphore_t));

    if (OS_OK != OS_SemaphoreCreate(sem, init_count, max_count)) {
        WLIB_LOG(WLIB_LOG_E, "Create semaphore failed.\r\n");
        return NULL;
    }
    return (wlib_sem_t)sem;
}

void wlib_os_sem_delete(wlib_sem_t *psem)
{
    WLIB_ASSERT(psem);
    if(*psem != NULL) {
        OS_SemaphoreDelete((OS_Semaphore_t *)*psem);
        OS_Free(*psem);
        *psem = NULL;
    }
}

int wlib_os_sem_wait(wlib_sem_t *psem, uint32_t timeout_ms)
{
    if (OS_OK != OS_SemaphoreWait((OS_Semaphore_t *)*psem, timeout_ms)) {
        return WIFI_ERR_OS_SERVICE;
    }
    return WIFI_ERR_NONE;
}

int wlib_os_sem_release(wlib_sem_t *psem)
{
    if (OS_OK != OS_SemaphoreRelease((OS_Semaphore_t *)*psem)) {
        return WIFI_ERR_OS_SERVICE;
    }
    return WIFI_ERR_NONE;
}

wlib_timer_t wlib_os_timer_creat(void *timer_cb, uint32_t data)
{
    WLIB_ASSERT(timer_cb);

    OS_Timer_t *timer = OS_Malloc(sizeof(OS_Timer_t));
    WLIB_ASSERT(timer);
    memset(timer, 0, sizeof(OS_Timer_t));

    if (OS_OK != OS_TimerCreate(timer, OS_TIMER_ONCE, (OS_TimerCallback_t)timer_cb, (void *)data, 200)) {
        WLIB_LOG(WLIB_LOG_E, "Create timer failed.\r\n");
        return NULL;
    }
    return (wlib_timer_t)timer;
}

void wlib_os_timer_delete(wlib_timer_t *ptimer)
{
    WLIB_ASSERT(ptimer);
    if(*ptimer != NULL) {
        OS_TimerDelete((OS_Timer_t *)*ptimer);
        OS_Free(*ptimer);
        *ptimer = NULL;
    }
}

void wlib_os_timer_start(wlib_timer_t timer, uint32_t ms)
{
    WLIB_ASSERT(timer);
    WLIB_ASSERT(ms > 0);
    OS_TimerChangePeriod((OS_Timer_t *)timer, ms);
}

void wlib_os_timer_stop(wlib_timer_t timer)
{
    WLIB_ASSERT((OS_Timer_t *)timer);
    OS_TimerStop((OS_Timer_t *)timer);
}

void wlib_os_delay_ms(uint32_t ms)
{
    OS_MsDelay(ms);
}



