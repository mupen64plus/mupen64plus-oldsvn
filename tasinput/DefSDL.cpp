//
// SDL Input for TASinput
// written by nmn
//


#include "MessageBoxGtk2.h"
#include <SDL.h>
#include <stdio.h>
#include <math.h>
#include "Controller.h"
#include "SDLinput.h"
#include "DefSDL.h"
#include "ConfigGtk2.h"
#inlcude <pthreads.h>

#define PLUGIN_NAME "TAS Input Plugin 0.6 for SDL/GTK+"

#define PI 3.14159265358979f

bool romIsOpen = false;

struct Status
{
	Status()
	{
	}

	void StartThread(int ControllerNumber)
	{
        g_thread_init();
        if (pthread_create(&statusThread, NULL, StatusDlgThreadProc, &dwThreadParam) != 0)
	}

	void StopThread()
	{
		if(statusDlg)
		{
			GtkDestroy(statusDlg);
			statusDlg = NULL;
		}
        if(statusThread)
        {
            pthread_cancel(statusThread);
		}
	}

	void EnsureRunning()
	{
		if(!statusDlg || !statusThread || !initialized)
		{
			if(statusDlg) DestroyWindow(statusDlg), statusDlg = NULL;
			if(statusThread) TerminateThread(statusThread, 0), statusThread = NULL;
			StartThread(Control);
		}
	}

	bool HasPanel(int num) { return 0 != (Extend & (1<<(num-1))); }
	void AddPanel(int num) { Extend |= (1<<(num-1)); }
	void RemovePanel(int num) { Extend &= ~(1<<(num-1)); }

    pthread_t statusThread[4];
    GtkWindow *statusDlg[4];
    GtkWidget analogDisplay;

	void RefreshAnalogPicture();
	void ActivateEmulatorWindow();
	bool IsWindowFromEmulatorProcessActive ();
	static bool IsAnyStatusDialogActive ();
	void GetKeys(BUTTONS * Keys);
	void SetKeys(BUTTONS ControllerInput);
};
int Status::frameCounter = 0;


Status status [NUMBER_OF_CONTROLS];


#define STICKPIC_SIZE (131)

int DllMain ( HINSTANCE hInstance, int fdwReason, PVOID pvReserved)
{    
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			g_hInstance = hInstance;
			break;

		case DLL_PROCESS_DETACH:
			FreeDirectInput();
			break;
	}

	return TRUE;
}

EXPORT void CALL CloseDLL (void)
{

    printf( "["PLUGIN_NAME"]: Closing...\n" );
    // some type of writeout for configuration
}

EXPORT void CALL ControllerCommand ( int Control, BYTE * Command)
{
}

EXPORT void CALL DllAbout ( HWND hParent )
{
	MessageBox(hParent, PLUGIN_NAME"\nFor DirectX 7 or higher\n\nBased on Def's Direct Input 0.54 by Deflection\nTAS Modifications by Nitsuja","About",MB_ICONINFORMATION | MB_OK);
}

EXPORT void CALL DllConfig ( HWND hParent )
{
	if (g_lpDI == NULL)
		Initialize(hParent);
	else
		DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_CONFIGDLG), hParent, (DLGPROC)ConfigDlgProc);

	// make config box restart the input status control if it's been closed
	if(romIsOpen)
		for(int i=0; i<NUMBER_OF_CONTROLS; i++)
			if(Controller[i].bActive)
				status[i].EnsureRunning();
			else
				status[i].StopThread();
}

EXPORT void CALL GetDllInfo ( PLUGIN_INFO * PluginInfo )
{
	PluginInfo->Version = 0x0100;
	PluginInfo->Type = PLUGIN_TYPE_CONTROLLER;
	wsprintf(PluginInfo->Name,PLUGIN_NAME);
}

EXPORT void CALL GetKeys(int Control, BUTTONS * Keys )
{
	
	if(Control >= 0 && Control < NUMBER_OF_CONTROLS && Controller[Control].bActive)
		status[Control].GetKeys(Keys);
	else
		Keys->Value = 0;
}

EXPORT void CALL SetKeys(int Control, BUTTONS ControllerInput)
{
	if(Control >= 0 && Control < NUMBER_OF_CONTROLS && Controller[Control].bActive)
		status[Control].SetKeys(ControllerInput);
}

