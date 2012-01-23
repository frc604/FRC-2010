/**
 * This is a program to conrol Quixiver Team 604 2010 robot in C++
 * YOU MUST start with the kickers and pokeys 
 * The controls are as follows:
 * 
 * right Joystick:
 * up- forward right drive
 * down- backwards right drive
 * button1(trigger)- shift to high gear
 * button3- pokey out
 * button2- pokey in
 * 
 * left joystick:
 * up- forward left drive
 * down- backwards leftve
 * button1(trigger)- shift to high gear
 * button2- pokey in
 * button3- pokey out
 * 
 * Manipulator- 
 * button1(trigger)- one kick
 * button2- Ballmagnet in
 * button3- ballmagnet out
 * +/- controls ball magnet speed
 * 
 */

#include "WPILib.h"

/* Port configuration for sensors and actuators. */
	#define LEFT_DRIVE_JOYSTICK_USB_PORT 3
	#define MANIPULATOR_JOYSTICK_USB_PORT 1
	#define RIGHT_DRIVE_JOYSTICK_USB_PORT 2

	#define FRONT_LEFT_MOTOR_PORT 5
	#define FRONT_RIGHT_MOTOR_PORT 8
	#define REAR_LEFT_MOTOR_PORT 7
	#define REAR_RIGHT_MOTOR_PORT 6
	
	#define BALL_MAGNET_MOTOR_1_PORT 1
	#define BALL_MAGNET_MOTOR_2_PORT 2
	
	#define READY_SENSOR_PORT 2
	
	#define COMPRESSOR_PORT 1
	#define PRESSURE_SWITCH_PORT 1
	#define LATCH_SOLENOID_PORT 1
	#define SHIFTER_SOLENOID_PORT 4
	#define RELEASE_SOLENOID_PORT 3
	#define POKEY_SOLENOID_FORWARD_PORT 6
	#define POKEY_SOLENOID_REVERSE_PORT 5
	#define RELOAD_SOLENOID_FORWARD_PORT 2
	#define RELOAD_SOLENOID_REVERSE_PORT 1

/* Button configuration. */
	/* Manipulator Button Configuration */
		#define MANIPULATOR_KICK_BUTTON 1
		#define MANIPULATOR_BALL_MAGNET_IN_BUTTON 2
		#define MANIPULATOR_BALL_MAGNET_OUT_BUTTON 3
	
	/* Driver Button Configuration */
		#define DRIVER_SHIFT_BUTTON 1
		#define DRIVER_POKEY_BUTTON 3

/* Actuator polarity and speed configuration. */
	#define SOLENOID_SHIFTER_HIGH_POWER_DIRECTION true
	#define SOLENOID_SHIFTER_LOW_POWER_DIRECTION false
	
	#define SOLENOID_POKEY_IN_DIRECTION DoubleSolenoid::kForward
	#define SOLENOID_POKEY_OUT_DIRECTION DoubleSolenoid::kReverse
	
	#define SOLENOID_RELOAD_PUSH_DIRECTION DoubleSolenoid::kForward
	#define SOLENOID_RELOAD_PULL_DIRECTION DoubleSolenoid::kReverse

	#define SOLENOID_RELEASE_UP_DIRECTION true
	#define SOLENOID_RELEASE_DOWN_DIRECTION false

class Robot2010Overkill : public SimpleRobot {
	Joystick joystickManipulator;
	Joystick joystickDriveLeft;
	Joystick joystickDriveRight;
	
	RobotDrive driveTrain;
	
	Victor motorBallMagnet1;
	Victor motorBallMagnet2;
	
	DigitalInput digitalReadySensor;
	
	Compressor *compressorPump;
	
	Solenoid solenoidLatch;
	Solenoid solenoidShifter;
	Solenoid solenoidRelease;
	DoubleSolenoid solenoidPokey;
	DoubleSolenoid solenoidReload;
	
	public:
		Robot2010Overkill(void):
			joystickManipulator(MANIPULATOR_JOYSTICK_USB_PORT),
			joystickDriveLeft(LEFT_DRIVE_JOYSTICK_USB_PORT),
			joystickDriveRight(RIGHT_DRIVE_JOYSTICK_USB_PORT),
			driveTrain(new Victor(FRONT_LEFT_MOTOR_PORT), new Victor(REAR_LEFT_MOTOR_PORT), new Victor(FRONT_RIGHT_MOTOR_PORT), new Victor(REAR_RIGHT_MOTOR_PORT)),
			motorBallMagnet1(BALL_MAGNET_MOTOR_1_PORT),
			motorBallMagnet2(BALL_MAGNET_MOTOR_2_PORT),
			digitalReadySensor(READY_SENSOR_PORT),
			solenoidLatch(LATCH_SOLENOID_PORT),
			solenoidShifter(SHIFTER_SOLENOID_PORT),
			solenoidRelease(RELEASE_SOLENOID_PORT),
			solenoidPokey(POKEY_SOLENOID_FORWARD_PORT, POKEY_SOLENOID_REVERSE_PORT),
			solenoidReload(RELOAD_SOLENOID_FORWARD_PORT, RELOAD_SOLENOID_REVERSE_PORT)
		{
			GetWatchdog().SetEnabled(false); // If you're just beginning, and nothing's going on, there's no need for Watchdog to be doing anything.
			driveTrain.SetSafetyEnabled(false);
			
			compressorPump = new Compressor(PRESSURE_SWITCH_PORT, COMPRESSOR_PORT);
		}
		
