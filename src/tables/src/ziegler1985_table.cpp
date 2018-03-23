/*******************************************************************************
 * Copyright (C) 2015 University of Oslo                                       *
 * Author: Vetle Wegner Ingeberg, v.w.ingeberg@fys.uio.no                      *
 *                                                                             *
 * --------------------------------------------------------------------------- *
 * This program is free software; you can redistribute it and/or modify it     *
 * under the terms of the GNU General Public License as published by the       *
 * Free Software Foundation; either version 3 of the license, or (at your      *
 * option) any later version.                                                  *
 *                                                                             *
 * This program is distributed in the hope that it will be useful, but         *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                  *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General   *
 * Public License for more details.                                            *
 *                                                                             *
 * You should have recived a copy of the GNU General Public License along with *
 * the program. If not, see <http://www.gnu.org/licenses/>.                    *
 *                                                                             *
 *******************************************************************************/


#include "ziegler1985_table.h"

#include <iostream>

int Get_minZ(){
	return 1;
}

int Get_maxZ(){
	return 92;
}

inline static bool in_table(const int &z) {
    if ( (z < Get_minZ()) || (z > Get_maxZ()) ){
        std::cerr << "Warning: Element Z=" <<  z << " is not included in the Ziegler table." << std::endl;
        return false;
    }
    return true;
}

double Get_lfctr(int z){
    if ( !in_table(z) ){
        return -1;
	}
	return p1[z - 1][7];
}

double Get_rho(int z){
    if ( !in_table(z) ){
        return -1;
    }
	return p1[z-1][4];
}

double Get_atrho(int z){
    if ( !in_table(z) ){
        return -1;
    }
	return p1[z-1][5]*1e22;
}

double Get_vfermi(int z){
    if ( !in_table(z) ){
        return -1;
    }
	return p1[z-1][6];
}

double Get_mm2(int z){
    if ( !in_table(z) ){
        return -1;
    }
    return p1[z-1][1];
}

bool Get_pcoef(int z, double *data){
    if ( !in_table(z) ){
        return false;
    }
	const double * pcoef = p2[z-1];
	for (int i = 0 ; i < 9 ; i++){
		data[i] = pcoef[i];
	}
    return true;
}

