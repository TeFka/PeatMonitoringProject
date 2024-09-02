EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Battery backup circuit"
Date "2023-02-07"
Rev "v01"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 "Author: Loo Chuan Long"
$EndDescr
$Comp
L Device:R R2
U 1 1 63DB84D3
P 5450 3050
F 0 "R2" H 5300 3100 50  0000 L CNN
F 1 "10k" H 5250 3000 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 5380 3050 50  0001 C CNN
F 3 "~" H 5450 3050 50  0001 C CNN
	1    5450 3050
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 63DD1F32
P 5050 3500
F 0 "R1" V 4843 3500 50  0000 C CNN
F 1 "825" V 4934 3500 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 4980 3500 50  0001 C CNN
F 3 "~" H 5050 3500 50  0001 C CNN
	1    5050 3500
	0    1    1    0   
$EndComp
Wire Wire Line
	5450 2900 5450 2650
Connection ~ 5450 2650
Wire Wire Line
	5450 3200 5450 3500
Connection ~ 5450 3500
Wire Wire Line
	5450 3500 5200 3500
Wire Wire Line
	5050 2650 5450 2650
Text GLabel 4800 3500 0    50   Input ~ 0
Feed
Wire Wire Line
	4900 3500 4800 3500
Wire Wire Line
	5050 4600 5650 4600
Text GLabel 6900 3600 2    50   Output ~ 0
Comm
Text Notes 4800 3700 0    39   ~ 0
4mA pass through \nfor NOT gate
Text Notes 4800 2750 0    39   ~ 0
Battery supply 1
Text Notes 4800 4700 0    39   ~ 0
Battery supply 2
Text Notes 8650 2800 0    39   ~ 0
To microcontroller
Text GLabel 3200 3600 2    50   Output ~ 0
Comm
Text GLabel 3200 3500 2    50   Input ~ 0
Feed
Text GLabel 8500 2650 2    50   Output ~ 0
3.3V
Text GLabel 3200 3400 2    50   Output ~ 0
3.3V
$Comp
L Device:LED D2
U 1 1 63E2E54C
P 6900 3950
F 0 "D2" V 6847 4030 50  0000 L CNN
F 1 "RED" V 6938 4030 50  0000 L CNN
F 2 "150060RS75000:0603_DIODE" H 6900 3950 50  0001 C CNN
F 3 "~" H 6900 3950 50  0001 C CNN
	1    6900 3950
	0    1    1    0   
$EndComp
Wire Wire Line
	6900 2650 6900 3800
Wire Wire Line
	6000 2950 6000 3500
Text Notes 6050 4450 0    50   ~ 0
Low (Off)
Text Notes 6100 2850 0    50   ~ 0
Low (On)
Wire Wire Line
	5450 3500 6000 3500
$Comp
L Transistor_FET:IRLML6402 Q2
U 1 1 63E4B682
P 6000 4500
F 0 "Q2" H 5850 4450 50  0000 L CNN
F 1 "IRLML6402" H 5600 4350 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 6200 4425 50  0001 L CIN
F 3 "https://www.infineon.com/dgdl/irlml6402pbf.pdf?fileId=5546d462533600a401535668d5c2263c" H 6000 4500 50  0001 L CNN
	1    6000 4500
	0    1    1    0   
$EndComp
$Comp
L Diode:1N4148WS D1
U 1 1 63E4CBFB
P 6000 2350
F 0 "D1" H 6000 2133 50  0000 C CNN
F 1 "1N4148WS" H 6000 2224 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-323" H 6000 2175 50  0001 C CNN
F 3 "https://www.vishay.com/docs/85751/1n4148ws.pdf" H 6000 2350 50  0001 C CNN
	1    6000 2350
	-1   0    0    1   
$EndComp
$Comp
L Transistor_FET:IRLML6402 Q1
U 1 1 63EA78B4
P 6000 2750
F 0 "Q1" H 5900 2650 50  0000 L CNN
F 1 "IRLML6402" H 5650 2550 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 6200 2675 50  0001 L CIN
F 3 "https://www.infineon.com/dgdl/irlml6402pbf.pdf?fileId=5546d462533600a401535668d5c2263c" H 6000 2750 50  0001 L CNN
	1    6000 2750
	0    1    -1   0   