void Status::GetKeys(BUTTONS * Keys)
{
	gettingKeys = true;

//	if(incrementingFrameNow)
//		frameCounter++;

	BUTTONS	ControllerInput;
	
	//Empty Keyboard Button Info 
	ControllerInput.Value = 0;
	
	if (Controller[Control].bActive == TRUE)
	{
		BYTE     buffer[256];   //Keyboard Info 
		DIJOYSTATE js;          //Joystick Info
		HRESULT  hr;
		int M1Speed = 0, M2Speed = 0;
		bool analogKey = false;

		if (Keys == NULL) { gettingKeys = false; return; }

		for(BYTE devicecount=0; devicecount<Controller[Control].NDevices; devicecount++)
		{		
			BYTE DeviceNum = (BYTE) Controller[Control].Devices[devicecount];
			if (DInputDev[DeviceNum].lpDIDevice != NULL)
			{
				LONG count;
		
				if((DInputDev[DeviceNum].DIDevInst.dwDevType & DIDEVTYPE_KEYBOARD) == DIDEVTYPE_KEYBOARD)
				{
					ZeroMemory( &buffer, sizeof(buffer) );	
					if FAILED(hr = DInputDev[DeviceNum].lpDIDevice->GetDeviceState(sizeof(buffer),&buffer))
					{
						hr = DInputDev[DeviceNum].lpDIDevice->Acquire();
						while( hr == DIERR_INPUTLOST ) 
							hr = DInputDev[DeviceNum].lpDIDevice->Acquire();
						gettingKeys = false;
						return;
					}
						
					for (count=0;count<NUMBER_OF_BUTTONS;count++)
					{
						if ( Controller[Control].Input[count].Device == DeviceNum )
						{
							switch ( Controller[Control].Input[count].type )
							{
								//Record Keyboard Button Info from Device State Buffer
								case INPUT_TYPE_KEY_BUT:
									if (BUTTONDOWN(buffer, Controller[Control].Input[count].vkey))
									{
										switch (count)
										{
											case 18:
												M1Speed = Controller[Control].Input[count].button; 
												break;

											case 19:
												M2Speed = Controller[Control].Input[count].button;
												break;

											case 0:
											case 1:
											case 2:
											case 3:
												analogKey = true;
												/* fall through */
											default:
												ControllerInput.Value |= Controller[Control].Input[count].button;
												break;
										}
									}
									break;

								default:
									break;
							}
						}
					}
				}

				else if((DInputDev[DeviceNum].DIDevInst.dwDevType & DIDEVTYPE_JOYSTICK) == DIDEVTYPE_JOYSTICK)
				{
					if FAILED(hr = DInputDev[DeviceNum].lpDIDevice->Poll())
					{
						hr = DInputDev[DeviceNum].lpDIDevice->Acquire();
						while( hr == DIERR_INPUTLOST ) 
							hr = DInputDev[DeviceNum].lpDIDevice->Acquire();
						gettingKeys = false;
						return;
					}
					if FAILED( hr = DInputDev[DeviceNum].lpDIDevice->GetDeviceState( sizeof(DIJOYSTATE), &js ) )
					{
						gettingKeys = false;
						return;
					}
				    
					for (count=0;count<NUMBER_OF_BUTTONS;count++)
					{
						if ( Controller[Control].Input[count].Device == DeviceNum )
						{
							BYTE count2;
							switch ( Controller[Control].Input[count].type ) 
							{
								//Get Joystick button Info from Device State js stucture
								case INPUT_TYPE_JOY_BUT:
									if (BUTTONDOWN(js.rgbButtons, Controller[Control].Input[count].vkey))
									{
										switch (count)
										{
											case 18:
												M1Speed = Controller[Control].Input[count].button; 
												break;

											case 19:
												M2Speed = Controller[Control].Input[count].button;
												break;

											case 0:
											case 1:
											case 2:
											case 3:
												analogKey = true;
												/* fall through */
											default:
												ControllerInput.Value |= Controller[Control].Input[count].button;
												break;
										}
									}
									break;
						
								case INPUT_TYPE_JOY_AXIS:
									switch (Controller[Control].Input[count].vkey)
									{
										case DIJOFS_YN:
											if( js.lY < (LONG) -Controller[Control].SensMin )
												GetNegAxisVal(js.lY, Control, count, &ControllerInput, M1Speed, M2Speed);
											break;
										case DIJOFS_YP:
											if( js.lY > (LONG) Controller[Control].SensMin )
												GetPosAxisVal(js.lY, Control, count, &ControllerInput, M1Speed, M2Speed);
											break;
										case DIJOFS_XN:
											if( js.lX < (LONG) -Controller[Control].SensMin )
												GetNegAxisVal(js.lX, Control, count, &ControllerInput, M1Speed, M2Speed);
											break;
										case DIJOFS_XP:
											if( js.lX > (LONG) Controller[Control].SensMin )
												GetPosAxisVal(js.lX, Control, count, &ControllerInput, M1Speed, M2Speed);
											break;
										case DIJOFS_ZN:
											if( js.lZ < (LONG) -Controller[Control].SensMin )
												GetNegAxisVal(js.lZ, Control, count, &ControllerInput, M1Speed, M2Speed);
											break;
										case DIJOFS_ZP:
											if( js.lZ > (LONG) Controller[Control].SensMin )
												GetPosAxisVal(js.lZ, Control, count, &ControllerInput, M1Speed, M2Speed);
											break;
										case DIJOFS_RYN:
											if( js.lRy < (LONG) -Controller[Control].SensMin )
												GetNegAxisVal(js.lRy, Control, count, &ControllerInput, M1Speed, M2Speed);
											break;
										case DIJOFS_RYP:
											if( js.lRy > (LONG) Controller[Control].SensMin )
												GetPosAxisVal(js.lRy, Control, count, &ControllerInput, M1Speed, M2Speed);
											break;
										case DIJOFS_RXN:
											if( js.lRx < (LONG) -Controller[Control].SensMin )
												GetNegAxisVal(js.lRx, Control, count, &ControllerInput, M1Speed, M2Speed);
											break;
										case DIJOFS_RXP:
											if( js.lRx > (LONG) Controller[Control].SensMin )
												GetPosAxisVal(js.lRx, Control, count, &ControllerInput, M1Speed, M2Speed);
											break;
										case DIJOFS_RZN:
											if( js.lRz < (LONG) -Controller[Control].SensMin )
												GetNegAxisVal(js.lRz, Control, count, &ControllerInput, M1Speed, M2Speed);
											break;
										case DIJOFS_RZP:
											if( js.lRz > (LONG) Controller[Control].SensMin )
												GetPosAxisVal(js.lRz, Control, count, &ControllerInput, M1Speed, M2Speed);
											break;
										case DIJOFS_SLIDER0N:
											if( js.rglSlider[0] < (LONG) -Controller[Control].SensMin )
												GetNegAxisVal(js.rglSlider[0], Control, count, &ControllerInput, M1Speed, M2Speed);
											break;
										case DIJOFS_SLIDER0P:
											if( js.rglSlider[0] > (LONG) Controller[Control].SensMin )
												GetPosAxisVal(js.rglSlider[0], Control, count, &ControllerInput, M1Speed, M2Speed);
											break;
										case DIJOFS_SLIDER1N:
											if( js.rglSlider[1] < (LONG) -Controller[Control].SensMin )
												GetNegAxisVal(js.rglSlider[1], Control, count, &ControllerInput, M1Speed, M2Speed);
											break;
										case DIJOFS_SLIDER1P:
											if( js.rglSlider[1] > (LONG) Controller[Control].SensMin )
												GetPosAxisVal(js.rglSlider[1], Control, count, &ControllerInput, M1Speed, M2Speed);
											break;
									}
									break;
								
								case INPUT_TYPE_JOY_POV:
									for (count2=0;count2<NUMBER_OF_CONTROLS;count2++)
									{
										if ( (js.rgdwPOV[count2] != -1)  && (LOWORD(js.rgdwPOV[count2]) != 0xFFFF) )
										{
											switch(Controller[Control].Input[count].vkey)
											{
												case DIJOFS_POV0N:
												case DIJOFS_POV1N:
												case DIJOFS_POV2N:
												case DIJOFS_POV3N:
													if( (js.rgdwPOV[count2] >= 31500) || (js.rgdwPOV[count2] <= 4500) )
													{
														switch (count)
														{
															case 18:
																M1Speed = Controller[Control].Input[count].button; 
																break;

															case 19:
																M2Speed = Controller[Control].Input[count].button;
																break;

															case 0:
															case 1:
															case 2:
															case 3:
																analogKey = true;
																/* fall through */
															default:
																ControllerInput.Value |= Controller[Control].Input[count].button;
																break;
														}

													}
													break;
												case DIJOFS_POV0E:
												case DIJOFS_POV1E:
												case DIJOFS_POV2E:
												case DIJOFS_POV3E:
													if ( (js.rgdwPOV[count2] >= 4500) && (js.rgdwPOV[count2] <= 13500) )
													{
														switch (count2)
														{
															case 18:
																M1Speed = Controller[Control].Input[count].button; 
																break;

															case 19:
																M2Speed = Controller[Control].Input[count].button;
																break;

															case 0:
															case 1:
															case 2:
															case 3:
																analogKey = true;
																/* fall through */
															default:
																ControllerInput.Value |= Controller[Control].Input[count].button;
																break;
														}
													}
													break;
												case DIJOFS_POV0S:
												case DIJOFS_POV1S:
												case DIJOFS_POV2S:
												case DIJOFS_POV3S:
													if ( (js.rgdwPOV[count2] >= 13500) && (js.rgdwPOV[count2] <= 22500) )
													{
														switch (count2)
														{
															case 18:
																M1Speed = Controller[Control].Input[count].button; 
																break;

															case 19:
																M2Speed = Controller[Control].Input[count].button;
																break;

															case 0:
															case 1:
															case 2:
															case 3:
																analogKey = true;
																/* fall through */
															default:
																ControllerInput.Value |= Controller[Control].Input[count].button;
																break;
														}
													}
													break;
												case DIJOFS_POV0W:
												case DIJOFS_POV1W:
												case DIJOFS_POV2W:
												case DIJOFS_POV3W:
													if ( (js.rgdwPOV[count2] >= 22500) && (js.rgdwPOV[count2] <= 31500) )
													{
														switch (count2)
														{
															case 18:
																M1Speed = Controller[Control].Input[count].button; 
																break;

															case 19:
																M2Speed = Controller[Control].Input[count].button;
																break;

															case 0:
															case 1:
															case 2:
															case 3:
																analogKey = true;
																/* fall through */
															default:
																ControllerInput.Value |= Controller[Control].Input[count].button;
																break;
														}
													}
													break;
											}
										}
									}

								default:
									break;
							}
						}
					}
				}
			}
		}
		
		if(M2Speed)
		{
			if ( ControllerInput.Y_AXIS < 0 ) 
				ControllerInput.Y_AXIS = (char) -M2Speed;
			else if ( ControllerInput.Y_AXIS > 0 )
				ControllerInput.Y_AXIS = (char) M2Speed;
			
			if ( ControllerInput.X_AXIS < 0 )
				ControllerInput.X_AXIS = (char) -M2Speed;
			else if ( ControllerInput.X_AXIS > 0 )
				ControllerInput.X_AXIS = (char) M2Speed;
		}
		if(M1Speed)
		{
			if ( ControllerInput.Y_AXIS < 0 ) 
				ControllerInput.Y_AXIS = (char) -M1Speed;
			else if ( ControllerInput.Y_AXIS > 0 )
				ControllerInput.Y_AXIS = (char) M1Speed;
			
			if ( ControllerInput.X_AXIS < 0 )
				ControllerInput.X_AXIS = (char) -M1Speed;
			else if ( ControllerInput.X_AXIS > 0 )
				ControllerInput.X_AXIS = (char) M1Speed;
		}
		if(analogKey)
		{
			if(ControllerInput.X_AXIS && ControllerInput.Y_AXIS)
			{
				const static float mult = 1.0f / sqrtf(2.0f);
				float mult2;
				if(Controller[Control].SensMax > 127)
					mult2 = (float)Controller[Control].SensMax * (1.0f / 127.0f);
				else
					mult2 = 1.0f;
				if(!relativeXOn)
					ControllerInput.X_AXIS = (int)(ControllerInput.X_AXIS * mult*mult2 + (ControllerInput.X_AXIS>0 ? 0.5f : -0.5f));
				if(!relativeYOn && relativeXOn != 3)
					ControllerInput.Y_AXIS = (int)(ControllerInput.Y_AXIS * mult*mult2 + (ControllerInput.Y_AXIS>0 ? 0.5f : -0.5f));

				int newX = (int)((float)ControllerInput.X_AXIS * xScale + (ControllerInput.X_AXIS>0 ? 0.5f : -0.5f));
				int newY = (int)((float)ControllerInput.Y_AXIS * yScale + (ControllerInput.Y_AXIS>0 ? 0.5f : -0.5f));
				if(abs(newX) >= abs(newY) && (newX > 127 || newX < -128))
				{
					newY = newY * (newY>0 ? 127 : 128) / abs(newX);
					newX = (newX>0) ? 127 : -128;
				}
				else if(abs(newX) <= abs(newY) && (newY > 127 || newY < -128))
				{
					newX = newX * (newX>0 ? 127 : 128) / abs(newY);
					newY = (newY>0) ? 127 : -128;
				}
				if(!newX && ControllerInput.X_AXIS) newX = (ControllerInput.X_AXIS>0) ? 1 : -1;
				if(!newY && ControllerInput.Y_AXIS) newY = (ControllerInput.Y_AXIS>0) ? 1 : -1;
				if(!relativeXOn)
					ControllerInput.X_AXIS = newX;
				if(!relativeYOn && relativeXOn != 3)
					ControllerInput.Y_AXIS = newY;
			}
			else
			{
				if(ControllerInput.X_AXIS && !relativeXOn)
				{
					int newX = (int)((float)ControllerInput.X_AXIS * xScale + (ControllerInput.X_AXIS>0 ? 0.5f : -0.5f));
					if(!newX && ControllerInput.X_AXIS) newX = (ControllerInput.X_AXIS>0) ? 1 : -1;
					ControllerInput.X_AXIS = min(127,max(-128,newX));
				}
				if(ControllerInput.Y_AXIS && !relativeYOn && relativeXOn != 3)
				{
					int newY = (int)((float)ControllerInput.Y_AXIS * yScale + (ControllerInput.Y_AXIS>0 ? 0.5f : -0.5f));
					if(!newY && ControllerInput.Y_AXIS) newY = (ControllerInput.Y_AXIS>0) ? 1 : -1;
					ControllerInput.Y_AXIS = min(127,max(-128,newY));
				}
			}
		}
	}

	ControllerInput.Value |= (buttonOverride.Value &= ~ControllerInput.Value);
//	if((frameCounter/2)%2 == 0)
	if(frameCounter%2 == 0)
		ControllerInput.Value ^= (buttonAutofire.Value &= ~buttonOverride.Value);
	else
		ControllerInput.Value ^= (buttonAutofire2.Value &= ~buttonOverride.Value);

	bool prevOverrideAllowed = overrideAllowed;
	overrideAllowed = true;
	SetKeys(ControllerInput);
	overrideAllowed = prevOverrideAllowed;

	if(overrideOn)
	{
		ControllerInput.X_AXIS = overrideX;
		ControllerInput.Y_AXIS = overrideY;
	}

	//Pass Button Info to Emulator
	Keys->Value = ControllerInput.Value;

	gettingKeys = false;
}

