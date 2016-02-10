#include "WPILib.h"
#include "AHRS.h"

class Robot: public IterativeRobot
{
public:
	LiveWindow *lw = LiveWindow::GetInstance();

	//Out 6
	//In 7
	//Half speed 8

	//Auto Modes
	const std::string AUTO_MODE_OFF = "Off";
	const std::string AUTO_MODE_FULL = "Full";
	const std::string AUTO_MODE_BREACHTEST = "Breach";

	const std::string BREACH_POS_0 = "0";
	const std::string BREACH_POS_1 = "1";
	const std::string BREACH_POS_2 = "2";
	const std::string BREACH_POS_3 = "3";

	const std::string AUTO_DEFTYPE_PORT = "Port";
	const std::string AUTO_DEFTYPE_CHEV = "Checv";
	const std::string AUTO_DEFTYPE_MOAT = "Moat";
	const std::string AUTO_DEFTYPE_RAMPARTS = "Ramp";
	const std::string AUTO_DEFTYPE_BRIDGE = "Bridge";
	const std::string AUTO_DEFTYPE_SALLY = "Sally";
	const std::string AUTO_DEFTYPE_RT = "Rough";
	const std::string AUTO_DEFTYPE_RW = "RockWall";



	//Constants
	double ANGLE_TOLERANCE = 1;   //Degrees
	double XY_TOLERANCE = 0.1;  //Meters

	//Vision Constants
	double TARGET_ORIGIN_X = 250;
	double TARGET_ORIGIN_Y = 250;
	double ORIGIN_X_TOL = 10;
	double ORIGIN_Y_TOL = 10;

	//Auto Modes
	SendableChooser *autoMode;
	SendableChooser *def[4];
	SendableChooser *toBreach;

	std::string autoSelected;
	std::string defense[4];
	int breachPos;

	int autoState;


	//Drive
	Victor left, right;
	RobotDrive drive;

	//Sensors
	AHRS *ahrs;


	//Human input
	Joystick mainStick;
	Joystick specials;


	//VISION_____________________
	//A structure to hold measurements of a particle
	struct ParticleReport {
		double PercentAreaToImageArea;
		double Area;
		double BoundingRectLeft;
		double BoundingRectTop;
		double BoundingRectRight;
		double BoundingRectBottom;
	};

	//Structure to represent the scores for the various tests used for target identification
	struct Scores {
		double Area;
		double Aspect;
	};

	//Images
	Image *frame;
	Image *binaryFrame;

	//Constants
	Range RING_HUE_RANGE = {101, 64};	//Default hue range for ring light
	Range RING_SAT_RANGE = {88, 255};	//Default saturation range for ring light
	Range RING_VAL_RANGE = {230, 255};	//Default value range for ring light
	double AREA_MINIMUM = 0.5; //Default Area minimum for particle as a percentage of total image area
	double SCORE_MIN = 75.0;  //Minimum score to be considered a tote
	double VIEW_ANGLE = 60; //View angle fo camera, set to Axis m1011 by default, 64 for m1013, 51.7 for 206, 52 for HD3000 square, 60 for HD3000 640x480
	ParticleFilterCriteria2 criteria[1];
	ParticleFilterOptions2 filterOptions = {0,0,1,1};
	Scores scores;

	IMAQdxSession session;
	int imaqError;

	double screenPosX;
	double screenPosY;

	//PCM LOCS
	const static int PCMA = 0;
	const static int PCMB = 1;

	//PDP
	PowerDistributionPanel pdp;


	//Pneumatics
	DoubleSolenoid shifter;

	DoubleSolenoid goingUpA;

	//Shooter
	CANTalon shooter;
	CANTalon shooterB;
	DoubleSolenoid shootyStick;

	Compressor compressor;

	//CANTalon angleMotor;
	//CANTalon lift;

	//CANTalon shootyMotor;

	/*--------------------------------------------------------------
	 *						Initialization
	 * -------------------------------------------------------------
	 */

	Robot():
		left(1),right(2),drive(left,right),mainStick(0),specials(1),pdp(0),
		shifter(PCMA,0,1),goingUpA(PCMA,2,3)
	,shooter(0),shooterB(1),shootyStick(PCMB,0,1),compressor(PCMA)//,
	//angleMotor(2),lift(3)
	//,shootyMotor(x)
	{

		//shooterB.SetControlMode(CANSpeedController::kFollower);
		shooterB.SetInverted(false);
		shooter.SetInverted(true);
		//shooterB.Set(10);

		//angleMotor.SetPositionMode();
	}


