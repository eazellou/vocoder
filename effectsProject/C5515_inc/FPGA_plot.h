/*
 * FPGA_plot.h
 *
 *  Created on: Feb 8, 2012
 *      Author: Kurt Metzger
 */

#ifndef FPGA_PLOT_H_
#define FPGA_PLOT_H_

void display_initialize(void);
void plot_dB_init(int, float, float, float, float, float, float, float, float);
void print_string(float x, float y, int step, int rotation, int color, char *cp);
void GoToData(int, int, float, float);
void DrawToData(int, int, float, float);
void display_close(void);
void EndPlot(void);

int Check_Switch(void);

#endif /* FPGA_PLOT_H_ */
