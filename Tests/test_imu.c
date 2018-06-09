#include "test_imu.h"
#include "cmsis_os.h"

void test_imu(void){
    onboard_imu_lib_init();
    while (1) {
        onboard_imu_update();
        print_imu_data();
        osDelay(20);
    }
}