const double p1[][8] = {
        // mai: most abundant isotope
        // Z    Amai Mmai      Mnorm     dens     dens      vfermi    ion_screening_length
        //           amu       amu       g/cm3    1e22/cm3  Bohr v
        { 1,   1,   1.0078,   1.008,   0.071486,   4.271,  1.0309,    1.  },
        { 2,   4,   4.0026,   4.0026,  0.12588,    1.894,  0.15976,   1.  },
        { 3,   7,   7.016,    6.939,   0.52969,    4.597,  0.59782,   1.1 }, // != LISE Mn,dens
        { 4,   9,   9.012,    9.0122,  1.8024,    12.046,  1.0781,    1.06}, // != LISE dens
        { 5,  11,  11.009,   10.811,   2.3502,    13.093,  1.0486,    1.01},
        { 6,  12,  12,       12.011,   2.2662,    11.364,  1,         1.03}, // != LISE dens
        { 7,  14,  14.003,   14.007,   0.80963,    3.481,  1.058,     1.04}, // != LISE dens
        { 8,  16,  15.995,   15.999,   1.1429,     4.302,  0.93942,   0.99}, // != LISE dens
        { 9,  19,  18.998,   18.998,   1.1111,     3.522,  0.74562,   0.95},
        {10,  20,  19.992,   20.183,   1.2015,     3.585,  0.3424,    0.9 }, // != LISE dens
        {11,  23,  22.99,    22.989,   0.97,       2.541,  0.45259,   0.82},
        {12,  24,  23.985,   24.312,   1.7366,     4.302,  0.71074,   0.81}, // != LISE Mn
        {13,  27,  26.982,   26.981,   2.6981,     6.023,  0.90519,   0.83}, // != LISE Mn,dens
        {14,  28,  27.977,   28.086,   2.3212,     4.977,  0.97411,   0.88},
        {15,  31,  30.994,   30.973,   1.8219,     3.542,  0.97184,   1.  }, // != LISE Mn
        {16,  32,  31.97,    32.064,   2.0686,     3.885,  0.89852,   0.95}, // != LISE Mn
        {17,  35,  34.969,   35.453,   1.8956,     3.22,   0.70827,   0.97},
        {18,  40,  39.98,    39.948,   1.6504,     2.488,  0.39816,   0.99},
        {19,  39,  38.96,    39.102,   0.86318,    1.329,  0.36552,   0.98}, // != LISE Mn
        {20,  40,  39.96,    40.08,    1.54,       2.014,  0.62712,   0.97},
        {21,  45,  44.956,   44.956,   2.9971,     4.015,  0.81707,   0.98}, // != LISE dens
        {22,  48,  47.95,    47.9,     4.5189,     5.682,  0.9943,    0.97},
        {23,  51,  50.94,    50.942,   6.1008,     7.213,  1.1423,    0.96}, // != LISE dens
        {24,  52,  51.94,    51.996,   7.1917,     8.33,   1.2381,    0.93}, // != LISE dens
        {25,  55,  54.94,    54.938,   7.4341,     8.15,   1.1222,    0.91},
        {26,  56,  55.94,    55.847,   7.8658,     8.483,  0.92705,   0.9 },
        {27,  59,  58.93,    58.933,   8.796,      8.989,  1.0047,    0.88}, // != LISE dens
        {28,  58,  57.94,    58.71,    8.8955,     9.125,  1.2,       0.9 }, // != LISE Mn
        {29,  63,  62.93,    63.54,    8.9493,     8.483,  1.0661,    0.9 }, // != LISE Mn,dens
        {30,  64,  63.93,    65.37,    7.1054,     6.546,  0.97411,   0.9 }, // != LISE Mn,dens
        {31,  69,  68.93,    69.72,    5.9085,     5.104,  0.84912,   0.9 }, // != LISE dens
        {32,  74,  73.92,    72.59,    5.3375,     4.428,  0.95,      0.85}, // != LISE Mn,dens
        {33,  75,  74.92,    74.922,   5.7192,     4.597,  1.0903,    0.9 }, // != LISE dens
        {34,  80,  79.92,    78.96,    4.7855,     3.65,   1.0429,    0.9 }, // != LISE dens
        {35,  79,  78.92,    79.909,   3.3995,     2.562,  0.49715,   0.91}, // != LISE Mn,dens
        {36,  84,  83.92,    83.8,     2.6021,     1.87,   0.37755,   0.92},
        {37,  85,  84.91,    85.47,    1.529,      1.077,  0.35211,   0.9 }, // != LISE dens
        {38,  88,  87.91,    87.62,    2.6,        1.787,  0.57801,   0.9 },
        {39,  90,  89.91,    88.905,   4.4902,     3.041,  0.77773,   0.9 }, // != LISE dens
        {40,  90,  89.9,     91.22,    6.4695,     4.271,  1.0207,    0.9 }, // != LISE dens
        {41,  93,  92.91,    92.906,   8.6024,     5.576,  1.029,     0.9 }, // stop lese checking here ^^^
        {42,  96,  95.5,     95.94,   10.206,      6.407,  1.2542,    0.88},
        {43,   0,   0,       99,       0,          0,      1.122,     0.9 },
        {44, 102, 101.9,    101.07,   12.177,      7.256,  1.1241,    0.88},
        {45, 103, 102.9,    102.91,   12.399,      7.256,  1.0882,    0.88},
        {46, 106, 105.9,    106.4,    11.955,      6.767,  1.2709,    0.9 },
        {47, 107, 106.9,    107.87,   10.473,      5.847,  1.2542,    0.9 },
        {48, 114, 113.9,    112.4,     8.5802,     4.597,  0.90094,   0.88},
        {49, 115, 114.9,    114.82,    7.3134,     3.836,  0.74093,   0.9 },
        {50, 120, 119.9,    118.69,    7.2816,     3.695,  0.86054,   0.9 },
        {51, 121, 120.9,    121.75,    6.6168,     3.273,  0.93155,   0.9 },
        {52, 130, 130,      127.6,     6.2244,     2.938,  1.0047,    0.9 },
        {53, 127, 126.9,    126.9,     4.9373,     2.343,  0.55379,   0.96},
        {54, 130, 129.5,    131.3,     3.0589,     1.403,  0.43289,   1.2 },
        {55, 134, 134,      132.91,    1.8986,     0.86,   0.32636,   0.9 },
        {56, 138, 138,      137.34,    3.5215,     1.544,  0.5131,    0.88},
        {57, 139, 139,      138.91,    6.1738,     2.676,  0.695,     0.88},
        {58, 140, 140,      140.12,    6.6724,     2.868,  0.72591,   0.85},
        {59, 141, 141,      140.91,    6.7744,     2.895,  0.71202,   0.9 },
        {60, 142, 142,      144.24,    7.0019,     2.923,  0.67413,   0.9 },
        {61,   0,   0,      147,       0,          0,      0.71418,   0.92},
        {62, 152, 152,      150.35,    7.5553,     3.026,  0.71453,   0.95},
        {63, 153, 153,      151.96,    5.2581,     2.084,  0.5911,    0.99},
        {64, 158, 157.9,    157.25,    7.902,      3.026,  0.70263,   1.03},
        {65, 160, 159.9,    158.92,    8.2773,     3.136,  0.68049,   1.05},
        {66, 164, 164,      162.5,     8.5526,     3.17,   0.68203,   1.07},
        {67, 165, 165,      164.93,    8.8198,     3.22,   0.68121,   1.08},
        {68, 166, 166,      167.26,    9.0902,     3.273,  0.68532,   1.1 },
        {69, 169, 169,      168.93,    9.3331,     3.327,  0.68715,   1.08},
        {70, 174, 174,      173.04,    6.9774,     2.428,  0.61884,   1.08},
        {71, 175, 175,      174.97,    9.8298,     3.383,  0.71801,   1.08},
        {72, 180, 180,      178.49,   13.124,      4.428,  0.83048,   1.08},
        {73, 181, 181,      180.95,   16.601,      5.525,  1.1222,    1.09},
        {74, 184, 184,      183.85,   19.292,      6.32,   1.2381,    1.09},
        {75, 187, 187,      186.2,    21.04,       6.805,  1.045,     1.1 },
        {76, 192, 192,      190.2,    22.562,      7.144,  1.0733,    1.11},
        {77, 193, 193,      192.2,    22.506,      7.052,  1.0953,    1.12},
        {78, 195, 195,      195.09,   21.438,      6.618,  1.2381,    1.13},
        {79, 197, 197,      196.97,   19.311,      5.904,  1.2879,    1.14},
        {80, 202, 202,      200.59,   13.553,      4.069,  0.78654,   1.15},
        {81, 205, 205,      204.37,   11.882,      3.501,  0.66401,   1.17},
        {82, 208, 208,      207.19,   11.322,      3.291,  0.84912,   1.2 },
        {83, 209, 209,      208.98,    9.8113,     2.827,  0.88433,   1.18},
        {84, 210, 210,      210,       9.2511,     2.653,  0.80746,   1.17},
        {85, 210, 210,      210,       0,          0,      0.43357,   1.17},
        {86, 222, 222,      222,       0,          0,      0.41923,   1.16},
        {87, 223, 223,      223,       0,          0,      0.43638,   1.16},
        {88, 226, 226,      226,       5.0222,     1.338,  0.51464,   1.16},
        {89, 227, 227,      227,       0,          0,      0.73087,   1.16},
        {90, 232, 232,      232,      11.658,      3.026,  0.81065,   1.16},
        {91, 231, 231,      231,      15.4,        4.015,  1.9578,    1.16},
        {92, 238, 238.04,   238.04,   19.043,      4.818,  1.0257,    1.16}
    };