$EndComp
$Comp
L Device:R R3
U 1 1 63EAC35C
P 5650 4350
F 0 "R3" H 5500 4400 50  0000 L CNN
F 1 "10k" H 5450 4300 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 5580 4350 50  0001 C CNN
F 3 "~" H 5650 4350 50  0001 C CNN
	1    5650 4350
	1    0    0    -1  
$EndComp
Wire Wire Line
	5650 4150 5650 4200
Wire Wire Line
	5650 4150 6000 4150
Wire Wire Line
	6000 4150 6000 4300
Wire Wire Line
	5650 4500 5650 4600
Connection ~ 5650 4600
Wire Wire Line
	5650 4600 5800 4600
Wire Wire Line
	6200 2650 6250 2650
Wire Wire Line
	5450 2650 5750 2650
Wire Wire Line
	5850 2350 5750 2350
Wire Wire Line
	5750 2350 5750 2650
Connection ~ 5750 2650
Wire Wire Line
	5750 2650 5800 2650
Wire Wire Line
	6150 2350 6250 2350
Wire Wire Line
	6250 2350 6250 2650
Connection ~ 6250 2650
Connection ~ 6900 2650
$Comp
L Device:R R4
U 1 1 63EDF987
P 6900 4350
F 0 "R4" H 6750 4400 50  0000 L CNN
F 1 "200" H 6700 4300 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 6830 4350 50  0001 C CNN
F 3 "~" H 6900 4350 50  0001 C CNN
	1    6900 4350
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 4600 6900 4600
Wire Wire Line
	6900 4600 6900 4500
Wire Wire Line
	6900 4200 6900 4100
Wire Wire Line
	6000 3550 6000 3500
Connection ~ 6000 3500
Wire Wire Line
	6000 4100 6000 4150
Connection ~ 6000 4150
Wire Wire Line
	5750 3850 5900 3850
Wire Wire Line
	6100 3850 6250 3850
Text GLabel 3200 2750 2    50   Input ~ 0
VDDB
$Comp
L power:GND #PWR01
U 1 1 63ED71EE
P 3750 3400
F 0 "#PWR01" H 3750 3150 50  0001 C CNN
F 1 "GND" H 3755 3227 50  0000 C CNN
F 2 "" H 3750 3400 50  0001 C CNN
F 3 "" H 3750 3400 50  0001 C CNN
	1    3750 3400
	0    1    1    0   
$EndComp
Text GLabel 3200 3700 2    50   Output ~ 0
GND
Text GLabel 3750 3400 2    50   Input ~ 0
GND
Text GLabel 3200 2850 2    50   Input ~ 0
GND
Text GLabel 5050 2650 0    50   Input ~ 0
VDDA
Text GLabel 5050 4600 0    50   Input ~ 0
VDDB
$Comp
L 74xGxx:74AHC1G04 U1
U 1 1 63EFDACC
P 6000 3850
F 0 "U1" V 5929 3980 50  0000 L CNN
F 1 "74AHC1G04" V 5850 4000 50  0000 L CNN
F 2 "SN74AHC1G04DCKRE4:SOT65P210X110-5N" H 6000 3850 50  0001 C CNN
F 3 "https://www.ti.com/lit/ds/symlink/sn74ahc1g04.pdf?HQS=dis-mous-null-mousermode-dsf-pf-null-wwe&ts=1676632684720&ref_url=https%253A%252F%252Fwww.mouser.co.uk%252F" H 6000 3850 50  0001 C CNN
	1    6000 3850
	0    1    1    0   
$EndComp
Wire Wire Line
	6900 2650 7050 2650
$Comp
L power:PWR_FLAG #FLG04
U 1 1 63EC57F6
P 7050 2650
F 0 "#FLG04" H 7050 2725 50  0001 C CNN
F 1 "PWR_FLAG" H 7050 2823 50  0000 C CNN
F 2 "" H 7050 2650 50  0001 C CNN
F 3 "~" H 7050 2650 50  0001 C CNN
	1    7050 2650
	1    0    0    -1  
$EndComp
Wire Wire Line
	3500 4150 3500 4350
Text GLabel 3150 4150 1    50   Input ~ 0
VDDA
Text GLabel 3500 4150 1    50   Input ~ 0
VDDB
Wire Wire Line
	3150 4150 3150 4350
