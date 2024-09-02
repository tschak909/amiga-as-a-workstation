/** 
 * @brief Heatmap in lores display
 */

#include "heatmap.h"

#include <stdio.h>

#include "exec/types.h"
#include "exec/io.h"
#include "exec/memory.h"

#include "graphics/gfx.h"
#include "hardware/dmabits.h"
#include "hardware/custom.h"
#include "hardware/blit.h"
#include "graphics/gfxmacros.h"
#include "graphics/copper.h"
#include "graphics/view.h"
#include "graphics/gels.h"
#include "graphics/regions.h"
#include "graphics/clip.h"
#include "exec/exec.h"
#include "graphics/text.h"
#include "graphics/gfxbase.h"

#include "libraries/dos.h"
#include "graphics/text.h"
#include "intuition/intuition.h"

#define XW 1
#define YW 1

extern struct Window *OpenWindow();
extern struct Screen *OpenScreen();

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;

struct Heatmap h;

struct NewScreen nsHeatmap = {
    0, 72,                       /* start position         */
    320, 128, 3,                /* width, height, depth   */
    0, 1,                       /* detail pen, block pen  */
    NULL,                      /* viewing mode           */
    CUSTOMSCREEN,               /* screen type            */
    NULL,                       /* use default font       */
    "Heatmap",                  /* screen title           */
    NULL                        /* ptr to addit'l gadgets */
};

struct NewScreen nsDescription = {
    0, 0,
    640, 200, 1,
    0, 1,
    HIRES,
    CUSTOMSCREEN,
    NULL, 
    "Description",
    NULL
};

struct NewWindow nwHeatmap = {
      0, 0,                     /* start position         */
      128, 128,                 /* width, height          */
      0, 1,                     /* detail pen, block pen  */
      CLOSEWINDOW,             /* IDCMP flags            */
      WINDOWCLOSE|GIMMEZEROZERO,            
                                /* window flags           */
      NULL,                     /* ptr to first user gadget */
      NULL,                     /* ptr to user checkmark  */ 
      "Heatmap",                /* window title           */ 
      NULL,                     /* pointer to screen      */
      NULL,                     /* pointer to superbitmap */
      0,0,128,128,              /* sizing limits min/max  */
      CUSTOMSCREEN              /* type of screen         */ 
      };

struct NewWindow nwLegend = {
	220, 12,
	100, 114,
        0, 7,
        NULL,
        BACKDROP|ACTIVATE,
	NULL,
	NULL,
	"Temp Scale",
	NULL,
	NULL,
	0,0,192,127,
	CUSTOMSCREEN
};

struct NewWindow nwDescription = {
	0, 0,
	640, 69,
	0, 1,
	NULL,
	BACKDROP,
	NULL,
	NULL,
	"Description Window",
	NULL,
	NULL,
	0,0,640,128,
	CUSTOMSCREEN
};

struct Window *wHeatmap;
struct Window *wLegend;

struct Window *wDescription;

struct Screen *sHeatmap;
struct Screen *sDescription;

