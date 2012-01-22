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

//ddefining drive motors
#define LEFTMOTOR1PORT 5 //left motor 1 is port 5 (digital side car PWM out)
#define LEFTMOTOR2PORT 8 //left motor 2 is port 8 (digital side car PWM out)
#define RIGHTMOTOR1PORT 7 //right motor 1 is port 7 (digital side car PWM out)
#define RIGHTMOTOR2PORT 6 //right motor 2 is port 6 (digital side car PWM out)

//defining joystick
#define LEFTDRIVESTICK 3 // left joystick is classmate usb port 3 (drive)
#define RIGHTDRIVESTICK 2 //right joystick is classmate usb port 2 (drive)
#define MUNIPULATOR 1 // manipulator joystick is usb port 1

//defining manipulator functions
#define BALLMAGNET1PORT 1 // ball magnet is port 1 digital side car)
#define BALLMAGNET2PORT 2 //ballmagnet is on port 2 digital side car

//crio defining ports


//defining solenoids
#define LATCHCHANNEL 1 //Solenoid 1 analog output 1
#define LATCHCHANNELSHIFT 4 //shifter solonid is in channel 4
#define POKEYBLACK 6 //pokey black is in channel 6
#define POKEYBLUE 5 //pokey blue is on channel 5
#define RELEASE 3 //kicker releaser is on channel 3
#define RELOADBLUE 1 //reload blue is on channel 1
#define RELOADBLACK 2 // reload balck is on chanel 2


#define COMPRPORT 1 //compressor spike out digital relay 1
#define PSWITCHPORT 1 //Pressure switch digital input port number

#define SENSOR 2
#define SENSORIN 1

#define PRESSURESWITCH 5




class RobotDemo : public SimpleRobot
{
	Victor leftmotor1; //the motors are controlled by victors
	Victor leftmotor2; //the motors are controlled by victors
	Victor rightmotor1; //the motors are controlled by victors
	Victor rightmotor2; //the motors are controlled by victors
	Victor ballmagnet1; //the motors are controlled by victors
	Victor ballmagnet2; //the motors are controlled by victors
	
	RobotDrive myRobot; // robot drive system
	Joystick leftdrivestick; //leftdrivestick is a joystick
	Joystick rightdrivestick; //rightdrive stick is a joystick
	Joystick manipulator; //munipulator is a joystick
	Solenoid latch; //latch is a solonoid
	Solenoid latchshift; //latchshift is a solonid
	Solenoid release; //release is a solenoid
	DoubleSolenoid pokey; //pokey is a double action solenoid
	DoubleSolenoid reload; // the reload feature is a double solenoid
	
	DigitalInput sensor;
	Compressor *cmprss;
	
	
	
public:
	RobotDemo(void):
		leftmotor1 (LEFTMOTOR1PORT),
		leftmotor2 (LEFTMOTOR2PORT),
		rightmotor1 (RIGHTMOTOR1PORT),
		rightmotor2 (RIGHTMOTOR2PORT),
		ballmagnet1 (BALLMAGNET1PORT),
		ballmagnet2 (BALLMAGNET2PORT),
		myRobot (leftmotor1, leftmotor2, rightmotor1, rightmotor2),
		leftdrivestick(LEFTDRIVESTICK),
		rightdrivestick(RIGHTDRIVESTICK),
		manipulator(MUNIPULATOR),
		latch(LATCHCHANNEL),
		latchshift(LATCHCHANNELSHIFT),
		release(RELEASE),
		pokey (POKEYBLACK, POKEYBLUE), //the pokey is is made of pokey blue and pokey black
		reload (RELOADBLACK, RELOADBLUE), //the reload is mede of reload black and reload blue
		sensor(SENSOR)
		{
		myRobot.SetExpiration(0.1);
		cmprss = new Compressor(COMPRPORT, PSWITCHPORT);
		cmprss->Start();
		DriverStationLCD *dsLCD = DriverStationLCD::GetInstance();
		dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "                     ");
		dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "                     ");
		dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "                     ");
		dsLCD->UpdateLCD();
		dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Kicker In");
		dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "Pokey In");
		dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "High Gear");
		dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "Sucking Power:");
		dsLCD->UpdateLCD();
	}

	/**
	 * Drive left & right motors for 2 seconds then stop
	 */
	void Autonomous(void)
	{
		myRobot.SetSafetyEnabled(false);
		latchshift.Set(1);
		myRobot.Drive(0.5, 0.0); 	// drive forwards half speed
		Wait(2.0); 				// wait for 2 seconds
		myRobot.Drive(0.0, 0.0); 	// stop robot

	}

	/**
	 * Runs the motors with tank drive.
	 */
	void OperatorControl(void)
	{
		myRobot.SetSafetyEnabled(true);
		Timer *kickTimer = new Timer(); //kicker timer
		int kickState = 0; //kicker state
		int readyComp = 0; //compressor ready
		DriverStationLCD *dsLCD = DriverStationLCD::GetInstance();
		


		
		while (IsOperatorControl())
		{		
		
		
//this part controls the shifter, keeping it in low gear; you can use any drive joytick to contol it
			if(leftdrivestick.GetRawButton(1) || rightdrivestick.GetRawButton(1)) //pneumatics if statement for shifter
						{
					latchshift.Set(1); //Open Solenoid latch shifter
					DriverStationLCD *dsLCD = DriverStationLCD::GetInstance();
					dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "                ");
					dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "High Gear");
					dsLCD->UpdateLCD();
							
					}else {
					latchshift.Set(0);
					DriverStationLCD *dsLCD = DriverStationLCD::GetInstance();
					dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "                ");
					dsLCD->Printf(DriverStationLCD::kUser_Line3, 1, "Low Gear");
					dsLCD->UpdateLCD();//keep latch open
				}
			
			
			myRobot.TankDrive(leftdrivestick, rightdrivestick);
			
			
