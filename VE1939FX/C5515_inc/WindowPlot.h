/*
 * WindowPlot.h
 *
 *  Created on: May 20, 2015
 *      Author: Kurt Metzger
 */

#ifndef CONSOLEPLOT_H_
#define CONSOLEPLOT_H_

#define PLT_GOTO  			(0x8000|(0<<11))
#define PLT_DRAWTO 			(0x8000|(1<<11))
#define PLT_CHARPROPERTIES	(0x8000|(2<<11))
#define PLT_DRAWCHAR 		(0x8000|(3<<11))
#define PLT_CONSOLECHAROUT 	(0x8000|(4<<11))

#define PLT_DOSOMETHING  	(0x8000|(5<<11))
#define PLT_CONSOLESTRINGIN (PLT_DOSOMETHING|0)
#define PLT_STARTPLOT		(PLT_DOSOMETHING|1)
#define PLT_ENDPLOT	 		(PLT_DOSOMETHING|2)
#define PLT_UPDATEPLOT 		(PLT_DOSOMETHING|3)
//#define PLT_SWITCHBUFFER 	(PLT_DOSOMETHING|4)
#define PLT_MARK_SET		(PLT_DOSOMETHING|5)
#define PLT_MARK_CLEAR		(PLT_DOSOMETHING|6)
//#define PLT_FLUSH			(PLT_DOSOMETHING|7)

#define PLT_SELECTCOLOR	(0x8000|(6<<11))
#define 	PLT_BLACK	(0x00)
#define 	PLT_RED		(0x01)
#define 	PLT_GREEN	(0x02)
#define		PLT_BLUE	(0x03)
#define		PLT_YELLOW	(0x04)
#define		PLT_CYAN	(0x05)
#define		PLT_MAGENTA	(0x06)
#define		PLT_WHITE   (0x07)

// character properties:
//
//    ((step&0x000F)<<5)|((rotation&0x0003)<<3)|(color&0x0007))
//
#endif /* CONSOLEPLOT_H_ */
