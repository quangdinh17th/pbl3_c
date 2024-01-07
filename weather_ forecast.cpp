#include <iostream>
#include <cmath>

using namespace std;
int main(){
	float curr_P = 0; 	//pressure from sensor
	float T = 0;		//temperature from sensor
	float h = 0;		//altitude from sensor
	float P0 = 0;
	float z = 0;		//forecast number
	int month = 0;
	//Measure pressure first time, not calculate
	cout<<"\nFirst pressure: ";
	cin>>curr_P;
	float prev_P = curr_P; //pressure of last measurement
	while(1){
		//read pressure sensor
		cout<<"\nCurrent pressure: ";
		cin>>curr_P;
		cout<<"\nTemperature: ";
		cin>>T;
		cout<<"\nAltitude: ";
		cin>>h;
		//read time
		cout<<"\nMonth: ";
		cin>>month;
		//calculate P0
		P0 = curr_P*pow((1 - ((0.0065*h)/(T + (0.0065*h) + 273.15))),-5.257);
		//calculate Z - Forecast number
		//pressure trend
		if(curr_P < prev_P){				//falling
			z = 127 - 0.12*P0;
			if(month >= 6 && month < 9){ 	//falling & summer
				z = z - 1;
			}
		}
		else if(curr_P == prev_P){			//steady
			z = 144 -0.13*P0;
		}
		else if(curr_P > prev_P){			//rising
			z = 185-0.16*P0;
			if(month >= 12 && month < 3){	//rising & winter
				z = z + 1;
			}
		}
		z = round(z);
		cout<<"\nZ:"<<z;
		switch((int)z){
				case 1:
					cout<<"\nSettled Fine";
				break;
				case 2:
					cout<<"\nFine Weather";
				break;
				case 3:
					cout<<"\nFine, Becoming Less Settled";
				break;
				case 4:
					cout<<"\nFairly Fine, Showery Later";
				break;
				case 5:
					cout<<"\nShowery, Becoming More Unsettled";
				break;
				case 6:
					cout<<"\nUnsettled, Rain Later";
				break;
				case 7:
					cout<<"\nRain at Times, Worse Later";
				break;
				case 8:
					cout<<"\nRain at Times, Becoming Very Unsettled";
				break;
				case 9:
					cout<<"\nVery Unsettled, Rain";
				break;
				case 10:
					cout<<"\nSettled Fine";
				break;
				case 11:
					cout<<"\nFine Weather";
				break;
				case 12:
					cout<<"\nFine, Possibly Showers";
				break;
				case 13:
					cout<<"\nFairly Fine, Showers Likely";
				break;
				case 14:
					cout<<"\nShowery, Bright Intervals";
				break;
				case 15:
					cout<<"\nChangeable, Some Rain";
				break;
				case 16:
					cout<<"\nUnsettled, Rain at Times";
				break;
				case 17:
					cout<<"\nRain at Frequent Intervals";
				break;
				case 18:
					cout<<"\nVery Unsettled, Rain";
				break;
				case 19:
					cout<<"\nStormy, Much Rain";
				break;
				case 20:
					cout<<"\nSettled Fine";
				break;
				case 21:
					cout<<"\nFine Weather";
				break;
				case 22:
					cout<<"\nBecoming Fine";
				break;
				case 23:
					cout<<"\nFairly Fine, Improving";
				break;
				case 24:
					cout<<"\nFairly Fine, Possibly Showers Early";
				break;
				case 25:
					cout<<"\nShowery Early, Improving";
				break;
				case 26:
					cout<<"\nChangeable, Mending";
				break;
				case 27:
					cout<<"\nRather Unsettled, Clearing Later";
				break;
				case 28:
					cout<<"\nUnsettled, Probably Improving";
				break;
				case 29:
					cout<<"\nUnsettled, Short Fine Intervals";
				break;
				case 30:
					cout<<"\nVery Unsettled, Finer at Times";
				break;
				case 31:
					cout<<"\nStormy, Possibly Improving";
				break;
				case 32:
					cout<<"\nStormy, Much Rain";
				break;
				default:
					cout<<"\n";
			}
		prev_P = curr_P;
	}
}