$Comp
L power:PWR_FLAG #FLG01
U 1 1 63ED06CB
P 3150 4350
F 0 "#FLG01" H 3150 4425 50  0001 C CNN
F 1 "PWR_FLAG" H 3150 4500 39  0000 C CNN
F 2 "" H 3150 4350 50  0001 C CNN
F 3 "~" H 3150 4350 50  0001 C CNN
	1    3150 4350
	-1   0    0    1   
$EndComp
$Comp
L power:PWR_FLAG #FLG02
U 1 1 63ECFF30
P 3500 4350
F 0 "#FLG02" H 3500 4425 50  0001 C CNN
F 1 "PWR_FLAG" H 3500 4500 39  0000 C CNN
F 2 "" H 3500 4350 50  0001 C CNN
F 3 "~" H 3500 4350 50  0001 C CNN
	1    3500 4350
	-1   0    0    1   
$EndComp
$Comp
L Connector:Conn_01x04_Male J2
U 1 1 63DBB650
P 3000 3500
F 0 "J2" H 3108 3781 50  0000 C CNN
F 1 "Conn_01x04_Male" H 3108 3690 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Horizontal" H 3000 3500 50  0001 C CNN
F 3 "~" H 3000 3500 50  0001 C CNN
	1    3000 3500
	1    0    0    -1  
$EndComp
Text GLabel 6250 3850 2    50   Input ~ 0
VDDA
Wire Wire Line
	6250 2650 6900 2650
$Comp
L power:GND #PWR02
U 1 1 63EB8E1A
P 5750 3850
F 0 "#PWR02" H 5750 3600 50  0001 C CNN
F 1 "GND" H 5755 3677 50  0000 C CNN
F 2 "" H 5750 3850 50  0001 C CNN
F 3 "" H 5750 3850 50  0001 C CNN
	1    5750 3850
	0    1    1    0   
$EndComp
Text GLabel 3900 4300 3    50   Input ~ 0
GND
$Comp
L power:PWR_FLAG #FLG03
U 1 1 63F3780A
P 3900 4050
F 0 "#FLG03" H 3900 4125 50  0001 C CNN
F 1 "PWR_FLAG" H 3900 4200 39  0000 C CNN
F 2 "" H 3900 4050 50  0001 C CNN
F 3 "~" H 3900 4050 50  0001 C CNN
	1    3900 4050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3900 4050 3900 4300
$Comp
L Connector:Conn_01x04_Male J1
U 1 1 63F5BCF6
P 3000 2850
F 0 "J1" H 3108 3131 50  0000 C CNN
F 1 "Conn_01x04_Male" H 3108 3040 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Horizontal" H 3000 2850 50  0001 C CNN
F 3 "~" H 3000 2850 50  0001 C CNN
	1    3000 2850
	1    0    0    -1  
$EndComp
Text GLabel 3200 2950 2    50   Input ~ 0
GND
Text GLabel 3200 3050 2    50   Input ~ 0
VDDA
Wire Wire Line
	7800 2650 8000 2650
$Comp
L Device:R R5
U 1 1 63F66DE7
P 8000 2900
F 0 "R5" H 7850 2950 50  0000 L CNN
F 1 "10k" H 7800 2850 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.20x1.40mm_HandSolder" V 7930 2900 50  0001 C CNN
F 3 "~" H 8000 2900 50  0001 C CNN
	1    8000 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	7500 2950 7500 3050
Wire Wire Line
	7500 3050 8000 3050
Wire Wire Line
	8000 2750 8000 2650
Connection ~ 8000 2650
Wire Wire Line
	8000 2650 8500 2650
Text GLabel 7500 3150 3    50   Input ~ 0
GND
Wire Wire Line
	7500 3150 7500 3050
Connection ~ 7500 3050
$Comp
L Regulator_Linear:LM1117-3.3 U2
U 1 1 63F658BE
P 7500 2650
F 0 "U2" H 7500 2892 50  0000 C CNN
F 1 "LM1117-3.3" H 7500 2801 50  0000 C CNN
F 2 "LM1117MP-3:VREG_LM1117MP-3.3_NOPB" H 7500 2650 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/lm1117.pdf" H 7500 2650 50  0001 C CNN
	1    7500 2650
	1    0    0    -1  
$EndComp
Connection ~ 7050 2650
Wire Wire Line
	7050 2650 7200 2650
$EndSCHEMATC
