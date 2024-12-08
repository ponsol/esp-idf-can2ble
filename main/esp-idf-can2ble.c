#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"

#include "bleperi.c"
#include "cantrans.c"

void app_main()
{
    nvs_flash_init();
    bleperi_main();
    cantrans_main();
}
