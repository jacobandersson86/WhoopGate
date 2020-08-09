/* 
 * Flash Config Console Commands
 * 
 * This part is for implementing a CLI for the API in the 
 * Flash Config library (flash_config.c/flash_config.h).
 * 
 * File: cmd_flash_config.c
 * Author: Jacob Andersson
 * Date: 2019-07-04
 * Company: Plejd AB
 * Project: PMStep
 *
*/

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "esp_err.h"
#include "flash_config.h"

static const char* TAG = "cmd_flash_config";
//---------------- Argument structs here! -------------
static struct
{
	struct arg_dbl * km;
	struct arg_dbl * L;
	struct arg_dbl * R;
	struct arg_int * Nr;
	struct arg_dbl * i;
	struct arg_end *end;
} set_motor_params_args;

static struct
{
	struct arg_dbl * kp;
	struct arg_dbl * ki;
	struct arg_dbl * kd;
	struct arg_end *end;
} set_position_pid_args;

static struct
{
	struct arg_dbl * kp;
	struct arg_dbl * ki;
	struct arg_dbl * kd;
	struct arg_end *end;
} set_speed_pid_args;

static struct
{
	struct arg_dbl * kp;
	struct arg_dbl * ki;
	struct arg_dbl * kd;
	struct arg_end *end;
} set_torque_pid_args;


static struct
{
	struct arg_dbl *quotient;
	struct arg_end *end;
} set_quotient_args;

static struct
{
	struct arg_dbl * hold_torque;
	struct arg_dbl * max_torque;
	struct arg_dbl * max_speed;
	struct arg_end * end;
} set_torque_and_speed_args;


//---------------- Command callbacks here! -------------
static int setMotorParamCommand(int argc, char** argv){
	int nerrors = arg_parse(argc, argv, (void**) &set_motor_params_args);
	if (nerrors != 0) {
		arg_print_errors(stderr, set_motor_params_args.end, argv[0]);
		return 1;
	}
	//flash_config_init();	//This is done once in main before start_cli_passive_mode()
	float km, L, R, i;
	int Nr;
	flash_config_get_motor_params(&km, &L, &R, &i, &Nr);
	if (set_motor_params_args.km->count == 1)
	{
		km = set_motor_params_args.km->dval[0];
	}
	if (set_motor_params_args.L->count == 1)
	{
		L = set_motor_params_args.L->dval[0];
	}
	if (set_motor_params_args.R->count == 1)
	{
		R = set_motor_params_args.R->dval[0];
	}
	if (set_motor_params_args.i->count == 1)
	{
		i = set_motor_params_args.i->dval[0];
	}
	if (set_motor_params_args.Nr->count == 1)
	{
		Nr = set_motor_params_args.Nr->ival[0];
	}
	ESP_LOGI(TAG, "km = %.2f \tL = %.6f \tR = %.2f \ti = %.2f \tN = %d\n", km, L, R, i, Nr);
	flash_config_set_motor_params(km, L, R, i, Nr);
	return 0;
}

static int setPositionPidCommand(int argc, char** argv){
	int nerrors = arg_parse(argc, argv, (void**) &set_position_pid_args);
		if (nerrors != 0) {
		arg_print_errors(stderr, set_position_pid_args.end, argv[0]);
		return 1;
	}
	//flash_config_init();	//This is done once in main before start_cli_passive_mode()
	float kp, ki, kd;
	flash_config_get_position_pid(&kp, &ki, &kd);
	if (set_position_pid_args.kp->count == 1)
	{
		kp = set_position_pid_args.kp->dval[0];
	}
	if (set_position_pid_args.ki->count == 1)
	{
		ki = set_position_pid_args.ki->dval[0];
	}
	if (set_position_pid_args.kd->count == 1)
	{
		kd = set_position_pid_args.kd->dval[0];
	}
	printf("kp = %.4f \t ki = %.4f \t kd = %.4f\n", kp, ki, kd);
	flash_config_set_position_pid(kp, ki, kd);
	
	return 0;
}