void Status::SetKeys(BUTTONS ControllerInput)
{
	bool changed = false;
	if(statusDlg)
	{
		if(buttonDisplayed.Value != ControllerInput.Value)
		{
			if(HasPanel(2))
			{
#define UPDATECHECK(idc,field) {if(buttonDisplayed.field != ControllerInput.field) CheckDlgButton(statusDlg, idc, ControllerInput.field);}
				UPDATECHECK(IDC_CHECK_A, A_BUTTON);
				UPDATECHECK(IDC_CHECK_B, B_BUTTON);
				UPDATECHECK(IDC_CHECK_START, START_BUTTON);
				UPDATECHECK(IDC_CHECK_L, L_TRIG);
				UPDATECHECK(IDC_CHECK_R, R_TRIG);
				UPDATECHECK(IDC_CHECK_Z, Z_TRIG);
				UPDATECHECK(IDC_CHECK_CUP, U_CBUTTON);
				UPDATECHECK(IDC_CHECK_CLEFT, L_CBUTTON);
				UPDATECHECK(IDC_CHECK_CRIGHT, R_CBUTTON);
				UPDATECHECK(IDC_CHECK_CDOWN, D_CBUTTON);
				UPDATECHECK(IDC_CHECK_DUP, U_DPAD);
				UPDATECHECK(IDC_CHECK_DLEFT, L_DPAD);
				UPDATECHECK(IDC_CHECK_DRIGHT, R_DPAD);
				UPDATECHECK(IDC_CHECK_DDOWN, D_DPAD);
#undef UPDATECHECK
			}
			buttonDisplayed.Value = ControllerInput.Value;
		}

		if(relativeXOn == 3 && radialRecalc)
		{
			radialDistance = sqrtf((float)((overrideX*overrideX)/(xScale*xScale) + (overrideY*overrideY)/(yScale*yScale)));
			if(radialDistance != 0)
				radialAngle = atan2f((float)-(overrideY/yScale), (float)(overrideX/xScale));
			radialRecalc = false;
		}

		// relative change amount
		int addX = (int)((ControllerInput.X_AXIS * xScale) / 12.0f);
		int addY = (int)((ControllerInput.Y_AXIS * yScale) / 12.0f);

		// calculate fractional (over time) change
		if(relativeControlNow && relativeXOn && ControllerInput.X_AXIS)
		{
			static float incrX = 0.0f;
			incrX += ((ControllerInput.X_AXIS * xScale) / 12.0f) - addX;
			if(incrX > 1.0f)
				addX++, incrX -= 1.0f;
			else if(incrX < -1.0f)
				addX--, incrX += 1.0f;
		}
		if(relativeControlNow && (relativeYOn || relativeXOn==3) && ControllerInput.Y_AXIS)
		{
			static float incrY = 0.0f;
			incrY += ((ControllerInput.Y_AXIS * yScale) / 12.0f) - addY;
			if(incrY > 1.0f)
				addY++, incrY -= 1.0f;
			else if(incrY < -1.0f)
				addY--, incrY += 1.0f;
		}

		if(relativeXOn && overrideAllowed)
		{
			if(relativeControlNow && ControllerInput.X_AXIS) // increment x position by amount relative to attempted new x position
			{
				if(relativeXOn != 3)
				{
					int nextVal = overrideX + addX;
					if(nextVal <= 127 && nextVal >= -128)
					{
						if(!overrideX || (relativeXOn==1 ? (((overrideX<0) == (ControllerInput.X_AXIS<0)) || !ControllerInput.X_AXIS) : ((overrideX<0) != (nextVal>0))))
							ControllerInput.X_AXIS = nextVal;
						else
							ControllerInput.X_AXIS = 0; // lock to 0 once on crossing +/- boundary, or "semi-relative" jump to 0 on direction reversal
					}
					else
					{
						ControllerInput.X_AXIS = nextVal>0 ? 127 : -128;
						if(abs(overrideY + addY) < 129)
							overrideY = overrideY * 127 / abs(nextVal);
					}
				}
				else // radial mode, angle change
				{
					radialAngle += 4*PI; // keeping it positive
					float oldAngle = fmodf(radialAngle, 2*PI);
					radialAngle = fmodf(radialAngle + (ControllerInput.X_AXIS) / (250.0f + 500.0f/(sqrtf(xScale*yScale)+0.001f)), 2*PI);

					// snap where crossing 45 degree boundaries
					if(radialAngle-oldAngle > PI) radialAngle -= 2*PI;
					else if(oldAngle-radialAngle > PI) oldAngle -= 2*PI;
					float oldDeg = oldAngle*(180.0f/PI);
					float newDeg = radialAngle*(180.0f/PI);
					for(float ang = 0 ; ang < 360 ; ang += 45)
						if(fabsf(oldDeg - ang) > (0.0001f) && (oldDeg < ang) != (newDeg < ang))
						{
							radialAngle = ang*(PI/180.0f);
							break;
						}

					float xAng = xScale * radialDistance * cosf(radialAngle);
					float yAng = yScale * radialDistance * sinf(radialAngle);
					int newX =  (int)(xAng + (xAng>0 ? 0.5f : -0.5f));
					int newY = -(int)(yAng + (yAng>0 ? 0.5f : -0.5f));
					if(newX > 127) newX = 127;
					if(newX < -128) newX = -128;
					if(newY > 127) newY = 127;
					if(newY < -128) newY = -128;
					ControllerInput.X_AXIS = newX;
					overrideY              = newY;
				}
			}
			else
				ControllerInput.X_AXIS = overrideX;
		}
		if((relativeYOn || (relativeXOn == 3)) && overrideAllowed)
		{
			if(relativeControlNow && (ControllerInput.Y_AXIS || (!relativeYOn && ControllerInput.Y_AXIS != LastControllerInput.Y_AXIS))) // increment y position by amount relative to attempted new y position
			{
				if(relativeXOn != 3) // not a typo (relativeXOn holds radial mode setting)
				{
					int nextVal = overrideY + addY;
					if(nextVal <= 127 && nextVal >= -128)
					{
						if(!overrideY || (relativeYOn==1 ? (((overrideY<0) == (ControllerInput.Y_AXIS<0)) || !ControllerInput.Y_AXIS) : ((overrideY<0) != (nextVal>0))))
							ControllerInput.Y_AXIS = nextVal;
						else
							ControllerInput.Y_AXIS = 0; // lock to 0 once on crossing +/- boundary, or "semi-relative" jump to 0 on direction reversal
					}
					else
					{
						if(abs(overrideX + addX) < 129)
							ControllerInput.X_AXIS = ControllerInput.X_AXIS * 127 / abs(nextVal);
						ControllerInput.Y_AXIS = nextVal>0 ? 127 : -128;
					}
				}
				else // radial mode, length change
				{
					if(!relativeYOn) // relative rotation + instant distance
					{
						if(ControllerInput.Y_AXIS < 0)
							radialDistance = 0;
						else if(!Controller[Control].SensMin || abs(ControllerInput.Y_AXIS) >= Controller[Control].SensMin)
						{
							radialDistance = (float)ControllerInput.Y_AXIS;
							if(radialDistance == 127) radialDistance = 128;
						}
						else
							radialDistance = (float)Controller[Control].SensMin;
						LastControllerInput.Y_AXIS = ControllerInput.Y_AXIS;
					}
					else if(relativeYOn == 1 && radialDistance && (radialDistance>0) != (ControllerInput.Y_AXIS>0))
						radialDistance = 0; // "semi-relative" distance, jump to 0 on direction reversal
					else
						radialDistance += addY;
					const static float maxDist = sqrtf(128*128+128*128);
					if(radialDistance > maxDist) radialDistance = maxDist;
					float xAng = xScale * radialDistance * cosf(radialAngle);
					float yAng = yScale * radialDistance * sinf(radialAngle);
					int newX =  (int)(xAng + (xAng>0 ? 0.5f : -0.5f));
					int newY = -(int)(yAng + (yAng>0 ? 0.5f : -0.5f));
					if(newX > 127) newX = 127;
					if(newX < -128) newX = -128;
					if(newY > 127) newY = 127;
					if(newY < -128) newY = -128;
					ControllerInput.X_AXIS = newX;
					ControllerInput.Y_AXIS = newY;
				}
			}
			else
				ControllerInput.Y_AXIS = overrideY;
		}
		if(LastControllerInput.X_AXIS != ControllerInput.X_AXIS || (AngDisp && LastControllerInput.Y_AXIS != ControllerInput.Y_AXIS))
		{
			if(HasPanel(1))
			{
				char str [256], str2 [256];
				GetDlgItemText(statusDlg, IDC_EDITX, str2, 256);
				if(!AngDisp)
					sprintf(str, "%d", ControllerInput.X_AXIS);
				else
				{
					float radialAngle = 4*PI + atan2f(((float)-ControllerInput.Y_AXIS)/yScale, ((float)ControllerInput.X_AXIS)/xScale);
					float angle2 = fmodf(90.0f + radialAngle*(180.0f/PI), 360.0f);
					sprintf(str, "%d", (int)(angle2 + (angle2>0 ? 0.5f : -0.5f)));
					skipEditX = true;
					overrideX = (int)ControllerInput.X_AXIS;
					RefreshAnalogPicture();
				}
				if(strcmp(str,str2))
					SetDlgItemText(statusDlg, IDC_EDITX, str);
			}
			changed = true;
		}
		if(LastControllerInput.Y_AXIS != ControllerInput.Y_AXIS || (AngDisp && LastControllerInput.X_AXIS != ControllerInput.X_AXIS))
		{
			if(HasPanel(1))
			{
				char str [256], str2 [256];
				GetDlgItemText(statusDlg, IDC_EDITY, str2, 256);
				if(!AngDisp)
					sprintf(str, "%d", -ControllerInput.Y_AXIS);
				else
				{
					float radialDistance = sqrtf(((float)(ControllerInput.X_AXIS*ControllerInput.X_AXIS)/(xScale*xScale) + (float)(ControllerInput.Y_AXIS*ControllerInput.Y_AXIS)/(yScale*yScale)));
					sprintf(str, "%d", (int)(0.5f + radialDistance));
					skipEditY = true;
					overrideY = (int)ControllerInput.Y_AXIS;
					RefreshAnalogPicture();
				}
				if(strcmp(str,str2))
					SetDlgItemText(statusDlg, IDC_EDITY, str);
			}
			changed = true;
		}
	}
	if(relativeYOn || relativeXOn != 3)
		LastControllerInput = ControllerInput;

	if(changed)
	{
		overrideX = ControllerInput.X_AXIS;
		overrideY = ControllerInput.Y_AXIS;
		RefreshAnalogPicture();
	}
}