	void RobotInit()
	{
		autoMode = new SendableChooser();
		autoMode->AddDefault("Auto Off", (void*)&AUTO_MODE_OFF);
		autoMode->AddObject("Full Auto", (void*)&AUTO_MODE_FULL);
		autoMode->AddObject("Breach Only (TEST)",(void*)&AUTO_MODE_BREACHTEST);
		SmartDashboard::PutData("Auto Modes", autoMode);


		try {
			/* Communicate w/navX MXP via the MXP SPI Bus.                                       */
			/* Alternatively:  I2C::Port::kMXP, SerialPort::Port::kMXP or SerialPort::Port::kUSB */
			/* See http://navx-mxp.kauailabs.com/guidance/selecting-an-interface/ for details.   */
			ahrs = new AHRS(SPI::Port::kMXP);
		} catch (std::exception ex) {
			std::string err_string = "Error instantiating navX MXP:  ";
			err_string += ex.what();
			DriverStation::ReportError(err_string.c_str());
		}

		toBreach = new SendableChooser();
		toBreach->AddDefault("2",(void*)&BREACH_POS_0);
		toBreach->AddObject("3",(void*)&BREACH_POS_1);
		toBreach->AddObject("4",(void*)&BREACH_POS_2);
		toBreach->AddObject("5",(void*)&BREACH_POS_3);

		SmartDashboard::PutData("Breach", toBreach);

		for(int j=0;j<4;j++)
		{
			def[j] = new SendableChooser();

			//Add each option
			def[j]->AddDefault("Portcullis", (void*)&AUTO_DEFTYPE_PORT);
			def[j]->AddObject("Cheval", (void*)&AUTO_DEFTYPE_CHEV);
			def[j]->AddObject("Moat", (void*)&AUTO_DEFTYPE_MOAT);
			def[j]->AddObject("Ramparts", (void*)&AUTO_DEFTYPE_RAMPARTS);
			def[j]->AddObject("Drawbridge", (void*)&AUTO_DEFTYPE_BRIDGE);
			def[j]->AddObject("Sally Door", (void*)&AUTO_DEFTYPE_SALLY);
			def[j]->AddObject("Rock Wall", (void*)&AUTO_DEFTYPE_RW);
			def[j]->AddObject("Rough Terrain", (void*)&AUTO_DEFTYPE_RT);

			SmartDashboard::PutData("Defense"+j, def[j]);
		}

		//IMAGE__________________
		//Create image stuff
		binaryFrame = imaqCreateImage(IMAQ_IMAGE_U8,0);
		frame = imaqCreateImage(IMAQ_IMAGE_RGB, 0);
		//the camera name (ex "cam0") can be found through the roborio web interface
		imaqError = IMAQdxOpenCamera("cam0", IMAQdxCameraControlModeController, &session);
		if(imaqError != IMAQdxErrorSuccess) {
			DriverStation::ReportError("IMAQdxOpenCamera error: " + std::to_string((long)imaqError) + "\n");
		}
		imaqError = IMAQdxConfigureGrab(session);
		if(imaqError != IMAQdxErrorSuccess) {
			DriverStation::ReportError("IMAQdxConfigureGrab error: " + std::to_string((long)imaqError) + "\n");
		}

		IMAQdxStartAcquisition(session);
	}
	/* ------------------------------------------------------------------------
	 * 									Teleop
	 * ------------------------------------------------------------------------
	 */

	void TeleopInit()
	{

	}

	void TeleopPeriodic()
	{
		IMAQdxGrab(session, frame, true, NULL);
		CameraServer::GetInstance()->SetImage(frame);
		//Wheel drive
		drive.ArcadeDrive(mainStick);

		//Shifter
		if(mainStick.GetRawButton(1))
			shifter.Set(shifter.kForward);
		else
			shifter.Set(shifter.kReverse);

		//Lift
		if(mainStick.GetRawButton(2))
		{
			goingUpA.Set(goingUpA.kForward);
		}
		else
		{
			goingUpA.Set(goingUpA.kReverse);
		}

		double speed = 0;
		if(specials.GetRawButton(3)) speed=1;
		else if(specials.GetRawButton(2)) speed=-1;
		double mult = -specials.GetRawAxis(2);
		mult+=1;
		mult*=0.5;

		speed*=mult;
		shooter.Set(speed);
		shooterB.Set(speed);


		//TODO: add American port
		if(specials.GetRawButton(1))
		{
			shootyStick.Set(shootyStick.kForward);
			//shootyMotor.Set(1);
		}
		else
		{
			shootyStick.Set(shootyStick.kReverse);
			//shootyMotor.Set(0);
		}

		//Shooter angle TODO Activate shooter angle & add smart systems
		/*
		if(specials.GetRawButton(6)||specials.GetRawButton(11))
		{
			angleMotor.Set(1);
		}
		else if(specials.GetRawButton(7) || specials.GetRawButton(10))
		{
			angleMotor.Set(-1);
		}
		 */

	}


