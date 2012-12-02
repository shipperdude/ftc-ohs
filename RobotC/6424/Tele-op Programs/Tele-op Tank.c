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

	Servo_Rotate(servo_IR, g_IRServoLowered);		// it gets in the way
	Servo_Rotate(servo_claw, g_clawServoFolded);	// servo bracket gets bent
	Servo_Rotate(servo_ramp, g_rampServoDefault);	// stops ramp from deploying

	Motor_SetMaxSpeed(g_FullRegulatedPower);

	Motor_ResetEncoder(motor_L);
	Motor_ResetEncoder(motor_R);
	Motor_ResetEncoder(motor_lift);

	// We might not even use this--the operator should have control.
	// Uncomment the next lines if the robot burns and crashes.

	//// Wait this long so the claw & IR servos get to update.
	//// The ramp-release servo shouldn't move; the long update time
	//// is to prevent sudden jerks that might release the ramp.
	//Time_Wait(10);

	return;
}



task main()
{
	// These will be used later and are declared here to save from having to
	// declare them every single loop.
	int powerL = 0;
	int powerR = 0;
	int powerPopcorn = 0;
	//// Not implemented yet. We'll implement when optimizing for speed.
	//MotorState isMotorStateL = MOTOR_JOYSTICK;
	//MotorState isMotorStateR = MOTOR_JOYSTICK;



	waitForStart();

	initializeRobot();



	while (true)
	{
		// Currently does (at least) 7 checks and 3 assignments per loop.
		Joystick_UpdateData();



		// These should be zeroed after every loop. In the case that there
		// isn't input, the motors won't keep moving at the last speed it had.
		powerL = 0;
		powerR = 0;
		powerLift = 0;
		powerPopcorn = 0;

		// POPCORN!!! (This comes first, obviously.)
		if ( Joystick_Button(BUTTON_B, CONTROLLER_2)==true )
		{
			powerPopcorn = g_FullDrivePower;
		}
		else if ( Joystick_Button(BUTTON_A, CONTROLLER_2)==true )
		{
			powerPopcorn = (-1)*g_FullDrivePower;
		}



		// See if a direction is being pressed, then test for the direction.
		// This is inside an `if` statement to optimize speed (less checking).
		// `JoystickController` arguments are not passed to increase speed.

		// Input from CONTROLLER_2 will be used to control the lift in
		// conjunction with CONTROLLER_1, but shouldn't override the driver,
		// since driver #1's input is processed last.

		// This is the code for CONTROLLER_2:
		if ( abs(joystick.joy2_y1)>g_JoystickThreshold )
		{
			isLiftState = LIFT_JOYSTICK;
			//powerLift = Math_ToLogarithmic(joystick.joy2_y1);
			powerLift = Math_ToLogarithmic(Joystick_Joystick(JOYSTICK_L, AXIS_Y, CONTROLLER_2));
		}
		if ( (	Joystick_Button(BUTTON_LB, CONTROLLER_2) ||
				Joystick_Button(BUTTON_RB, CONTROLLER_2)) ==true )
		{
			isLiftState = LIFT_JOYSTICK;
			powerLift /= g_FineTuneFactor;
		}

		// This is the code for CONTROLLER_1, along with two unimplemented
		// functions for putting rings on and taking rings off.
		if ( Joystick_Direction() != DIRECTION_NONE )
		{
			switch ( Joystick_Direction() )
			{

				// Operate lift at full power if F/B.
				case DIRECTION_F:
					isLiftState = LIFT_JOYSTICK;
					powerLift = g_FullLiftPower;
					break;
				case DIRECTION_B:
					isLiftState = LIFT_JOYSTICK;
					powerLift = (-1)*g_FullLiftPower;
					break;

				case DIRECTION_L:
					sub_PutRingOn();
					break;
				case DIRECTION_R:
					sub_TakeRingOff();
					break;
			}
		}



		// See if a button (not masked) is being pressed, then react.
		// This is inside an `if` statement to optimize speed (less checking).

		// The argument to this first `if` statement is a masked version
		// of the "bitmap" of buttons directly from the controller.

		// Everything other than the buttons used are masked off, to increase
		// processing speed (possibly, just speculation). Reasoning:
		// `&` compares all bits of the variables, so we might as well mask
		// everything we won't need, in case something irrelevant is pressed.

		// A `0` value means no buttons (that we are testing for) are pressed.
		// Directly using the struct since this is the only possible time to
		// use it, and this is very low-level anyways.

		//if ( joystick.joy1_Buttons != false )
		//if ( (g_ControllerMaskA & joystick.joy1_Buttons) != false )
		{

			// Buttons Y/B/A will control lift height.
			if ( Joystick_Button(BUTTON_Y)==true )
			{
				isLiftState = LIFT_TOP;
			}
			if ( Joystick_Button(BUTTON_B)==true )
			{
				isLiftState = LIFT_MIDDLE;
			}
			if ( Joystick_Button(BUTTON_A)==true )
			{
				isLiftState = LIFT_BOTTOM;
			}

			// If only X is pressed, weigh the ring.
			// If JOYR is pressed as well, deploy ramp.
			if ( Joystick_Button(BUTTON_X)==true )
			{
				if ( Joystick_Button(BUTTON_JOYR) == true )
				{
					Servo_Rotate(servo_ramp, g_rampServoDeployed);
				}
				else
				{
					isLiftState = LIFT_FETCH;
				}
			}

			// Buttons LT/RT will fine-tune the lift.
			if ( Joystick_Button(BUTTON_RT)==true )
			{
				isLiftState = LIFT_JOYSTICK;
				powerLift = g_FullLiftPower/g_FineTuneFactor;
			}
			if ( Joystick_Button(BUTTON_LT)==true )
			{
				isLiftState = LIFT_JOYSTICK;
				powerLift = (-1)*g_FullLiftPower/g_FineTuneFactor;
			}

		}



		// L/R motor code. Only triggered when a joystick returns a
		// value greater than the "drive" threshold (`global vars.h`).

		// Logarithmic control probably won't be implemented anytime soon.
		// Also need to stop using the `joystick` struct and switch to the
		// encapsulated version (Joystick_Joystick(...)).



		// Y-axis code:
		if ( 	abs(Joystick_Joystick(JOYSTICK_L, AXIS_Y)) > g_JoystickThreshold ||
				abs(Joystick_Joystick(JOYSTICK_R, AXIS_Y)) > g_JoystickThreshold )
		{
			powerL = Math_ToLogarithmic(Joystick_Joystick(JOYSTICK_L, AXIS_Y));
			powerR = Math_ToLogarithmic(Joystick_Joystick(JOYSTICK_R, AXIS_Y));
		}

		// Last check: if LB/RB is pressed, fine-tune the power level.
		if ( (Joystick_Button(BUTTON_LB)||Joystick_Button(BUTTON_RB)) ==true )
		{
			powerL /= g_FineTuneFactor;
			powerR /= g_FineTuneFactor;
		}



		// CONTROLLER_2 has the same masking implementation as CONTROLLER_1.
		// For a detailed explanation of the mechanism, see those comments.

		// CONTROLLER_2 is only tested for button X (currently).

		//if ( joystick.joy2_Buttons != false )
		//if ( (g_ControllerMaskB & joystick.joy2_Buttons) != false )
		{

			// If X is pressed, the MOO shall be released!
			if ( Joystick_Button(BUTTON_X, CONTROLLER_2)==true )
			{
				//StartTask(sub_MOO);
				PlaySoundFile("moo.rso");
			}
			if ( Joystick_Button(BUTTON_Y, CONTROLLER_2)==true )
			{
				//StopTask(sub_MOO);
				sub_CowsWithGuns();
			}

		}



		switch (isLiftState)
		{
			case LIFT_BOTTOM:
				sub_LiftToHeight(g_BottomLiftAngle);
				break;
			case LIFT_MIDDLE:
				sub_LiftToHeight(g_MiddleLiftAngle);
				break;
			case LIFT_TOP:
				sub_LiftToHeight(g_TopLiftAngle);
				break;
			case LIFT_FETCH:
				sub_LiftToHeight(g_FetchLiftAngle);
		}



		// Flush the controller input buffer periodically (every 1/4 sec?)



		Motor_SetPower(motor_L, powerL);
		Motor_SetPower(motor_R, powerR);
		Motor_SetPower(motor_lift, powerLift);
		Motor_SetPower(motor_popcorn, powerPopcorn);



	}
}