void GetNegAxisVal(LONG AxisValue, int Control, LONG count, BUTTONS *ControllerInput, int &M1Speed, int &M2Speed)
{
	switch(count)
	{
		case 0:
			if ( AxisValue < (LONG) -Controller[Control].SensMax)
				ControllerInput->Y_AXIS = min(127,Controller[Control].SensMax);
			else
				ControllerInput->Y_AXIS = -AxisValue; 
			break;
		case 1:
			if ( AxisValue < (LONG) -Controller[Control].SensMax)
				ControllerInput->Y_AXIS = -min(128,Controller[Control].SensMax);
			else
				ControllerInput->Y_AXIS = AxisValue;
			break;
		case 2:
			if ( AxisValue < (LONG) -Controller[Control].SensMax)
				ControllerInput->X_AXIS = -min(128,Controller[Control].SensMax);
			else
				ControllerInput->X_AXIS = AxisValue; 
			break;
		case 3:
			if ( AxisValue < (LONG) -Controller[Control].SensMax)
				ControllerInput->X_AXIS = min(127,Controller[Control].SensMax);
			else
				ControllerInput->X_AXIS = -AxisValue; 
			break;
		
		case 18:
			M1Speed = Controller[Control].Input[count].button; 
			break;
		case 19:
			M2Speed = Controller[Control].Input[count].button;
			break;

		default: 
			ControllerInput->Value |= Controller[Control].Input[count].button; 
			break;
	}
}