static int setSpeedPidCommand(int argc, char** argv){
	int nerrors = arg_parse(argc, argv, (void**) &set_speed_pid_args);
		if (nerrors != 0) {
		arg_print_errors(stderr, set_speed_pid_args.end, argv[0]);
		return 1;
	}
	//flash_config_init();	//This is done once in main before start_cli_passive_mode()
	float kp, ki, kd;
	flash_config_get_speed_pid(&kp, &ki, &kd);
	if (set_speed_pid_args.kp->count == 1)
	{
		kp = set_speed_pid_args.kp->dval[0];
	}
	if (set_speed_pid_args.ki->count == 1)
	{
		ki = set_speed_pid_args.ki->dval[0];
	}
	if (set_speed_pid_args.kd->count == 1)
	{
		kd = set_speed_pid_args.kd->dval[0];
	}
	printf("kp = %.4f \t ki = %.4f \t kd = %.4f\n", kp, ki, kd);
	flash_config_set_speed_pid(kp, ki, kd);
	
	return 0;
}

static int setTorquePidCommand(int argc, char** argv){
	int nerrors = arg_parse(argc, argv, (void**) &set_torque_pid_args);
		if (nerrors != 0) {
		arg_print_errors(stderr, set_torque_pid_args.end, argv[0]);
		return 1;
	}
	//flash_config_init();	//This is done once in main before start_cli_passive_mode()
	float kp, ki, kd;
	flash_config_get_torque_pid(&kp, &ki, &kd);
	if (set_torque_pid_args.kp->count == 1)
	{
		kp = set_torque_pid_args.kp->dval[0];
	}
	if (set_torque_pid_args.ki->count == 1)
	{
		ki = set_torque_pid_args.ki->dval[0];
	}
	if (set_torque_pid_args.kd->count == 1)
	{
		kd = set_torque_pid_args.kd->dval[0];
	}
	printf("kp = %.4f \t ki = %.4f \t kd = %.4f\n", kp, ki, kd);
	flash_config_set_torque_pid(kp, ki, kd);
	
	return 0;
}


static int setQuotientCommand(int argc, char** argv){
	int nerrors = arg_parse(argc, argv, (void**) &set_quotient_args);
	if (nerrors != 0) {
		arg_print_errors(stderr, set_quotient_args.end, argv[0]);
		return 1;
	}
	//flash_config_init();	//This is done once in main before start_cli_passive_mode()
	float q;
	flash_config_get_voltage_divider_quotient(&q);
	if (set_quotient_args.quotient->count == 1)
	{
        //Save the new value
        q = set_quotient_args.quotient->dval[0];
	}
	flash_config_set_voltage_divider_quotient(q);
	printf("Quotient = %.6f\n", q);
	return 0;
}