	/* ---------------------------------------------------------------
	 * 								Test & Disable
	 * ---------------------------------------------------------------
	 */
	void TestPeriodic()
	{
		lw->Run();
	}

	void DisabledPeriodic()
	{
		drive.ArcadeDrive(0.0,0);
		shooter.Set(0);
	}

	/**----------------------------------------------------------------------
	 * 							Autonomous
	 * ----------------------------------------------------------------------
	 */
	void AutonomousInit()
	{
		autoState = 0;
		drive.ArcadeDrive(0.0,0);
		drive.SetSafetyEnabled(false);
		left.SetSafetyEnabled(false);
		right.SetSafetyEnabled(false);
		//autoMode = (SendableChooser*) SmartDashboard::GetData("Auto Modes");
		//DriverStation::ReportError("Getting Auto mode...\n");
		//DriverStation::ReportError(autoMode->GetSelected());
		autoSelected = *((std::string*)autoMode->GetSelected());
		DriverStation::ReportError("Mode: "+autoSelected);

		//toBreach = (SendableChooser*) SmartDashboard::GetData("Breach");
		std::string pos = *((std::string*)toBreach->GetSelected());
		if(pos==BREACH_POS_0) breachPos=0;
		if(pos==BREACH_POS_1) breachPos=1;
		if(pos==BREACH_POS_2) breachPos=2;
		if(pos==BREACH_POS_3) breachPos=3;

		for(int j=0;j<4;j++)
		{
			defense[j] = *((std::string*)def[j]->GetSelected());
			//DriverStation::ReportError(defense[j]);
		}
		DriverStation::ReportError("Done!");

		ahrs->ResetDisplacement();
		ahrs->ZeroYaw();

	}

	void AutonomousPeriodic()
	{
		//DriverStation::ReportError("Looping");
		if(autoSelected=="Full")
		{
			//Do everything
			switch(autoState)
			{
			case 0: {
				//Move forward 1.12 meters
				if(!NavigateTo(0,1.12)) return;
				autoState++;
				break;
			}
			case 1: {

				//Breach functions are timed, run once only
				Breach(breachPos);
				autoState++;
				break;
			}
			case 2:
			{
				//Drive to position on arc around tower
				//if(!NavigateTo(0,1)) return; TODO Calculate Arc
				break;
			}
			case 3:
			{
				//Initial firing sequence

				//Raise angle up

				break;
			}
			case 4:
			{
				//Confirm & adjust using vision

				if(!AutoAim()) break;
				autoState++;
				break;
			}
			case 5:
			{
				//Fire
				shooter.Set(1);
				shooterB.Set(1);
				//Let motors spin up
				Wait(1);

				//Fire TODO American port
				shootyStick.Set(shootyStick.kForward);
				//shootyMotor.Set(1);
				Wait(1);
				autoState++;

				break;
			}
			case 6:
			{
				//done
				drive.ArcadeDrive(0.0,0);
				shooter.Set(0);
				shooterB.Set(0);
				shootyStick.Set(shootyStick.kReverse);
				//shootyMotor.Set(0);
				break;
			}
			}
		}
		else if(autoSelected=="Breach")
		{
			//Breach only
			switch(autoState)
			{
			case 0: {
				Breach(breachPos);
				autoState++;
				break;
			}
			case 1: {
				//Done
				drive.ArcadeDrive(0.0,0);
			}
			}
		}
	}





	/*------------------------------------------------------------------------
	 *                        BREACHING        TODO Write breach protocols
	 * -----------------------------------------------------------------------
	 */