void GetPosAxisVal(LONG AxisValue, int Control, LONG count, BUTTONS *ControllerInput, int &M1Speed, int &M2Speed) {
	switch(count)
	{
		case 0:
			if ( AxisValue > (LONG) Controller[Control].SensMax)
				ControllerInput->Y_AXIS = min(127,Controller[Control].SensMax);
			else
				ControllerInput->Y_AXIS = AxisValue;
			break;
		case 1:
			if ( AxisValue > (LONG) Controller[Control].SensMax)
				ControllerInput->Y_AXIS = -min(128,Controller[Control].SensMax);
			else
				ControllerInput->Y_AXIS = -AxisValue; 
			break;
		case 2:
			if ( AxisValue > (LONG) Controller[Control].SensMax)
				ControllerInput->X_AXIS = -min(128,Controller[Control].SensMax);
			else
				ControllerInput->X_AXIS = -AxisValue; 
			break;
		case 3:
			if ( AxisValue > (LONG) Controller[Control].SensMax)
				ControllerInput->X_AXIS = min(127,Controller[Control].SensMax);
			else
				ControllerInput->X_AXIS = AxisValue; 
			break;
		
		case 18:
			M1Speed = Controller[Control].Input[count].button; 
			break;
		case 19:
			M2Speed = Controller[Control].Input[count].button;
			break;

		default: 
			ControllerInput->Value |= Controller[Control].Input[count].button; 
			break;
	}
}