static int setTorqueAndSpeedCommand(int argc, char** argv){
	int nerrors = arg_parse(argc, argv, (void**) &set_torque_and_speed_args);
	if (nerrors != 0) {
		arg_print_errors(stderr, set_torque_and_speed_args.end, argv[0]);
		return 1;
	}
	//flash_config_init();	//This is done once in main before start_cli_passive_mode()
	float hold_torque, max_torque, max_speed;
	flash_config_get_speed_and_torque(&hold_torque, &max_torque, &max_speed);
	if (set_torque_and_speed_args.hold_torque->count == 1)
	{
		hold_torque = set_torque_and_speed_args.hold_torque->dval[0];
	}
	if (set_torque_and_speed_args.max_torque->count == 1)
	{
		max_torque = set_torque_and_speed_args.max_torque->dval[0];
	}
	if (set_torque_and_speed_args.max_speed->count == 1)
	{
		max_speed = set_torque_and_speed_args.max_speed->dval[0];
	}
	flash_config_set_speed_and_torque(hold_torque, max_torque, max_speed);
	printf("Hold torque = %.2f\t Max torque = %.2f\t Max speed = %.2f\n", hold_torque, max_torque, max_speed);
	return 0;
}
//---------------- Register commands here! -------------
void register_flash_config(){
	//-------------Set motor parameters command
	set_motor_params_args.km 	= arg_dbl0("k","km", "<Nm/A>", "Motor tourque constant");
	set_motor_params_args.L		= arg_dbl0("L", NULL, "<H>", "Motor phase inductance");
	set_motor_params_args.R		= arg_dbl0("R", NULL, "<Ohm>", "Motor phase series resistance");
	set_motor_params_args.i 	= arg_dbl0("i", NULL, "<A>", "Motor maximum winding current");
	set_motor_params_args.Nr	= arg_int0("n", "Nr", "<n>", "Number of rotor teeth / number of poles");
	set_motor_params_args.end	= arg_end(2);

	const esp_console_cmd_t set_motor_params_cmd = {
		.command = "flash_config_motor_params",
		.help = "Set the motor parameters",
		.hint = NULL,
		.func = &setMotorParamCommand,
		.argtable = &set_motor_params_args
	};
	
	ESP_ERROR_CHECK(esp_console_cmd_register(&set_motor_params_cmd));
	//-------------Set position PID command
	set_position_pid_args.kp	= arg_dbl0("p", "kp", NULL, "Set proportional value");
	set_position_pid_args.ki	= arg_dbl0("i", "ki", NULL, "Set integral value");
	set_position_pid_args.kd	= arg_dbl0("d", "kd", NULL, "Set differental value");
	set_position_pid_args.end	= arg_end(2);

	const esp_console_cmd_t set_position_pid_cmd = {
		.command = "flash_config_position_pid",
		.help = "Set the motor controller position PID",
		.hint = NULL,
		.func = &setPositionPidCommand,
		.argtable = &set_position_pid_args
	};

	ESP_ERROR_CHECK(esp_console_cmd_register(&set_position_pid_cmd));

	//-------------Set speed PID command
	set_speed_pid_args.kp	= arg_dbl0("p", "kp", NULL, "Set proportional value");
	set_speed_pid_args.ki	= arg_dbl0("i", "ki", NULL, "Set integral value");
	set_speed_pid_args.kd	= arg_dbl0("d", "kd", NULL, "Set differental value");
	set_speed_pid_args.end	= arg_end(2);

	const esp_console_cmd_t set_speed_pid_cmd = {
		.command = "flash_config_speed_pid",
		.help = "Set the motor controller speed PID",
		.hint = NULL,
		.func = &setSpeedPidCommand,
		.argtable = &set_speed_pid_args
	};

	ESP_ERROR_CHECK(esp_console_cmd_register(&set_speed_pid_cmd));

	//-------------Set torque PID command
	set_torque_pid_args.kp	= arg_dbl0("p", "kp", NULL, "Set proportional value");
	set_torque_pid_args.ki	= arg_dbl0("i", "ki", NULL, "Set integral value");
	set_torque_pid_args.kd	= arg_dbl0("d", "kd", NULL, "Set differental value");
	set_torque_pid_args.end	= arg_end(2);

	const esp_console_cmd_t set_torque_pid_cmd = {
		.command = "flash_config_torque_pid",
		.help = "Set the motor controller torque PID",
		.hint = NULL,
		.func = &setTorquePidCommand,
		.argtable = &set_torque_pid_args
	};

	ESP_ERROR_CHECK(esp_console_cmd_register(&set_torque_pid_cmd));

    //-------------Set quotient command
	set_quotient_args.quotient = arg_dbl0(NULL, NULL, "<q>", "Quotient for input voltage divider");
	set_quotient_args.end = arg_end(2);

	const esp_console_cmd_t set_quotient_cmd = {
		.command = "flash_config_voltage_divider_quotient",
		.help = "Set the quotient for the input voltage divider",
		.hint = NULL,
		.func = &setQuotientCommand,
		.argtable = &set_quotient_args
	};

	ESP_ERROR_CHECK(esp_console_cmd_register(&set_quotient_cmd));

	//-------------Set torque and speed command
	set_torque_and_speed_args.hold_torque = arg_dbl0("t","hold_torque","<Nm>", "Holding torque");
	set_torque_and_speed_args.max_torque = arg_dbl0("m","max_torque","<Nm>", "Maximum torque");
	set_torque_and_speed_args.max_speed = arg_dbl0("s","max_speed","<rad/s>", "Maximum speed");
	set_torque_and_speed_args.end = arg_end(2);

	const esp_console_cmd_t set_torque_and_speed_cmd = {
		.command = "flash_config_torque_and_speed",
		.help = "Set values for allowed speed and torque",
		.hint = NULL,
		.func = &setTorqueAndSpeedCommand,
		.argtable = &set_torque_and_speed_args
	};

	ESP_ERROR_CHECK(esp_console_cmd_register(&set_torque_and_speed_cmd));
}