	//Switcher: call to activate breach type
	void Breach(int toBreachPos)
	{
		if(defense[toBreachPos]==AUTO_DEFTYPE_PORT){
			BreachPortcullis();
		}
		if(defense[toBreachPos]==AUTO_DEFTYPE_CHEV){
			BreachCheval();
		}
		if(defense[toBreachPos]==AUTO_DEFTYPE_MOAT){
			BreachMoat();
		}
		if(defense[toBreachPos]==AUTO_DEFTYPE_RAMPARTS){
			BreachRamparts();
		}
		if(defense[toBreachPos]==AUTO_DEFTYPE_BRIDGE){
			BreachDrawbridge();
		}
		if(defense[toBreachPos]==AUTO_DEFTYPE_SALLY){
			BreachSally();
		}
		if(defense[toBreachPos]==AUTO_DEFTYPE_RW){
			BreachRockWall();
		}
		if(defense[toBreachPos]==AUTO_DEFTYPE_RT){
			BreachRoughTerrain();
		}
	}


	//Functions to breach each type, excluding lowbar

	void BreachPortcullis()
	{
		DriverStation::ReportError("Breaching Portcullis");
	}

	void BreachCheval()
	{
		DriverStation::ReportError("Breaching Cheval");
	}

	void BreachMoat()
	{
		DriverStation::ReportError("Breaching Moat");
	}

	void BreachRamparts()
	{
		DriverStation::ReportError("Breaching Ramparts");
	}

	void BreachDrawbridge()
	{
		DriverStation::ReportError("Breaching Drawbridge");
	}

	void BreachSally()
	{
		DriverStation::ReportError("Breaching Sally Port");
	}

	void BreachRockWall()
	{
		DriverStation::ReportError("Breaching Rock Wall");
	}

	void BreachRoughTerrain()
	{
		DriverStation::ReportError("Breaching Rough Terrain");
	}

	/*----------------------------------------------------------------------
	 * 							NavX-MXP Navigation    TODO Test this
	 * ---------------------------------------------------------------------
	 */

	//IMPORTANT: All accel values ARE displaced by current orientation; as such, so are dispacements!!!
	bool NavigateTo(double xTarget, double yTarget)
	{
		//Get current location (swapped due to 3d to 2d conversion)
		double currentY = ahrs->GetDisplacementX();
		double currentX = ahrs->GetDisplacementY();

		//Get offset
		double xOff = xTarget-currentX;
		double yOff = yTarget-currentY;

		if(abs(xOff)<XY_TOLERANCE && abs(yOff)<XY_TOLERANCE)
		{
			//At or close enough to location, true=proceed to next action
			return true;
		}

		//Not at location yet
		//Turn towoards target, stop this execution if turning
		//Adjust if xOff is exactly 0 to prevent y/0 crash
		if(xOff==0) xOff = 0.0000000000001;
		double angle = atan(abs(yOff)/abs(xOff));

		//If still rotating, return false to halt execution
		//Manually select quadrant bc atan sucks
		if(xOff>0&&yOff>0)
		{
			if(!RotateToAngle(90-angle)) return false;
		}
		else if(xOff<0&&yOff>0)
		{
			if(!RotateToAngle(-(90-angle))) return false;
		}
		else if(xOff>0&&yOff<0)
		{
			if(!RotateToAngle(90+angle)) return false;
		}
		else if(xOff>0&&yOff>0)
		{
			if(!RotateToAngle(-(90+angle))) return false;
		}

		//If we are still here, we are facing in right direction. Onward!
		drive.ArcadeDrive(1,0);

		return false;
	}


	bool RotateToAngle(double targetAngle)
	{
		double currentAngle = ahrs->GetYaw();

		//If at angle already, stop and return
		if(currentAngle>targetAngle-ANGLE_TOLERANCE && currentAngle<targetAngle+ANGLE_TOLERANCE)
		{
			return true;  //true=proceed to next action
		}

		//Not at angle, go to it
		if(currentAngle>targetAngle)
		{
			//Turn left
			drive.ArcadeDrive(0.0,-1);
		}
		else
		{
			//Turn right
			drive.ArcadeDrive(0.0,1);
		}
		return false;
	}





	/* ----------------------------------------------------------------------
	 * 							Image Processing     TODO Test Vision
	 * ----------------------------------------------------------------------
	 */


