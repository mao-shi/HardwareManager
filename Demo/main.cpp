#include "..\HardwareManager\HardwareManager.h"
#include <iostream>

#pragma comment(lib, "..\\Bin\\HardwareManager.lib")

using namespace HardwareManagerNamespace;

int main()
{
	CComputer Computer;
	Computer.Test();
	//CCPUTemperature Cpu;

	//CPUTemperature Temperature = { 0 };
	//Cpu.GetTemperature(Temperature);
	//CPUPerformance Performance = { 0 };
	//Cpu.GetPerformance(Performance);
	//
	//printf("cpu temperature: %d¡æ\n", Temperature.Temperature);
	//printf("cpu performance: %d %d\n", Performance.LoadPercentage, Performance.SpeedPercentage);
	//
	//system("pause");
	return 0;
}
