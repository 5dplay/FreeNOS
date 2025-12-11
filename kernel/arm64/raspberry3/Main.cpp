/*
 * Copyright (C) 2025 Ivan Tan
 * Copyright (C) 2015 Niek Linnenbank
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/* https://github.com/bztsrc/raspi3-tutorial/blob/master/03_uart1 */

extern Address __start, __end, __bootimg;

#include <FreeNOS/System.h>
#include <FreeNOS/arm64/ARM64Kernel.h>
#include <MemoryBlock.h>
#include <arm64/ARM64Map.h>
#include <arm64/ARM64Paging.h>
#include <arm64/ARM64FirstTable.h>
#include <arm64/ARM64Exception.h>
#include <arm64/ARM64Control.h>
#include <DeviceLog.h>
#include "CoreInfo.h"
#include "BootImage.h"
#include "RaspberryKernel.h"
#include "Support.h"
#include "PL011.h"
#include "uart.h"
#include "mbox.h"

static char ALIGN(16 * 1024) SECTION(".data") tmpPageDir[sizeof(ARM64FirstTable)];

extern C int kernel_main(void)
{
    // set up serial console
    uart_init();

    unsigned long el;
    asm volatile("mrs %0, CurrentEL" : "=r" (el) : : "cc");
    el = (el >> 2) & 3;

    uart_puts("EL: ");
    uart_hex(el);
    uart_puts("\n");

    uart_puts("Hello World!\n");

    // Fill coreInfo
    BootImage *bootimage = (BootImage *) &__bootimg;
    MemoryBlock::set(&coreInfo, 0, sizeof(CoreInfo));
    coreInfo.bootImageAddress = (Address) (bootimage);
    coreInfo.bootImageSize    = bootimage->bootImageSize;
    coreInfo.kernel.phys      = (Address) &__start;
    coreInfo.kernel.size      = ((Address) &__end - (Address) &__start);
    coreInfo.memory.phys      = RAM_ADDR;
    coreInfo.memory.size      = RAM_SIZE;

    Arch::MemoryMap mem;
    uart_puts("1111!\n");
    ARM64Paging paging(&mem, (Address) &tmpPageDir, RAM_ADDR);
    uart_puts("2222!\n");

    // Activate MMU
    paging.initialize();
    uart_puts("3333!\n");
    paging.activate(true);
    uart_puts("4444!\n");

    // Clear BSS
    clearBSS();
    uart_puts("5555!\n");

    // Initialize heap
    Kernel::initializeHeap();
    uart_puts("6666!\n");
    uart_puts("6666.6666!\n");

    // Run all constructors first
    constructors();
    uart_puts("7777!\n");

    // Open the serial console as default Log
    PL011 pl011(UART0_IRQ);
    pl011.initialize();
    uart_puts("8888!\n");

    DeviceLog console(pl011);
    console.setMinimumLogLevel(Log::Notice);
    uart_puts("9999!\n");

    RaspberryKernel kernel(&coreInfo);

    uart_puts("1000!\n");
    return kernel.run();
}