	bool AutoAim()
	{
		VisionStuff();
		if(screenPosX>TARGET_ORIGIN_X-ORIGIN_X_TOL && screenPosX<TARGET_ORIGIN_X+ORIGIN_X_TOL && screenPosY>TARGET_ORIGIN_Y-ORIGIN_Y_TOL && screenPosY<TARGET_ORIGIN_Y+ORIGIN_Y_TOL)
		{
			return true;  //true=proceed to next action
		}
		else
		{
			//We need to adjust
			//Check L/R
			if(screenPosX>TARGET_ORIGIN_X-ORIGIN_X_TOL && screenPosX<TARGET_ORIGIN_X+ORIGIN_X_TOL)
			{
				//Need vert adjustment
				drive.ArcadeDrive(0.0,0.0);
				/*
				if(screenPosY<TARGET_ORIGIN_Y)
				{
					//Decrease angle
					angleMotor.Set(-0.3);
				}
				else
				{
					//Increase angle
					angleMotor.Set(0.3);
				}
				 */
			}
			else
			{
				//Need horiz adjustment
				if(screenPosX>TARGET_ORIGIN_X)
				{
					//Turn right
					drive.ArcadeDrive(0.0,0.3);
				}
				else
				{
					drive.ArcadeDrive(0.0,-0.3);
				}
			}
		}
		return false;
	}


	void VisionStuff()
	{
		IMAQdxGrab(session, frame, true, NULL);
		//Threshold the image looking for ring light color
		imaqError = imaqColorThreshold(binaryFrame, frame, 255, IMAQ_HSV, &RING_HUE_RANGE, &RING_SAT_RANGE, &RING_VAL_RANGE);

		//Count particles in the image
		int numParticles = 0;
		imaqError = imaqCountParticles(binaryFrame, 1, &numParticles);
		SmartDashboard::PutNumber("Masked particles", numParticles);

		//Send masked image to dashboard to assist in tweaking mask.
		SendToDashboard(binaryFrame, imaqError);

		//filter out small particles
		float areaMin = SmartDashboard::GetNumber("Area min %", AREA_MINIMUM);
		criteria[0] = {IMAQ_MT_AREA_BY_IMAGE_AREA, areaMin, 100, false, false};
		imaqError = imaqParticleFilter4(binaryFrame, binaryFrame, criteria, 1, &filterOptions, NULL, NULL);

		//Send particle count after filtering to dashboard
		imaqError = imaqCountParticles(binaryFrame, 1, &numParticles);
		SmartDashboard::PutNumber("Filtered particles", numParticles);

		if(numParticles > 0) {
			//Measure particles and sort by particle size
			std::vector<ParticleReport> particles;
			for(int particleIndex = 0; particleIndex < numParticles; particleIndex++)
			{
				ParticleReport par;
				imaqMeasureParticle(binaryFrame, particleIndex, 0, IMAQ_MT_AREA_BY_IMAGE_AREA, &(par.PercentAreaToImageArea));
				imaqMeasureParticle(binaryFrame, particleIndex, 0, IMAQ_MT_AREA, &(par.Area));
				imaqMeasureParticle(binaryFrame, particleIndex, 0, IMAQ_MT_BOUNDING_RECT_TOP, &(par.BoundingRectTop));
				imaqMeasureParticle(binaryFrame, particleIndex, 0, IMAQ_MT_BOUNDING_RECT_LEFT, &(par.BoundingRectLeft));
				imaqMeasureParticle(binaryFrame, particleIndex, 0, IMAQ_MT_BOUNDING_RECT_BOTTOM, &(par.BoundingRectBottom));
				imaqMeasureParticle(binaryFrame, particleIndex, 0, IMAQ_MT_BOUNDING_RECT_RIGHT, &(par.BoundingRectRight));
				particles.push_back(par);
			}
			sort(particles.begin(), particles.end(), CompareParticleSizes);

			ParticleReport best = particles.at(0);
			//Origin is average of edges
			screenPosX = (best.BoundingRectRight+best.BoundingRectLeft)/2;
			screenPosY = (best.BoundingRectBottom+best.BoundingRectTop)/2;

			SmartDashboard::PutNumber("Target X", screenPosX);
			SmartDashboard::PutNumber("Target Y", screenPosY);


		} else {

		}
	}

	void SendToDashboard(Image *image, int error)
	{
		//DriverStation::ReportError("Sending to dashboard");
		if(error < ERR_SUCCESS) {
			DriverStation::ReportError("Send To Dashboard error: " + std::to_string((long)imaqError) + "\n");
		} else {
			DriverStation::ReportError("[ERROR]Sending...");
			CameraServer::GetInstance()->SetImage(binaryFrame);
			DriverStation::ReportError("[ERROR]Sent!");
		}
	}

	//Comparator function for sorting particles. Returns true if particle 1 is larger
		static bool CompareParticleSizes(ParticleReport particle1, ParticleReport particle2)
		{
			//we want descending sort order
			return particle1.PercentAreaToImageArea > particle2.PercentAreaToImageArea;
		}

};

START_ROBOT_CLASS(Robot)
