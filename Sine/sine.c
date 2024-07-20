/**
 * @brief Sine Wave
 */

#include <lattice/math.h>

#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <exec/exec.h>

#include <graphics/gfx.h>
#include <hardware/dmabits.h>
#include <hardware/custom.h>
#include <hardware/blit.h>
#include <graphics/gfxmacros.h>
#include <graphics/copper.h>
#include <graphics/view.h>
#include <graphics/gels.h>
#include <graphics/regions.h>
#include <graphics/clip.h>
#include <graphics/text.h>
#include <graphics/gfxbase.h>

#include <libraries/dos.h>
#include <graphics/text.h>
#include <intuition/intuition.h>

extern struct Window *OpenWindow();
extern struct Screen *OpenScreen();
extern struct MsgPort *CreatePort();
extern struct IOStdReq *CreateStdIO();

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;

struct NewWindow nwSine =
{
   0, 0,                 /* Start Position */
   480, 200,               /* Width, Height */
   0, 1,                   /* Detail Pen, Block Pen */
   /* IDCMP Events to pass to our event loop: */
   GADGETUP | GADGETDOWN | MOUSEBUTTONS | MENUPICK | CLOSEWINDOW,
   /* Window Flags (wanted gadgets, etc.) */
   GIMMEZEROZERO | WINDOWDRAG | WINDOWDEPTH | WINDOWCLOSE |
   SMART_REFRESH | ACTIVATE,
   NULL,                  /* Pointer to first user gadget */
   NULL,                  /* Pointer to user checkmark */
   "f(x)=sin()",          /* Window Title */
   NULL,                  /* Pointer to Screen */
   NULL,                  /* Pointer to Superbitmap */
   50,40,480,200,         /* Sizing min and max */
   WBENCHSCREEN           /* We can change this to CUSTOMSCREEN */
};

struct Window *wSine;

/**
 * @brief Set up Intuition and graphics.library
 * @return 0 on failure, 1 on success.
 */
init()
{
   GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0);

   if (!GfxBase)
   {
       printf("Could not open graphics.library\n");
       return 0;
   }

   IntuitionBase = 
       (struct IntuitionBase *)OpenLibrary("intuition.library",0);

   if (!IntuitionBase)
   {
       printf("Could not open intuition.library\n");
       return 0;
   }

   wSine = (struct Window *)OpenWindow(&nwSine);

   if (!wSine)
   {
	printf("Could not open window.\n");
        return 0;
   }
   
}

/**
 * @brief display grid
 */
grid()
{
    /* Set to block pen */
    SetAPen(wSine->RPort, 1);

    /* Set solid pattern */
    SetDrPt(wSine->RPort,0xFFFF);

    /* Draw Zero crossing */
    Move(wSine->RPort,0,90);
    Draw(wSine->RPort,480,90);

    /* Set Dotted pattern */
    SetDrPt(wSine->RPort,0x8888);
    
    /* Draw amplitude markers */
    Move(wSine->RPort,0,6);
    Draw(wSine->RPort,480,6);
    Move(wSine->RPort,0,24);
    Draw(wSine->RPort,480,24);
    Move(wSine->RPort,0,48);
    Draw(wSine->RPort,480,48);
    Move(wSine->RPort,0,72);
    Draw(wSine->RPort,480,72);
    Move(wSine->RPort,0,184-6);
    Draw(wSine->RPort,480,184-6);
    Move(wSine->RPort,0,184-24);
    Draw(wSine->RPort,480,184-24);
    Move(wSine->RPort,0,184-48);
    Draw(wSine->RPort,480,184-48);
    Move(wSine->RPort,0,184-72);
    Draw(wSine->RPort,480,184-72);

    /* Put legends */
    Move(wSine->RPort,0,8);
    Text(wSine->RPort,"72",2);
    Move(wSine->RPort,0,24);
    Text(wSine->RPort,"48",2);
    Move(wSine->RPort,0,48);
    Text(wSine->RPort,"24",2);
    Move(wSine->RPort,0,72);
    Text(wSine->RPort," 6",2);

    Move(wSine->RPort,0,184-8);
    Text(wSine->RPort,"72",2);
    Move(wSine->RPort,0,184-24);
    Text(wSine->RPort,"48",2);
    Move(wSine->RPort,0,184-48);
    Text(wSine->RPort,"24",2);
    Move(wSine->RPort,0,184-72);
    Text(wSine->RPort," 6",2);
}

/**
 * @brief display sine wave
 */
sine()
{
    float x, y;
    int angle=0;

    /* Set the primary pen */
    SetAPen(wSine->RPort, 3);

    /* Set solid pattern */
    SetDrPt(wSine->RPort, 0xFFFF);

    Move(wSine->RPort,0,90);

    for (x=0; x<480; x++)
    {
	y = 90*sin(angle*PI/90)+90;
        Draw(wSine->RPort,(int)x,(int)y);
        angle++;
    }

}

done()
{
	CloseWindow(wSine);
        CloseLibrary(IntuitionBase);
	CloseLibrary(GfxBase);
}

/**
 * @brief main
 */
main()
{
    if (!init())
        return 1;

    grid();

    sine();

    while (Wait(1 << wSine->UserPort->mp_SigBit))
    {
        struct IntuiMessage *message;
        
        message = GetMsg(wSine->UserPort);

        ReplyMsg(message);

        if (message->Class == CLOSEWINDOW)
            break;
    }
    
    done();

    return 0;
}