EXPORT void CALL InitiateControllers (HWND hMainWindow, CONTROL Controls[4]) 
{
	HKEY hKey;
	int dwSize, dwType;

	for (BYTE i=0; i<NUMBER_OF_CONTROLS; i++) 
    {
		ControlDef[i] = &Controls[i];
		ControlDef[i]->Present = FALSE;
		ControlDef[i]->RawData = FALSE;
		ControlDef[i]->Plugin  = PLUGIN_NONE;

		Controller[i].NDevices = 0;
		Controller[i].bActive = i==0 ? TRUE : FALSE;
		Controller[i].SensMax = 128;
		Controller[i].SensMin = 32;
		Controller[i].Input[18].button = 42;
		Controller[i].Input[19].button = 20;
		wsprintf(Controller[i].szName,"Controller %d",i+1);		
	}

	if (g_lpDI == NULL) 
    {
		Initialize(hMainWindow);
	}
	
	dwType = REG_BINARY;
	dwSize = sizeof(DEFCONTROLLER);

	if ( RegCreateKeyEx(HKEY_CURRENT_USER, SUBKEY, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, 0) != ERROR_SUCCESS ) {
		MessageBox(NULL, "Could not create Registry Key", "Error", MB_ICONERROR | MB_OK);
	}
	else {
		for (BYTE NController = 0; NController<NUMBER_OF_CONTROLS; NController++) {
			if ( RegQueryValueEx(hKey, Controller[NController].szName, 0, &dwType, (LPBYTE)&Controller[NController], &dwSize) == ERROR_SUCCESS ) {
				if ( Controller[NController].bActive )
					ControlDef[NController]->Present = TRUE;
				else
					ControlDef[NController]->Present = FALSE;
				
				if ( Controller[NController].bMemPak )
					ControlDef[NController]->Plugin = PLUGIN_MEMPAK;
				else
					ControlDef[NController]->Plugin = PLUGIN_NONE;

				if ( dwSize != sizeof(DEFCONTROLLER) ) {
					dwType = REG_BINARY;
					dwSize = sizeof(DEFCONTROLLER);
					ZeroMemory( &Controller[NController], sizeof(DEFCONTROLLER) );
					
					Controller[NController].NDevices = 0;
					Controller[NController].bActive = NController==0 ? TRUE : FALSE;
					ControlDef[NController]->Present = FALSE;
					ControlDef[NController]->Plugin  = PLUGIN_NONE;
					Controller[NController].SensMax = 128;
					Controller[NController].SensMin = 32;
					Controller[NController].Input[18].button = 42;
					Controller[NController].Input[19].button = 20;
					wsprintf(Controller[NController].szName,"Controller %d",NController+1);

					RegDeleteValue(hKey, Controller[NController].szName);
					RegSetValueEx(hKey, Controller[NController].szName, 0, dwType, (LPBYTE)&Controller[NController], dwSize);
				}
			}
			else {
				dwType = REG_BINARY;
				dwSize = sizeof(DEFCONTROLLER);
				RegDeleteValue(hKey, Controller[NController].szName);
				RegSetValueEx(hKey, Controller[NController].szName, 0, dwType, (LPBYTE)&Controller[NController], dwSize);
			}
		}		
	}
	RegCloseKey(hKey);
}

