#include "WPILib.h"

/* Port configuration for sensors and actuators. */
	#define LEFT_DRIVE_JOYSTICK_USB_PORT 3
	#define MANIPULATOR_JOYSTICK_USB_PORT 1
	#define RIGHT_DRIVE_JOYSTICK_USB_PORT 2

	#define FRONT_LEFT_MOTOR_PORT 5
	#define FRONT_RIGHT_MOTOR_PORT 7
	#define REAR_LEFT_MOTOR_PORT 8
	#define REAR_RIGHT_MOTOR_PORT 6
	
	#define BALL_MAGNET_MOTOR_1_PORT 1
	#define BALL_MAGNET_MOTOR_2_PORT 2
	
	#define READY_SENSOR_PORT 2
	
	#define COMPRESSOR_PORT 1
	#define PRESSURE_SWITCH_PORT 1
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

/* Xbox button configuration. */
	#define XBOX_KICK_BUTTON 1
	#define XBOX_BALL_MAGNET_IN_BUTTON 6
	#define XBOX_BALL_MAGNET_OUT_BUTTON 5
	
	#define XBOX_POKEY_BUTTON 2

/* Actuator polarity and speed configuration. */
	#define SOLENOID_SHIFTER_HIGH_POWER_DIRECTION true
	#define SOLENOID_SHIFTER_LOW_POWER_DIRECTION false
	
	#define SOLENOID_POKEY_IN_DIRECTION DoubleSolenoid::kForward
	#define SOLENOID_POKEY_OUT_DIRECTION DoubleSolenoid::kReverse
	
	#define SOLENOID_RELOAD_PUSH_DIRECTION DoubleSolenoid::kReverse
	#define SOLENOID_RELOAD_PULL_DIRECTION DoubleSolenoid::kForward

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
	
	Solenoid solenoidShifter;
	Solenoid solenoidRelease;
	DoubleSolenoid solenoidPokey;
	DoubleSolenoid solenoidReload;
	
	float virtualThrottle;
	bool trackingThrottle;
	Timer *throttleTimer;
	
	public:
		Robot2010Overkill(void):
			joystickManipulator(MANIPULATOR_JOYSTICK_USB_PORT),
			joystickDriveLeft(LEFT_DRIVE_JOYSTICK_USB_PORT),
			joystickDriveRight(RIGHT_DRIVE_JOYSTICK_USB_PORT),
			driveTrain(new Victor(FRONT_LEFT_MOTOR_PORT), new Victor(REAR_LEFT_MOTOR_PORT), new Victor(FRONT_RIGHT_MOTOR_PORT), new Victor(REAR_RIGHT_MOTOR_PORT)),
			motorBallMagnet1(BALL_MAGNET_MOTOR_1_PORT),
			motorBallMagnet2(BALL_MAGNET_MOTOR_2_PORT),
			digitalReadySensor(READY_SENSOR_PORT),
			solenoidShifter(SHIFTER_SOLENOID_PORT),
			solenoidRelease(RELEASE_SOLENOID_PORT),
			solenoidPokey(POKEY_SOLENOID_FORWARD_PORT, POKEY_SOLENOID_REVERSE_PORT),
			solenoidReload(RELOAD_SOLENOID_FORWARD_PORT, RELOAD_SOLENOID_REVERSE_PORT)
		{
			GetWatchdog().SetEnabled(false); // If you're just beginning, and nothing's going on, there's no need for Watchdog to be doing anything.
			driveTrain.SetSafetyEnabled(false);
			
			compressorPump = new Compressor(PRESSURE_SWITCH_PORT, COMPRESSOR_PORT);
			throttleTimer = new Timer();
			
			virtualThrottle = 0;
			trackingThrottle = false;
		}
		
		typedef enum
		{
			kKick = 0,
			kSuck = 1,
			kSpit = 2,
			kShift = 3,
			kPokey = 4
		} ActionType;
		
		typedef enum
		{
			kLeftX = 0,
			kLeftY = 1,
			kRightX = 2,
			kRightY = 3,
			kThrottle = 4
		} AxisType;
		
		bool GetAction(ActionType action) {
			bool isXbox = joystickDriveLeft.GetThrottle() == 0;
			
			switch(action) {
				case kKick:
					if(isXbox)
						return joystickManipulator.GetRawButton(XBOX_KICK_BUTTON);
					else
						return joystickManipulator.GetRawButton(MANIPULATOR_KICK_BUTTON);
					
					break;
				case kSuck:
					if(isXbox)
						return joystickManipulator.GetRawButton(XBOX_BALL_MAGNET_IN_BUTTON);
					else
						return joystickManipulator.GetRawButton(MANIPULATOR_BALL_MAGNET_IN_BUTTON);
					
					break;
				case kSpit:
					if(isXbox)
						return joystickManipulator.GetRawButton(XBOX_BALL_MAGNET_OUT_BUTTON);
					else
						return joystickManipulator.GetRawButton(MANIPULATOR_BALL_MAGNET_OUT_BUTTON);
					
					break;
				case kShift:
					if(isXbox)
						return joystickManipulator.GetRawAxis(3) > 0.2 || joystickManipulator.GetRawAxis(3) < -0.2;
					else
						return joystickDriveLeft.GetRawButton(DRIVER_SHIFT_BUTTON) || joystickDriveRight.GetRawButton(DRIVER_SHIFT_BUTTON);
					
					break;
				case kPokey:
					if(isXbox)
						return joystickManipulator.GetRawButton(XBOX_POKEY_BUTTON);
					else
						return joystickDriveLeft.GetRawButton(DRIVER_POKEY_BUTTON) || joystickDriveRight.GetRawButton(DRIVER_POKEY_BUTTON);
					
					break;
			}
			
			return false;
		}
		
		float GetAxis(AxisType axis) {
			bool isXbox = joystickDriveLeft.GetThrottle() == 0;
			
			switch(axis) {
				case kLeftX:
					if(isXbox)
						return joystickManipulator.GetRawAxis(1);
					else
						return joystickDriveLeft.GetX();
					
					break;
				case kLeftY:
					if(isXbox)
						return joystickManipulator.GetRawAxis(2);
					else
						return joystickDriveLeft.GetY();
					
					break;
				case kRightX:
					if(isXbox)
						return joystickManipulator.GetRawAxis(4);
					else
						return joystickDriveRight.GetX();
					
					break;
				case kRightY:
					if(isXbox)
						return joystickManipulator.GetRawAxis(5);
					else
						return joystickDriveRight.GetY();
					
					break;
				case kThrottle:
					if(isXbox) {
						bool buttonX = joystickManipulator.GetRawButton(3);
						bool buttonY = joystickManipulator.GetRawButton(4);
						
						if(buttonX || buttonY) {
							if(!trackingThrottle) {
								throttleTimer->Reset();
								throttleTimer->Start();
								
								trackingThrottle = true;
							} else {
								virtualThrottle += throttleTimer->Get() / 3 * ((buttonX) ? -1 : 1);
								throttleTimer->Reset();
								
								if(virtualThrottle > 1)
									virtualThrottle = 1;
								if(virtualThrottle < -1)
									virtualThrottle = -1;
							}
						} else {
							if(trackingThrottle) {
								throttleTimer->Stop();
								trackingThrottle = false;
							}
						}
						
						return virtualThrottle;
					} else {
						if(trackingThrottle) {
							throttleTimer->Stop();
							trackingThrottle = false;
						}
						
						return joystickManipulator.GetThrottle();
					}
					
					break;
			}
			
			return 0;
		}
		
		void Autonomous(void) {
			GetWatchdog().SetEnabled(false); // No need for Watchdog in Autonomous, either.
			driveTrain.SetSafetyEnabled(false);
			
			int reloadState = 0;
			Timer *reloadTimer = new Timer();
			
			while(IsAutonomous() && IsEnabled()) {
				compressorPump->Start();
				
				switch(reloadState) {
					case 0:
						if(digitalReadySensor.Get() != 1) reloadState = 1;
						
						break;
					case 1:
						reloadState = 2;
						
						reloadTimer->Start();
						solenoidReload.Set(SOLENOID_RELOAD_PULL_DIRECTION);
						
						break;
					case 2:
						if(reloadTimer->Get() >= 1.5) {
							reloadState = 3;
							
							reloadTimer->Reset();
							
							solenoidRelease.Set(SOLENOID_RELEASE_DOWN_DIRECTION);
						}
						
						break;
					case 3:
						if(reloadTimer->Get() >= 0.75) {
							reloadState = 4;
							
							reloadTimer->Stop();
							
							solenoidReload.Set(SOLENOID_RELOAD_PUSH_DIRECTION);
						}
				}
			}
			
			compressorPump->Stop(); // Okay, fun's over
		}
		
		void OperatorControl(void) {
			GetWatchdog().SetEnabled(true); // We do want Watchdog in Teleop, though.
			driveTrain.SetSafetyEnabled(true);
			
			/* Declare and initialize variables. */
				Timer *kickTimer = new Timer();
				
				int kickState = 0;
				
				float floatThrottle = 0.0;
				
			/* Debug Functionality */
				DriverStationLCD *dsLCD = DriverStationLCD::GetInstance();
				
				dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "                     ");
				dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "                     ");
				dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "                     ");
				dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "                     ");
				dsLCD->Printf(DriverStationLCD::kUser_Line5, 1, "                     ");
				
				dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Kicker In");
				dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "Pokey In");
				dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "High Gear");
				dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "Magnet Power:");
				dsLCD->Printf(DriverStationLCD::kUser_Line5, 1, "Joystick Mode");
				dsLCD->UpdateLCD();
			
			while (IsOperatorControl() && IsEnabled()) {
				GetWatchdog().Feed(); // Feed the watchdog; keep our overlords happy.
				compressorPump->Start();
				
				if(joystickDriveLeft.GetThrottle() == 0) {
					dsLCD->Printf(DriverStationLCD::kUser_Line5, 1, "                     ");
					dsLCD->Printf(DriverStationLCD::kUser_Line5, 1, "Xbox Mode");
					dsLCD->UpdateLCD();
				} else {
					dsLCD->Printf(DriverStationLCD::kUser_Line5, 1, "                     ");
					dsLCD->Printf(DriverStationLCD::kUser_Line5, 1, "Joystick Mode");
					dsLCD->UpdateLCD();
				}
				
				/* Manipulator Control */
					/* Kicker Control */
						switch(kickState) {
							case 0:
								if(GetAction(kKick)) {
									kickState = 1;
									
									kickTimer->Reset();
									kickTimer->Start();
									
									solenoidReload.Set(SOLENOID_RELOAD_PUSH_DIRECTION);
									solenoidRelease.Set(SOLENOID_RELEASE_UP_DIRECTION);
									
									dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "                     ");
									dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Kicking");
									dsLCD->UpdateLCD();
								}
								
								break;
							case 1:
								if(kickTimer->Get() >= 1) {
									kickState = 2;
									
									kickTimer->Reset();
									
									solenoidReload.Set(SOLENOID_RELOAD_PULL_DIRECTION);
									
									dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "                     ");
									dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Reloading");
									dsLCD->UpdateLCD();
								}
								
								break;
							case 2:
								if(kickTimer->Get() >= 1.5) {
									kickState = 3;
									
									kickTimer->Reset();
									
									solenoidRelease.Set(SOLENOID_RELEASE_DOWN_DIRECTION);
									
									dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "                     ");
									dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Latching");
									dsLCD->UpdateLCD();
								}
								
								break;
							case 3:
								if(kickTimer->Get() >= 0.75) {
									kickState = 4;
									
									kickTimer->Reset();
									
									solenoidReload.Set(SOLENOID_RELOAD_PUSH_DIRECTION);
									
									dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "                     ");
									dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Cooldown");
									dsLCD->UpdateLCD();
								}
							case 4:
								if(kickTimer->Get() >= 10) {
									kickState = 0;
									
									kickTimer->Stop();
									
									dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "                     ");
									dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Kicker In");
									dsLCD->UpdateLCD();
								} else {
									dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "                     ");
									dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Cooldown");
									dsLCD->UpdateLCD();
								}
								
								break;
						}
						
						/* Ball Magnet Control */
							floatThrottle = (GetAxis(kThrottle) - 1) / 2;
							
							dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "                 ");
							dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "Magnet Power: %f", floatThrottle * -1);
							
							dsLCD->UpdateLCD();
							
							if(GetAction(kSuck)) {
								motorBallMagnet1.Set(floatThrottle);
								motorBallMagnet2.Set(floatThrottle);
							} else if(GetAction(kSpit)) {
								motorBallMagnet1.Set(floatThrottle * -1);
								motorBallMagnet2.Set(floatThrottle * -1);
							} else {
								motorBallMagnet1.Set(0.0);
								motorBallMagnet2.Set(0.0);
							}
				
				/* Drive Control */
					/* Shifting */
						if(GetAction(kShift)) {
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
						driveTrain.TankDrive(GetAxis(kLeftY), GetAxis(kRightY));
					
					/* Pokey Control */
						if(GetAction(kPokey)) {
							solenoidPokey.Set(SOLENOID_POKEY_OUT_DIRECTION);
							
							if(kickState == 0) {
								kickState = 4;
								
								kickTimer->Reset();
								kickTimer->Start();
							} else if(kickState == 4) {
								kickTimer->Reset();
							}
							
							dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "                ");
							dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "Pokey Out");
							dsLCD->UpdateLCD();
						} else {
							solenoidPokey.Set(SOLENOID_POKEY_IN_DIRECTION);
							
							dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "                ");
							dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "Pokey In");
							dsLCD->UpdateLCD();
						}
				
				GetWatchdog().SetEnabled(false);
				driveTrain.SetSafetyEnabled(false);
				
				compressorPump->Stop();
			}
		}
};

START_ROBOT_CLASS(Robot2010Overkill);
