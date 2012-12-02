#pragma config(Hubs,  S1, HTServo,  HTMotor,  HTMotor,  none)
#pragma config(Sensor, S2,     infrared,       sensorI2CCustom)
#pragma config(Sensor, S3,     color,          sensorCOLORFULL)
#pragma config(Sensor, S4,     ultrasonic,     sensorSONAR)
#pragma config(Motor,  motorA,          motor_popcorn, tmotorNXT, openLoop)
#pragma config(Motor,  motorB,          motor_B,       tmotorNXT, openLoop)
#pragma config(Motor,  motorC,          motor_C,       tmotorNXT, openLoop)
#pragma config(Motor,  mtr_S1_C2_1,     motor_L,       tmotorTetrix, PIDControl, encoder)
#pragma config(Motor,  mtr_S1_C2_2,     motor_R,       tmotorTetrix, PIDControl, reversed, encoder)
#pragma config(Motor,  mtr_S1_C3_1,     motor_lift,    tmotorTetrix, PIDControl, encoder)
#pragma config(Motor,  mtr_S1_C3_2,     motor_G,       tmotorTetrix, openLoop)
#pragma config(Servo,  srvo_S1_C1_1,    servo_IR,             tServoStandard)
#pragma config(Servo,  srvo_S1_C1_2,    servo_claw,           tServoStandard)
#pragma config(Servo,  srvo_S1_C1_3,    servo_ramp,           tServoStandard)
#pragma config(Servo,  srvo_S1_C1_4,    servo_D,              tServoNone)
#pragma config(Servo,  srvo_S1_C1_5,    servo_E,              tServoNone)
#pragma config(Servo,  srvo_S1_C1_6,    servo_F,              tServoNone)
// Code generated by the 'ROBOTC' configuration wizard.
#include "JoystickDriver.c"
#include "hitechnic-irseeker-v2.h"
#include "enums.h"
#include "typedefs.h"
#include "global vars.h"
#include "structs.h"
#include "low-level functions.h"
#include "high-level functions.h"
#include "subroutines.h"



void initializeRobot()
{
	// Sensors are config'ed and setup by RobotC (need to stabalize).

	Servo_SetSpeed(servo_IR, 10);		// maximum speed!
	Servo_SetSpeed(servo_claw, 10);		// maximum speed!
	Servo_SetSpeed(servo_ramp, 100);	// slowly update so ramp doesn't release.

	Servo_Rotate(servo_IR, g_IRServoExtended);		// will fold back up in tele-op
	Servo_Rotate(servo_claw, g_clawServoExtended);	// will be folded in tele-op
	Servo_Rotate(servo_ramp, g_rampServoDefault);	// stops ramp from deploying

	Motor_SetMaxSpeed(g_FullRegulatedPower);

	Motor_ResetEncoder(motor_L);
	Motor_ResetEncoder(motor_R);
	Motor_ResetEncoder(motor_lift);

	// Wait this long so the claw & IR servos get to update.
	// The ramp-release servo shouldn't move; the long update time
	// is to prevent sudden jerks that might release the ramp.
	// We don't need to wait for the IR sensor to stabalize since
	// the robot doesn't read from it until it's at the first column,
	// which should be ample time for RobotC to setup the sensor.
	Time_Wait(10);

	return;
}



task main()
{
	waitForStart();

	initializeRobot();



	// The amount of time the robot...

	// ...drives forward to get in position to lap.
	const int forwardTimeA	= 50;
	// ...turns 90 deg to be parallel to the wall.
	const int turnTimeA		= 110;
	// ...drives forward to be in the right spot to pass every time.
	const int forwardTimeB	= 50;
	// ...needs to complete a single lap (to the same spot as above).
	const int lapTime		= 1000;



	// Power of the motor on outer side of the turn.
	const int masterPower	= 100;
	// Power of the motor on inner side of the turn.
	const int slavePower	= masterPower*0.5;



	Move_Forward	(forwardTimeA, g_AccurateMotorPower);
	Turn_Left		(turnTimeA, g_AccurateMotorPower, g_AccurateMotorPower);
	Move_Forward	(forwardTimeB, g_AccurateMotorPower);

	while (true)
	{
		Turn_Right		(lapTime, masterPower, slavePower);
		//PlaySoundFile	("killed.rso");
		PlaySoundFile	("moo.rso");
	}
}