init()
{
    GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0);

    if (!GfxBase)
    {
    	printf("Could not open graphics library.\n");
        return 0;
    }

    IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",0);

    if (!IntuitionBase)
    {
    	printf("Could not open intuition library.\n");
        return 0;
    }

    sDescription = (struct Screen *)OpenScreen(&nsDescription);

    if (!sDescription)
    {
	printf("Could not open description screen. \n");
        return 0;
    }

    sHeatmap = (struct Screen *)OpenScreen(&nsHeatmap);

    if (!sHeatmap)
    {
	printf("Could not open Heatmap screen.\n");
        return 0;
    }

    nwHeatmap.Screen = sHeatmap;
    nwLegend.Screen = sHeatmap;
    nwDescription.Screen = sDescription;

    wLegend = (struct Window *)OpenWindow(&nwLegend);

    if (!wLegend)
    {
	printf("Could not open legend window\n");
	return 0;
    }

    wHeatmap = (struct Window *)OpenWindow(&nwHeatmap);

    if (!wHeatmap)
    {
    	printf("Could not open Heatmap window.\n");
        return 0;
    }

    wDescription = (struct Window *)OpenWindow(&nwDescription);

    if (!wDescription)
    {
	printf("Could not open description window.\n");
	return 0;
    }

    /* Set Screen palette */
    
    SetRGB4(&sHeatmap->ViewPort, 0, 0x00, 0x00, 0x00);
    SetRGB4(&sHeatmap->ViewPort, 1, 0x00, 0x00, 0x0C);
    SetRGB4(&sHeatmap->ViewPort, 2, 0x00, 0x08, 0x00);
    SetRGB4(&sHeatmap->ViewPort, 3, 0x00, 0x0C, 0x00);
    SetRGB4(&sHeatmap->ViewPort, 4, 0x00, 0x0F, 0x00);
    SetRGB4(&sHeatmap->ViewPort, 5, 0x08, 0x00, 0x00);
    SetRGB4(&sHeatmap->ViewPort, 6, 0x0C, 0x00, 0x00);
    SetRGB4(&sHeatmap->ViewPort, 7, 0x0F, 0x0F, 0x0F);

    /* Go ahead and set loading message */
    Move(wHeatmap->RPort,0,56);
    SetAPen(wHeatmap->RPort,7);
    Text(wHeatmap->RPort,"Loading...",10);

    return 1;
}

done()
{
     if (wHeatmap)
         CloseWindow(wHeatmap);

     if (wDescription)
	 CloseWindow(wDescription);

     if (wLegend)
	CloseWindow(wLegend);

     if (sDescription)
         CloseScreen(sDescription);

     if (sHeatmap)
         CloseScreen(sHeatmap);

     if (IntuitionBase)
         CloseLibrary(IntuitionBase);
     
     if (GfxBase)
         CloseLibrary(GfxBase);
}

/**
 * @brief update legend
 */
legend()
{
	int i;
	char *t[8];

	t[0] = "< 20 °C";
	t[1] = "  25 °C";
	t[2] = "  30 °C";
	t[3] = "  35 °C";
	t[4] = "  40 °C";
	t[5] = "  45 °C";
	t[6] = "  50 °C";
	t[7] = "  55 °C";

	/* Draw nice filled rects for legend chips */
	for (i=0;i<8;i++)
	{
	    SetAPen(wLegend->RPort,i);
	    RectFill(wLegend->RPort,8,i*8+16,16,i*8+16+8);
	    Move(wLegend->RPort,24,i*8+16+8);
	    SetAPen(wLegend->RPort,7);
	    Text(wLegend->RPort,t[i],7);
	}
}

/**
 * @brief Description
 */
description()
{
	SetAPen(wDescription->RPort,1);

	Move(wDescription->RPort,12,12+12);
	Text(wDescription->RPort, h.name, strlen(h.name));

	Move(wDescription->RPort,12,24+12);
        Text(wDescription->RPort, h.ts, strlen(h.ts));

	Move(wDescription->RPort,12,36+12);
        Text(wDescription->RPort, h.desc, strlen(h.desc));
}

/**
 * @brief display heatmap
 */
heatmap()
{
    short o=0;
    short x=0, y=0;
    short xo=0, yo=0;

    for (y=0; y<128; y++)
    {
	for (x=0; x<128; x++)
    	{
            SetAPen(wHeatmap->RPort, h.data[o++]);
	    WritePixel(wHeatmap->RPort,xo,yo);
            xo ++;
    	}

	xo = 0;
        yo++;
     }
}

acquire()
{
	FILE *fp = fopen("h.dat","r");

	printf("Opening h.dat...\n");

	if (!fp)
	{
		printf("Could not open h.dat\n");
		return 0;
	}

	fread(&h,16624,1,fp);

	fclose(fp);
}

main()
{
    if (!init())
    {
        done();
        return 1;
    }

    acquire();
    legend();
    heatmap();
    description();

    while (Wait(1 << wHeatmap->UserPort->mp_SigBit))
    {
        struct IntuiMessage *message;
        
        message = GetMsg(wHeatmap->UserPort);

        ReplyMsg(message);

        if (message->Class == CLOSEWINDOW)
            break;    
    }

    done();
}