void Initialize() 
{	
	//Initialize Direct Input function
    if(!InitSDLInput()) 
    {
		messagebox(NULL,"SDL Initialization Failed!","Error",MB_ICONERROR | MB_OK);
        FreeSDLInput();
	}
}

BOOL CheckForDeviceChange(HKEY hKey) 
{
	BOOL DeviceChanged;
	int dwSize, dwType;
	
	dwType = REG_BINARY;
	dwSize = sizeof(DEFCONTROLLER);

	DeviceChanged = FALSE;
	
	for(BYTE DeviceNumCheck=0; DeviceNumCheck<MAX_DEVICES; DeviceNumCheck++) 
    {
		if( memcmp(&Guids[DeviceNumCheck], &DInputDev[DeviceNumCheck].DIDevInst.guidInstance, sizeof(GUID)) != 0) 
        {
			DeviceChanged = TRUE;			
			for( BYTE NController=0; NController<NUMBER_OF_CONTROLS; NController++) 
            {
				RegQueryValueEx(hKey, Controller[NController].szName, 0, &dwType, (LPBYTE)&Controller[NController], &dwSize);
				for( BYTE DeviceNum=0; DeviceNum<Controller[NController].NDevices; DeviceNum++) 
                {
					if( Controller[NController].Devices[DeviceNum] == DeviceNumCheck) 
                    {
						Controller[NController].NDevices = 0; 
						Controller[NController].bActive = FALSE;
						RegSetValueEx(hKey, Controller[NController].szName, 0, dwType, (LPBYTE)&Controller[NController], dwSize);
					}
				}
			}
		}
	}

	return DeviceChanged;
}

