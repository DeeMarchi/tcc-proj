#pragma once

void ds18b20_task(void *args);
float ds18b20_get_current_temp();
char *ds18b20_get_current_temp_str();
void ds18b20_set_activation_temp(float temp);
float ds18b20_get_activation_temp();
