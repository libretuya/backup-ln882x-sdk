#ifndef __MEMORY_MAP_LN881X_H__
#define __MEMORY_MAP_LN881X_H__

#define SIZE_4KB                    (0x1000)
#define SIZE_8KB                    (2 * SIZE_4KB)
#define SIZE_16KB                   (4 * SIZE_4KB)
#define SIZE_32KB                   (8 * SIZE_4KB)
#define SIZE_64KB                   (16 * SIZE_4KB)
#define SIZE_128KB                  (32 * SIZE_4KB)
#define SIZE_2MB                    (32 * SIZE_64KB)

// Memory map is shown in the following figure:
// *-----------------------------------------------------------------------------------------------------*
// |   RAM_BLOCK0  |               RAM_BLOCK1                    |  RETENTION_MEM   | User unavailable   |
// *-----------------------------------------------------------------------------------------------------*
// |               |              |               |              |                  |                    |
// |    SRAM0 64KB |   SRAM1 64KB |   SRAM2 64KB  | SRAM3_0  32KB|  SRAM3_1  8KB    |    SRAM3_2  32KB   |
// |               |              |               |              |  retention mem   |    cache mem       |
// *-----------------------------------------------------------------------------------------------------*
// @0x1FFF0000     @0x20000000    @0x20010000     @0x20020000    @0x20028000        @0x2002A000          @0x20032000

//RAM
#define RAM_BASE					(0x1fff0000)
#define SRAM0_BASE                  (RAM_BASE)
#define SRAM0_SIZE                  (SIZE_64KB)
#define SRAM1_BASE                  (SRAM0_BASE + SRAM0_SIZE)
#define SRAM1_SIZE                  (SIZE_64KB)
#define SRAM2_BASE                  (SRAM1_BASE + SRAM1_SIZE)
#define SRAM2_SIZE                  (SIZE_64KB)
#define SRAM3_0_BASE                (SRAM2_BASE + SRAM2_SIZE)
#define SRAM3_0_SIZE                (SIZE_32KB)
#define SRAM3_1_BASE                (SRAM3_0_BASE + SRAM3_0_SIZE)
#define SRAM3_1_SIZE                (SIZE_8KB)

#define SRAM3_2_BASE                (SRAM3_1_BASE + SRAM3_1_SIZE)
#define SRAM3_2_SIZE                (SIZE_32KB)
#define RETENTION_MEM_BASE			(SRAM3_1_BASE)
#define RETENTION_MEM_SIZE			(SIZE_8KB)

#define CACHE_MEM_BASE				(SRAM3_2_BASE)
#define CACHE_MEM_SIZE				(SIZE_32KB)

#define RAM_TOP						(CACHE_MEM_BASE + CACHE_MEM_SIZE)

#define RAM_BLOCK0_BASE             (RAM_BASE)
#define RAM_BLOCK0_SIZE             (SRAM0_SIZE)
#define RAM_BLOCK1_BASE             (SRAM1_BASE)
#define RAM_BLOCK1_SIZE             (SRAM1_SIZE + SRAM2_SIZE + SRAM3_0_SIZE)

//ROM and BOOTROM
#define ROM_MEM_BASE                (0x00000000)
#define ROM_MEM_SIZE				(SIZE_64KB)
#define BOOTROM_BASE 				(ROM_MEM_BASE)
#define BOOTROM_LIMIT				(SIZE_4KB)
#define BOOTROM_RW_SECTION_BASE	    (SRAM2_BASE)
#define BOOTROM_RW_SECTION_LIMIT    (2 * SIZE_4KB)

//BOOTRAM
#define BOOTRAM_HEADER_IN_RAM_BASE          (RAM_BASE)
#define BOOTRAM_HEADER_SIZE                 (24)
#define BOOTRAM_IN_RAM_BASE			        (BOOTRAM_HEADER_IN_RAM_BASE + BOOTRAM_HEADER_SIZE)

//#define UART_DOWNLOAD_BASE	                (BOOT_RAM_BASE)
//#define UART_DOWNLOAD_SIZE	                (BOOT_RAM_SIZE)

//FLASH
#define FLASH_BASE                          (0x10000000)
#define FLASH_BASE_OFFSET                   (0)
#define BOOTRAM_HEADER_ON_FLASH_OFFSET      (FLASH_BASE_OFFSET)
#define BOOTRAM_ON_FLASH_OFFSET             (BOOTRAM_HEADER_ON_FLASH_OFFSET + BOOTRAM_HEADER_SIZE)
#define BOOTRAM_PARTITION_SIZE			    (8 * SIZE_4KB)
#define BOOTRAM_SIZE                        (BOOTRAM_PARTITION_SIZE - BOOTRAM_HEADER_SIZE)
#define PARTITION_TABLE0_OFFSET             (BOOTRAM_HEADER_ON_FLASH_OFFSET + BOOTRAM_PARTITION_SIZE)
#define PARTITION_TABLE0_SIZE               (SIZE_4KB)
#define PARTITION_TABLE1_OFFSET             (PARTITION_TABLE0_OFFSET + PARTITION_TABLE0_SIZE)
#define PARTITION_TABLE1_SIZE               (SIZE_4KB)
#define PARTITION_TABLE_FLAG_OFFSET         (PARTITION_TABLE1_OFFSET + PARTITION_TABLE1_SIZE)
#define PARTITION_TABLE_FLAG_SIZE           (SIZE_4KB)
#define SYSTEM_PARAMETER_OFFSET             (PARTITION_TABLE_FLAG_OFFSET + PARTITION_TABLE_FLAG_SIZE)
#define SYSTEM_PARAMETER_SIZE               (2 * SIZE_4KB)
//#define PRIMARY_PARTITION_OFFSET            (SYSTEM_PARAMETER_OFFSET + 2 * SYSTEM_PARAMETER_SIZE)
//#define FLASH_SIZE                          (SIZE_2MB)


#endif /* __MEMORY_MAP_LN881X_H__ */