		void Autonomous(void) {
			GetWatchdog().SetEnabled(false); // No need for Watchdog in Autonomous, either.
			driveTrain.SetSafetyEnabled(false);
			
			compressorPump->Start(); // Let's start up the compressor and charge up for Teleop.
			
			while(IsAutonomous() && IsEnabled()) {
				// TODO: Latch up the stuff, prepare for firing.
				// If there's nothing left to do, keep the compressor running.
			}
			
			compressorPump->Stop(); // Okay, fun's over
		}
		
		void OperatorControl(void) {
			GetWatchdog().SetEnabled(true); // We do want Watchdog in Teleop, though.
			driveTrain.SetSafetyEnabled(true);
			
			compressorPump->Start(); // Let's start up the compressor too, while we're at it.
			
			/* Declare and initialize variables. */
				Timer *kickTimer = new Timer();
				
				int kickState = 0;
				int readyComp = 0;
				
				float floatThrottle = 0.0;
				
			/* Debug Functionality */
				DriverStationLCD *dsLCD = DriverStationLCD::GetInstance();
				
				dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "                     ");
				dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "                     ");
				dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "                     ");
				dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "                     ");
				
				dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Kicker In");
				dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "Pokey In");
				dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "High Gear");
				dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "Sucking Power:");
				dsLCD->UpdateLCD();
			
			while (IsOperatorControl() && IsEnabled()) {
				/* Drive Control */
					/* Shifting */
						if(joystickDriveLeft.GetRawButton(DRIVER_SHIFT_BUTTON) || joystickDriveRight.GetRawButton(DRIVER_SHIFT_BUTTON)) {
							solenoidShifter.Set(SOLENOID_SHIFTER_HIGH_POWER_DIRECTION);
							
							dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "                ");
							dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "High Gear");
							dsLCD->UpdateLCD();
						} else {
							solenoidShifter.Set(SOLENOID_SHIFTER_LOW_POWER_DIRECTION);
							
							dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "                ");
							dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "Low Gear");
							dsLCD->UpdateLCD();
						}
					
					/* Drive Train */
						driveTrain.TankDrive(joystickDriveLeft, joystickDriveRight);
					
					/* Pokey Control */
						if(joystickDriveLeft.GetRawButton(DRIVER_POKEY_BUTTON) || joystickDriveRight.GetRawButton(DRIVER_POKEY_BUTTON)) {
							solenoidPokey.Set(SOLENOID_POKEY_OUT_DIRECTION);
							
							dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "                ");
							dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "Pokey Out");
							dsLCD->UpdateLCD();
						} else {
							solenoidPokey.Set(SOLENOID_POKEY_IN_DIRECTION);
							
							dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "                ");
							dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "Pokey In");
							dsLCD->UpdateLCD();
						}
				
				/* Manipulator Control */
						/* Kicker Control */
							switch(kickState) {
								case 0:
									if(joystickManipulator.GetRawButton(MANIPULATOR_KICK_BUTTON)) {
										kickState = 1;
										
										kickTimer->Reset();
										kickTimer->Start();
										
										solenoidRelease.Set(SOLENOID_RELEASE_UP_DIRECTION);
									}
									
									break;
								case 1:
									if(kickTimer->Get() >= 1) {
										kickState = 1;
										
										kickTimer->Reset();
										kickTimer->Start();
										
										solenoidReload.Set(SOLENOID_RELOAD_PULL_DIRECTION);
									}
									
									break;
								case 2:
									if(digitalReadySensor.Get() == 1 || kickTimer->Get() >= 1.5) {
										kickState = 0;
										
										kickTimer->Stop();

										solenoidRelease.Set(SOLENOID_RELEASE_DOWN_DIRECTION);
										solenoidReload.Set(SOLENOID_RELOAD_PUSH_DIRECTION);
									}
									
									break;
							}
						
						/* Ball Magnet Control */
							floatThrottle = joystickManipulator.GetThrottle();
							
							dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "                 ");
							dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "Magnet Power: %f", floatThrottle);
							
							dsLCD->UpdateLCD();
							
							if(joystickManipulator.GetRawButton(MANIPULATOR_BALL_MAGNET_IN_BUTTON)) {
								motorBallMagnet1.Set(floatThrottle);
								motorBallMagnet2.Set(floatThrottle);
							} else if(joystickManipulator.GetRawButton(MANIPULATOR_BALL_MAGNET_OUT_BUTTON)) {
								motorBallMagnet1.Set(floatThrottle * -1);
								motorBallMagnet2.Set(floatThrottle * -1);
							} else {
								motorBallMagnet1.Set(0.0);
								motorBallMagnet2.Set(0.0);
							}
			}
		}
};

START_ROBOT_CLASS(Robot2010Overkill);