const double p2[][9] = {
       // Z ................proton stopping cross section coefficients................
        { 1, 0.0091827, 0.0053496, 0.69741, 0.48493, 316.07,  1.0143, 9329.3, 0.053989},
        { 2, 0.11393,   0.0051984, 1.0822,  0.39252, 1081,    1.0645, 4068.5, 0.017699},
        { 3, 0.85837,   0.0050147, 1.6044,  0.38844, 1337.3,  1.047,  2659.2, 0.01898},
        { 4, 0.8781,    0.0051049, 5.4232,  0.2032,  1200.6,  1.0211, 1401.8, 0.038529},
        { 5, 1.4608,    0.0048836, 2.338,   0.44249, 1801.3,  1.0352, 1784.1, 0.02024},
        { 6, 3.2579,    0.0049148, 2.7156,  0.36473, 2092.2,  1.0291, 2643.6, 0.018237},
        { 7, 0.59674,   0.0050837, 4.2073,  0.30612, 2394.2,  1.0255, 4892.1, 0.016006},
        { 8, 0.75253,   0.0050314, 4.0824,  0.30067, 2455.8,  1.0181, 5069.7, 0.017426},
        { 9, 1.226,     0.0051385, 3.2246,  0.32703, 2525,    1.0142, 7563.6, 0.019469},
        {10, 1.0332,    0.0051645, 3.004,   0.33889, 2338.6, 0.99997, 6991.2, 0.021799},
                             
        {11, 6.0972,    0.0044292, 3.1929,  0.45763, 1363.3,  0.95182, 2380.6, 0.081835},
        {12, 14.013,    0.0043646, 2.2641,  0.36326, 2187.4,  0.99098, 6264.8, 0.0462},
        {13, 0.039001,  0.0045415, 5.5463,  0.39562, 1589.2,  0.95316, 816.16, 0.047484},
        {14, 2.072,     0.0044516, 3.5585,  0.53933, 1515.2,  0.93161, 1790.3, 0.035189},
        {15, 17.575,    0.0038346, 0.078694,1.2388,  2806,    0.97284, 1037.6, 0.012879},
        {16, 16.126,    0.0038315, 0.054164,1.3104,  2813.3,  0.96587, 1251.4, 0.011847},
        {17, 3.217,     0.0044579, 3.6696,  0.5091,  2734.6,  0.96253, 2187.5, 0.016907},
        {18, 2.0379,    0.0044775, 3.0743,  0.54773, 3505,    0.96575, 1714,   0.011701},
        {19, 0.74171,   0.0043051, 1.1515,  0.95083, 917.21,  0.8782,  389.93, 0.18926},
        {20, 9.1316,    0.0043809, 5.4611,  0.31327, 3891.8,  0.97933, 6267.9, 0.015196},
                             
        {21, 7.2247,    0.0043718, 6.1017,  0.37511, 2829.2,  0.95218, 6376.1, 0.020398},
        {22, 0.147,     0.0048456, 6.3485,  0.41057, 2164.1,  0.94028, 5292.6, 0.050263},
        {23, 5.0611,    0.0039867, 2.6174,  0.57957, 2218.9,  0.92361, 6323,   0.025669},
        {24, 0.53267,   0.0042968, 0.39005, 1.2725,  1872.7,  0.90776, 64.166, 0.030107},
        {25, 0.47697,   0.0043038, 0.31452, 1.3289,  1920.5,  0.90649, 45.576, 0.027469},
        {26, 0.027426,  0.0035443, 0.031563,2.1755,  1919.5,  0.90099, 23.902, 0.025363},
        {27, 0.16383,   0.0043042, 0.073454,1.8592,  1918.4,  0.89678, 27.61,  0.023184},
        {28, 4.2562,    0.0043737, 1.5606,  0.72067, 1546.8,  0.87958, 302.02, 0.040944},
        {29, 2.3508,    0.0043237, 2.882,   0.50113, 1837.7,  0.89992, 2377,   0.04965},
        {30, 3.1095,    0.0038455, 0.11477, 1.5037,  2184.7,  0.89309, 67.306, 0.016588},
                             
        {31, 15.322,     0.0040306, 0.65391, 0.67668, 3001.7,  0.92484, 3344.2, 0.016366},
        {32, 3.6932,    0.0044813, 8.608,   0.27638, 2982.7,  0.9276,  3166.6, 0.030874},
        {33, 7.1373,    0.0043134, 9.4247,  0.27937, 2725.8,  0.91597, 3166.1, 0.025008},
        {34, 4.8979,    0.0042937, 3.7793,  0.50004, 2824.5,  0.91028, 1282.4, 0.017061},
        {35, 1.3683,    0.0043024, 2.5679,  0.60822, 6907.8,  0.9817,  628.01, 0.0068055},
        {36, 1.8301,    0.0042983, 2.9057,  0.6038,  4744.6,  0.94722, 936.64, 0.0092242},
        {37, 0.42056,   0.0041169, 0.01695, 2.3616,  2252.7,  0.89192, 39.752, 0.027757},
        {38, 30.78,     0.0037736, 0.55813, 0.76816, 7113.2,  0.97697, 1604.4, 0.0065268},
        {39, 11.576,    0.0042119, 7.0244,  0.37764, 4713.5,  0.94264, 2493.2, 0.01127},
        {40, 6.2406,    0.0041916, 5.2701,  0.49453, 4234.6,  0.93232, 2063.9, 0.011844},
                             
        {41, 0.33073,   0.0041243, 1.7246,  1.1062,  1930.2,  0.86907, 27.416, 0.038208},
        {42, 0.017747,  0.0041715, 0.14586, 1.7305,  1803.6,  0.86315, 29.669, 0.032123},
        {43, 3.7229,    0.0041768, 4.6286,  0.56769, 1678,    0.86202, 3094,   0.06244},
        {44, 0.13998,   0.0041329, 0.25573, 1.4241,  1919.3,  0.86326, 72.797, 0.032235},
        {45, 0.2859,    0.0041386, 0.31301, 1.3424,  1954.8,  0.86175, 115.18, 0.029342},
        {46, 0.76002,   0.0042179, 3.386,   0.76285, 1867.4,  0.85805, 69.994, 0.036448},
        {47, 6.3957,    0.0041935, 5.4689,  0.41378, 1712.6,  0.85397, 18493,  0.056471},
        {48, 3.4717,    0.0041344, 3.2337,  0.63788, 1116.4,  0.81959, 4766,   0.1179},
        {49, 2.5265,    0.0042282, 4.532,   0.53562, 1030.8,  0.81652, 16252,  0.19722},
        {50, 7.3683,    0.0041007, 4.6791,  0.51428, 1160,    0.82454, 17956,  0.13316},

        {51, 7.7197,    0.004388,  3.242,   0.68434, 1428.1,  0.83389, 1786.7, 0.066512},
        {52, 16.78,     0.0041918, 9.3198,  0.29569, 3370.9,  0.90298, 7431.7, 0.02616},
        {53, 4.2132,    0.0042098, 4.6753,  0.57945, 3503.9,  0.89261, 1468.9, 0.014359},
        {54, 4.0818,    0.004214,  4.4425,  0.58393, 3945.3,  0.90281, 1340.5, 0.013414},
        {55, 0.18517,   0.0036215,0.00058788,3.5315, 2931.3,  0.88936,  26.18, 0.026393},
        {56, 4.8248,    0.0041458, 6.0934,  0.57026, 2300.1,  0.86359, 2980.7, 0.038679},
        {57, 0.49857,   0.0041054, 1.9775,  0.95877, 786.55,  0.78509,  806.6, 0.40882},
        {58, 3.2754,    0.0042177, 5.768,   0.54054, 6631.3,  0.94282, 744.07, 0.0083026},
        {59, 2.9978,    0.0040901, 4.5299,  0.62025, 2161.2,  0.85669, 1268.6, 0.043031},
        {60, 2.8701,    0.004096,  4.2568,  0.6138,  2130.4,  0.85235, 1704.1, 0.039385},

        {61, 10.853,    0.0041149, 5.8906,  0.46834, 2857.2,  0.87550, 3654.2, 0.029955},
        {62, 3.6407,    0.0041782, 4.8742,  0.57861, 1267.7,  0.82211, 3508.2, 0.24174},
        {63, 17.645,    0.0040992, 6.5855,  0.32734, 3931.3,  0.90754, 5156.7, 0.036278},
        {64, 7.5309,    0.0040814, 4.9389,  0.50679, 2519.7,  0.85819, 3314.6, 0.030514},
        {65, 5.4742,    0.0040829, 4.897,   0.51113, 2340.1,  0.85296, 2342.7, 0.035662},
        {66, 4.2661,    0.0040667, 4.5032,  0.55257, 2076.4,  0.84151, 1666.6, 0.040801},
        {67, 6.8313,    0.0040486, 4.3987,  0.51675, 2003,    0.83437, 1410.4, 0.03478},
        {68, 1.2707,    0.0040553, 4.6295,  0.57428, 1626.3,  0.81858, 995.68, 0.055319},
        {69, 5.7561,    0.0040491, 4.357,   0.52496, 2207.3,  0.83796, 1579.5, 0.027165},
        {70, 14.127,    0.0040596, 5.8304,  0.37755, 3645.9,  0.87823, 3411.8, 0.016392},

        {71, 6.6948,    0.0040603, 4.9361,  0.47961, 2719,    0.85249, 1885.8, 0.019713},
        {72, 3.0619,    0.0040511, 3.5803,  0.59082, 2346.1,  0.83713, 1222,   0.020072},
        {73, 10.811,    0.0033008, 1.3767,  0.76512, 2003.7,  0.82269, 1110.6, 0.024958},
        {74, 2.7101,    0.0040961, 1.2289,  0.98598, 1232.4,  0.79066, 155.42, 0.047294},
        {75, 0.52345,   0.0040244, 1.4038,  0.8551,  1461.4,  0.79677, 503.34, 0.036789},
        {76, 0.4616,    0.0040203, 1.3014,  0.87043, 1473.5,  0.79687, 443.09, 0.036301},
        {77, 0.97814,   0.0040374, 2.0127,  0.7225,  1890.8,  0.81747, 930.7,  0.02769},
        {78, 3.2086,    0.0040510, 3.6658,  0.53618, 3091.2,  0.85602, 1508.1, 0.015401},
        {79, 2.0035,    0.0040431, 7.4882,  0.3561,  4464.3,  0.88836, 3966.5, 0.012839},
        {80, 15.43,     0.0039432, 1.1237,  0.70703, 4595.7,  0.88437, 1576.5, 0.0088534},

        {81, 3.1512,    0.0040524, 4.0996,  0.5425,  3246.3,  0.85772, 1691.8, 0.015058},
        {82, 7.1896,    0.0040588, 8.6927,  0.35842, 4760.6,  0.88833, 2888.3, 0.011029},
        {83, 9.3209,    0.0040540, 11.543,  0.32027, 4866.2,  0.89124, 3213.4, 0.011935},
        {84, 29.242,    0.0036195, 0.16864, 1.1226,  5688,    0.89812, 1033.3, 0.0071303},
        {85, 1.8522,    0.0039973, 3.1556,  0.65096, 3755,    0.86383, 1602,   0.012042},
        {86, 3.222,     0.0040041, 5.9024,  0.52678, 4040.2,  0.86804, 1658.4, 0.011747},
        {87, 9.3412,    0.0039661, 7.921,   0.42977, 5180.9,  0.88773, 2173.2, 0.0092007},
        {88, 36.183,    0.0036003, 0.58341, 0.86747, 6990.2,  0.91082, 1417.1, 0.0062187},
        {89, 5.9284,    0.0039695, 6.4082,  0.52122, 4619.5,  0.88083, 2323.5, 0.011627},
        {90, 5.2454,    0.0039744, 6.7969,  0.48542, 4586.3,  0.87794, 2481.5, 0.011282},

        {91, 33.702,    0.0036901, 0.47257, 0.89235, 5295.7,  0.8893,  2053.3, 0.0091908},
        {92, 2.7589,    0.0039806, 3.2092,  0.66122, 2505.4,  0.82863, 2065.1, 0.022816}
    };