EXPORT void CALL ReadController ( int Control, BYTE * Command )
{
	// XXX: Increment frame counter here because the plugin specification provides no means of finding out when a frame goes by.
	//      Mupen64 calls ReadController(-1) every input frame, but other emulators might not do that.
	//      (The frame counter is used only for autofire and combo progression.)
	if(Control == -1)
		Status::frameCounter++;
//		for(Control = 0; Control < NUMBER_OF_CONTROLS; Control++)
//			if(Controller[Control].bActive)
//				status[Control].frameCounter++;
}

EXPORT void CALL RomClosed (void) {

    // quit SDL joystick subsystem
    SDL_QuitSubSystem( SDL_INIT_JOYSTICK );

    // release/ungrab mouse
    SDL_WM_GrabInput( SDL_GRAB_OFF );
    SDL_ShowCursor( 1 );
    
    romIsOpen = false;
}


EXPORT void CALL RomOpen (void) 
{
	for(int i=0; i<NUMBER_OF_CONTROLS; i++)
    {
		if(Controller[i].bActive)
        {
			status[i].StartThread(i);
        }
		else
        {
			status[i].Control = i;
        }
    }
}

EXPORT void CALL WM_KeyDown( WPARAM wParam, LPARAM lParam )
{
}

EXPORT void CALL WM_KeyUp( WPARAM wParam, LPARAM lParam )
{
}

void Status::RefreshAnalogPicture ()
{
    analogDisplay
}

static bool IsMouseOverControl (HWND hDlg, int dialogItemID)
{
	POINT pt;
    RECT rect;
    
    GetCursorPos(&pt);
    GetWindowRect(GetDlgItem(hDlg, dialogItemID), &rect);
    
    return (pt.x <= rect.right && pt.x >= rect.left && pt.y <= rect.bottom && pt.y >= rect.top);
}

bool Status::IsAnyStatusDialogActive ()
{
	HWND hWnd = GetForegroundWindow();
	if(!hWnd) return false;
	for(int i=0; i<NUMBER_OF_CONTROLS; i++)
		if(hWnd == status[i].statusDlg)
			return true;
	return false;
}