//pokey program left joystick or right joystick button 3 to make pokey go out.
			if(leftdrivestick.GetRawButton(3) || rightdrivestick.GetRawButton(3))//any joystick controls pokey
			{
				DriverStationLCD *dsLCD = DriverStationLCD::GetInstance();
				dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "                ");
				dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "Pokey Out");
				dsLCD->UpdateLCD();
				pokey.Set(DoubleSolenoid::kReverse); //pokey.Set(Value);
			
			
			}else{
				pokey.Set(DoubleSolenoid::kForward);
				DriverStationLCD *dsLCD = DriverStationLCD::GetInstance();
				dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "                ");
				dsLCD->Printf(DriverStationLCD::kUser_Line2, 1, "Pokey In");
				dsLCD->UpdateLCD();
			}
			
//This code controls the kick funtion. It is made to kick and then to reload back to possition.		
			if(manipulator.GetRawButton(1))

			{	kickTimer->Reset();
				kickTimer->Start();
				reload.Set(DoubleSolenoid::kReverse);
				release.Set(1);
			}else{
			
			if((sensor.Get()==1)||(kickTimer->Get()>=1.5)) 
			{release.Set(0);
			
				
			
			
			}else if ((sensor.Get()==0)||(kickTimer->Get()>=1.5)) 
			{reload.Set(DoubleSolenoid::kForward);
			
			}if (((kickTimer->Get()>=2)) )
			{reload.Set(DoubleSolenoid::kReverse);
			kickTimer->Stop();
						kickTimer->Reset();
			}
			}

//ballmagnet program uses both ballmagnet motors		
			if(manipulator.GetRawButton(2)) //ball magnet in
			{
				
				float x = manipulator.GetThrottle();
					x++;
												x = 0.25*x;
									//		x = -x; //inversion for wingman joystick
												x = x;
												ballmagnet1.Set(x);
												ballmagnet2.Set(x);
												DriverStationLCD *dsLCD = DriverStationLCD::GetInstance();
												dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "                 ");
												dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "Sucking Power: %f", x);
												dsLCD->UpdateLCD();
			
			}
			else if(manipulator.GetRawButton(3)) //ball magnet out
			{
				ballmagnet1.Set(-0.2);
				ballmagnet2.Set(-0.2);
				DriverStationLCD *dsLCD = DriverStationLCD::GetInstance();
				dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "                          ");
				dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "Expelling");
				dsLCD->UpdateLCD();
			}
			else //ball magnet off
			{

				DriverStationLCD *dsLCD = DriverStationLCD::GetInstance();
								dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "                       ");
								dsLCD->Printf(DriverStationLCD::kUser_Line4, 1, "Ball Magnet:Off");
								dsLCD->UpdateLCD();
								ballmagnet1.Set(0.0);
								ballmagnet2.Set(0.0);
				
			}
		
		
		}}
	
};

START_ROBOT_CLASS(RobotDemo);

