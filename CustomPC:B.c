/*	File:		CustomPC/B.c	Contains:	Sample showing how to implement custom PicComments and QuickDraw bottlenecks. 				See the ReadMe for code details.	Written by: 		Copyright:	Copyright � 1991-1999 by Apple Computer, Inc., All Rights Reserved.				You may incorporate this Apple sample source code into your program(s) without				restriction. This Apple sample source code has been provided "AS IS" and the				responsibility for its operation is yours. You are not permitted to redistribute				this Apple sample source code as "Apple sample source code" after having made				changes. If you're going to re-distribute the source, we require that you make				it clear in the source that the code was descended from Apple sample source				code, but that you've made changes.	Change History (most recent first):				08/2000			JM		Carbonized, non-Carbon code is commented out										for demonstration purposes.				7/9/1999		KG		Updated for Metrowerks Codewarror Pro 2.1				03/97	v. 2.0	ik		Rewritten as 'Custom PicComments/Bottlenecks'				10/91	v. 1.0	dh		Shipped as 'DTS Groupies' sample.				*/#include "CarbonPrefix.h"#include <Dialogs.h>#include <Fonts.h>#include <Menus.h>#include <Devices.h>#include <Resources.h>/*------ constants --------------------------------------------------------------------------*/#define kCreatorType	'EGAD'		/* Our creator type.				*/#define rMenuBar		128			/* The menubar resource ID.			*/#define mApple			128			/* Apple menu ID.					*/#define iAbout			1			/* "About�" menu item index.		*/#define mFile			129			/* File menu ID.					*/#define iQuit			1			/* Quit menu item index.			*/#define kMaxPICTs		50			/* Max. no. of pictures we handle.	*/#define kCustomComment	100			/* Custom PicComment indicator.		*/#define kSubPICTComment	200			/* Our (sub-picture) sub-PicComment.									   This comment indicates that we've									   stored a picture inside of a									   picture.  We use it to extract									   the individual PICTs.			*//*------ types --------------------------------------------------------------------------*/typedef struct TPICTRec {	int			numPICTs;			/* The number of sub-pictures, 		*/	PicHandle	picture[kMaxPICTs];	/* and their PicHandles,			*/	Rect		curPos[kMaxPICTs];	/* and their last drawn positions.	*/} TPICTRec;/*------ globals --------------------------------------------------------------------------*/static TPICTRec		gPICTRec;		/* Our global picture record.		*/static Rect			gPictsBounds;	/* The bounds used by our pictures.	*/static Boolean		gQuitting;		/* "Quitting?" flag.				*/static WindowPtr	gTheWindow;		/* Our window's pointer.			*//*------ prototypes ------------------------------------------------------------------------*//*extern void			CompositePictures(void);//extern pascal void	CustomPicProc(int kind, int dataSize, Handle dataHandle);extern pascal void  CustomPicProc(short kind, short dataSize, Handle dataHandle)extern void			DisassemblePictures(void);extern void			DoMenuCommand(long menuResult);extern void			EventLoop(void);extern void			MoveThePicts(Rect *wBounds);extern void			MakeThePicts(void);extern void			ShowThePicts(void);*/void			CompositePictures(void);pascal void     CustomPicProc(short kind, short dataSize, Handle dataHandle);void			DisassemblePictures(void);void			DoMenuCommand(long menuResult);void			EventLoop(void);void			MoveThePicts(Rect *wBounds);void			MakeThePicts(void);void			ShowThePicts(void);/*------ CompositePictures ----------------------------------------------------------------*///	CompositePictures groups all of the pictures in the global picture record//	into one "composite" picture.  It removes all of the old pictures and//	stores the new one./*----------------------------------------------------------------------------------------*/void CompositePictures(){	PicHandle	aPICT, groupPICT;	RgnHandle	oldClip;	int			idx;	long		dataSize;	long		ownerApp;	short		localPicComment;/*	Save the old clipping region, and set a valid one so our grouped	picture develops ok.												*/	oldClip = NewRgn();	GetClip(oldClip);	ClipRect(&gPictsBounds);	groupPICT = OpenPicture(&gPictsBounds);/*	Create a picture to contain all the other ones, then draw those into	it, separated by our PicComments.  Kill the individual pictures as	we go.  Finally, close the composite picture.						*/	ownerApp = kCreatorType;	localPicComment = kSubPICTComment;	for(idx = 0; idx < gPICTRec.numPICTs; idx++)	{		aPICT = gPICTRec.picture[idx];/*	We don't just use a single custom PicComment since another app may	use the same comment and conflicts could result.  (Not in this app,	but in the real world.)  We add six bytes to the handle and store the	creator type of the app that made the picture followed by 2 bytes	for a local PicComment kind within the app.  If we used more than	one PicComment in this app, this extra information would be	necessary.															*/		dataSize = GetHandleSize((Handle) aPICT) +6;		SetHandleSize((Handle) aPICT, dataSize);		BlockMove((Ptr) *aPICT, (Ptr) *aPICT +6, dataSize -6);		BlockMove(&ownerApp, (Ptr) *aPICT, 4);		BlockMove(&localPicComment, (Ptr) *aPICT +4, 2);		PicComment(kCustomComment, dataSize, (Handle) aPICT);/*	Fix the original PicHandle so that we can draw our picture for apps	that don't know about our custom comments.							*/		BlockMove((Ptr) *aPICT +6, (Ptr) *aPICT, dataSize -6);		SetHandleSize((Handle) aPICT, dataSize -6);		DrawPicture(aPICT, &(*aPICT)->picFrame);		KillPicture(aPICT);		gPICTRec.picture[idx] = NULL;	}	ClosePicture();/*	Restore the original clipping region and update our global picture	record so that we have one consolidated picture, in the first slot.	We set it's current position to (0, 0, 0, 0) so that we don't waste	time erasing anything on the first draw.							*/	SetClip(oldClip);	DisposeRgn(oldClip);	gPICTRec.numPICTs = 1;	gPICTRec.picture[0] = groupPICT;	SetRect(&gPICTRec.curPos[0], 0, 0, 0, 0);}/*------ CustomPicProc ----------------------------------------------------------------*///		CustomPicProc is our replacement for the port's StdCommentProc.// 		in the global picture record/*----------------------------------------------------------------------------------------*///pascal void CustomPicProc(int kind, int dataSize, Handle dataHandle)pascal void CustomPicProc(short kind, short dataSize, Handle dataHandle){	int			nextNum;	long		ownerApp;	short		localPicComment;	Handle		theHandle;/*	If this is a custom PicComment, see if it's ours.  In this app,	we know it always will be, but when you import other pictures	you can't be so sure.												*/	if (kind == kCustomComment && (gPICTRec.numPICTs < kMaxPICTs))	{		if (dataSize < 6) return;						/* Not ours?	*/				BlockMove((Ptr) *dataHandle, &ownerApp, 4);		BlockMove((Ptr) *dataHandle +4, &localPicComment, 2);		if ((ownerApp != kCreatorType) ||				/* Not ours?	*/		    (localPicComment != kSubPICTComment)) return;/*	This is indeed our picture comment.  Create a handle for the data we	found, store it in our global picture record and bump the number of	pictures we have.  The reason that we clear the picture's curPos	rect is so that we won't waste time erasing anything the first time	we enter MoveTheGroupies.											*/		nextNum = gPICTRec.numPICTs;		gPICTRec.picture[nextNum] = (PicHandle) dataHandle;		SetRect(&gPICTRec.curPos[nextNum], 0, 0, 0, 0);/*	After we create the handle for the data, we have to remember that	we have 6 bytes of identifying "garbage" in front of the picture	data.  To remove that, BlockMove all the picture data to the	beginning of the handle and reset the handle's size.  This is kind	of a hassle, but it's really best to store your custom PicComments	this way.  Otherwise, you may misinterpret someone elses comments	or cause them to misinterpret yours.								*/		if (HandToHand((Handle *) &gPICTRec.picture[nextNum]) == noErr)		{			++gPICTRec.numPICTs;			theHandle = (Handle) gPICTRec.picture[nextNum];			BlockMove((Ptr) *theHandle +6, (Ptr) *theHandle, dataSize -6);			SetHandleSize(theHandle, dataSize -6);		}	}}/*------ DisassemblePictures ----------------------------------------------------------------*///	DisassemblePictures ungroups the first picture in the global picture//	record.  It replaces that picture with new pictures of every picture//	it contained.  All drawing is done within another "dummy" picture//	so that nothing draws on the screen. The reason we can't use an empty//	clipping region to do this is that PicComments will be clipped out along//	with everything else, and we'd be hosed.  (We need the PicComments!)	//	This code is written so that it installs the GrafProcs correctly for//	both GrafPorts and CGrafPorts./*----------------------------------------------------------------------------------------*/void DisassemblePictures(){	GrafPtr		curPort;	//QDProcs		theQDProcs;		/* If we're using a GrafPort�			*/	CQDProcs	theCQDProcs;	/* If we're using a CGrafPort�			*/	PicHandle	dummyPICT;/*	Reset the number of pictures in our global picture record to zero.	There's actually one picture there at this point (the composite	one), but we must set this to zero so that our PicComment handler	stores extracted pictures in the right place.						*/	gPICTRec.numPICTs = 0;/*	Get the current port and the standard QDProcs or CQDProcs,	depending on whether we have a GrafPort or CGrafPort.				*/	GetPort(&curPort);	//if (curPort->portBits.rowBytes < 0)				/* CGrafPort�		*/	if (GetPortBitMapForCopyBits(curPort)->rowBytes < 0)	{		SetStdCProcs(&theCQDProcs);		theCQDProcs.commentProc = NewQDCommentProc(CustomPicProc);		//curPort->grafProcs = (QDProcsPtr) &theCQDProcs;		SetPortGrafProcs(curPort, &theCQDProcs);	}	else											/* GrafPort�		*/	{		/*SetStdProcs(&theQDProcs);		theQDProcs.commentProc = NewQDCommentProc(CustomPicProc);		//curPort->grafProcs = (QDProcsPtr) &theQDProcs;		SetPortGrafProcs(curPort, &theQDProcs);*/			}/*	Open our dummy picture and draw into it so that our PicComment	handler is called to parse the picture.  When finished, close the	picture, kill it and remove our grafProcs.							*/	dummyPICT = OpenPicture(&(*gPICTRec.picture[0])->picFrame);	DrawPicture(gPICTRec.picture[0], &(*gPICTRec.picture[0])->picFrame);	ClosePicture();	KillPicture(dummyPICT);	//curPort->grafProcs = NULL;	SetPortGrafProcs(curPort, NULL);}/*------ DoMenuCommand ----------------------------------------------------------------*///		DoMenuCommand handles our menu items./*----------------------------------------------------------------------------------------*/void DoMenuCommand(long menuResult){	int			menuID, menuItem;	//Str255		daName;	/*MenuHandle	theMenu;*/	//GrafPtr		savePort;/*	Get the menu ID and item ID.			*/	menuID = (menuResult >>16) & 0xFFFF;	menuItem = menuResult & 0xFFFF;/*	Do what we're supposed to.				*/	switch (menuID)	{		case mApple:						/*	Apple Menu					*/			switch (menuItem)			{				case iAbout:				/*	-> Handle "About�"			*/					break;				default:					/*	-> The rest are DAs.		*/					/*GetPort(&savePort);					GetMenuItemText(GetMenuHandle(mApple), menuItem, daName);					OpenDeskAcc((ConstStr255Param)daName);					SetPort(savePort);*/					break;			};		case mFile:							/*	File Menu					*/			switch (menuItem)			{				case iQuit:					gQuitting = true;		/*	-> Quit						*/					break;			}	}	HiliteMenu(0);}/*------ EventLoop ----------------------------------------------------------------*///		EventLoop is a main event loop.  It calls WaitNextEvent and//		other nice stuff.  It also makes our pictures assemble, disassemble, //		and move about./*----------------------------------------------------------------------------------------*/void EventLoop(){	EventRecord		theEvent;	WindowPtr		whichWindow;	short			partCode;	Rect			dragRect;	RgnHandle		grayRgn;	char			key, time;	unsigned long	finalTicks;	Rect			tempRect1;/*	Set up the rectangle for where we can drag windows.  Initialize	our "time-through-the-loop" counter to -1 so that it gets bumped	to zero on the first pass.  This will enable us assemble the	grouped picture as we go through the first time.						*/	grayRgn = GetGrayRgn();	//dragRect = (*grayRgn)->rgnBBox;	GetRegionBounds(GetGrayRgn(), &dragRect);	time = -1;/*	We have a counter which goes from 0-26 and is incremented each time	we go through this code. At time = 0, We assemble the grouped image.	At time = 12, we break all the PICTs out of it.  At time = 27, we	cycle back to time = 0.  In between these life altering times,	(at least for groupies), we draw all of our current pictures in	random places.  This clearly shows whether the PICTs are currently	grouped or not.  We go through this loop until the user quits.			*/	do	{		//SetPort(gTheWindow);		SetPortWindowPort(gTheWindow);		time = ++time % 27;	 		if (time == 0)		{			CompositePictures();			/*	Group the pictures.				*/			//EraseRect(&(gTheWindow)->portRect);			EraseRect(GetPortBounds(GetWindowPort(gTheWindow), &tempRect1));		} 		if (time == 12)		{			DisassemblePictures();		/*	Ungroup the pictures.			*/			//EraseRect(&(gTheWindow)->portRect);			EraseRect(GetPortBounds(GetWindowPort(gTheWindow), &tempRect1));		}/*	Move all pictures so we can see their current state.					*/		//MoveThePicts(&(gTheWindow)->portRect);		MoveThePicts(GetPortBounds(GetWindowPort(gTheWindow), &tempRect1));/*	Delay so our graphics don't flash.										*/		Delay((time < 12)? 40:10, &finalTicks);	/*	Handle any pending events.												*/		if (WaitNextEvent(everyEvent, &theEvent, 0, NULL))			switch (theEvent.what)			{				case mouseDown:					/*	Handle mouse clicks.	*/									partCode = FindWindow(theEvent.where, &whichWindow);					switch (partCode)					{						case inContent:							if (whichWindow != FrontWindow())								SelectWindow(whichWindow);							break;							case inDrag:							DragWindow(whichWindow, theEvent.where, &dragRect);							break;							case inMenuBar:							DoMenuCommand(MenuSelect(theEvent.where));							case inSysWindow:							//SystemClick(&theEvent, whichWindow);							break;					}					break;					case updateEvt:					/*	Handle update events.	*/						BeginUpdate((WindowPtr) theEvent.message);						EndUpdate((WindowPtr) theEvent.message);					break;				case keyDown:					/*	Handle key presses.		*/				case autoKey: 					key = (char) (theEvent.message & charCodeMask);					if (((theEvent.modifiers & cmdKey) != 0) && (theEvent.what == keyDown))						DoMenuCommand(MenuKey(key));					break;			}	}	while (!gQuitting);}/*------ MoveThePicts ----------------------------------------------------------------*///	MoveThePicts moves the current pictures� somewhere randomly.  It//	first erases all the pictures in descending order.  Then it redraws//	them in new locations in ascending order. This way we don't wipe out//	any of the new pictures when the old ones are erased./*----------------------------------------------------------------------------------------*/void MoveThePicts(Rect *wBounds){	int		newLeft, newTop, width, height, idx;	float	maxX, maxY;	Rect	picFrame, curPos;/*	First erase all pictures in reverse order.  Also, calculate their	new locations and store those in their curPos fields.				*/	for (idx = gPICTRec.numPICTs -1; idx >= 0; idx--)	{		curPos = gPICTRec.curPos[idx];		EraseRect(&curPos);		picFrame = (*gPICTRec.picture[idx])->picFrame;		width = picFrame.right -picFrame.left;		height = picFrame.bottom -picFrame.top;/*	To calculate new positions, we find the maximum position we can	have for the picture's top left corner.  Then, we find a random	point that's bounded by (0, 0) and that maximum.  Finally, we	set this picture's current position so that it has this point for	its top left corner.												*/		maxX = (wBounds->right - wBounds->left) -width;		maxY = (wBounds->bottom - wBounds->top) -height;				newTop = (((float) Random() +32767)/65534.0) * maxX;		newLeft = (((float) Random() +32767)/65534.0) * maxY;			curPos.top = newTop;		curPos.left = newLeft;		curPos.bottom = newTop +height;		curPos.right = newLeft +width;		gPICTRec.curPos[idx] = curPos;	}/*	Now draw all the pictures in their new positions.					*/	for (idx = 0; idx < gPICTRec.numPICTs; idx++)		DrawPicture(gPICTRec.picture[idx], &gPICTRec.curPos[idx]);}/*------ MakeThePicts ----------------------------------------------------------------*///	MakeThePicts creates the pictures that will be grouped.  These can be//	any QuickDraw pictures.  For this example, I use four pictures; one//	containing a square, one with a circle, one with a triangle and one//	with some text.  These are all stored in the global picture record.	//	This routine is only called once, to put some pictures into the works to//	start with./*----------------------------------------------------------------------------------------*/void MakeThePicts(){	RgnHandle	oldClip;	PolyHandle	trianglePoly;	int			fNum, vPos;/*	Save the current clipping region so that we can restore it later.	Set our own clipping region, so that we know we have a valid one.	Also initialize the number of pictures in our global picture	structure to zero.													*/	oldClip = NewRgn();	GetClip(oldClip);	SetRect(&gPictsBounds, 0, 0, 150, 150);	ClipRect(&gPictsBounds);		gPICTRec.numPICTs = 0;/*	Create a picture with a blue square in it.  We set the curPos	rectangle for all of these pictures to (0, 0, 0, 0) so that	we don't do any unnecessary erasing the first time they enter	MoveTheGroupies.													*/	gPICTRec.picture[0] = OpenPicture(&gPictsBounds);	ForeColor(blueColor);	PaintRect(&gPictsBounds);	ClosePicture();	SetRect(&gPICTRec.curPos[0], 0, 0, 0, 0);	++gPICTRec.numPICTs;/*	Create a picture with a red circle in it.							*/	gPICTRec.picture[1] = OpenPicture(&gPictsBounds);	ForeColor(redColor);	PaintOval(&gPictsBounds);	ClosePicture();	SetRect(&gPICTRec.curPos[1], 0, 0, 0, 0);	++gPICTRec.numPICTs;/*	Create a picture with a green triangle in it.	*/	gPICTRec.picture[2] = OpenPicture(&gPictsBounds);	ForeColor(greenColor);	trianglePoly = OpenPoly();	MoveTo(gPictsBounds.left, gPictsBounds.bottom);	LineTo((gPictsBounds.right - gPictsBounds.left)/2, gPictsBounds.top);	LineTo(gPictsBounds.right, gPictsBounds.bottom);	LineTo(gPictsBounds.left, gPictsBounds.bottom);	ClosePoly();	PaintPoly(trianglePoly);	KillPoly(trianglePoly);	ClosePicture();	SetRect(&gPICTRec.curPos[2], 0, 0, 0, 0);	++gPICTRec.numPICTs;/*	Create a picture with some text in it.	*/	gPICTRec.picture[3] = OpenPicture(&gPictsBounds);	ForeColor(blackColor);	GetFNum((ConstStr255Param) "\pTimes", (short*)&fNum);	TextFont(fNum);	TextSize(12);	TextFont(bold);	vPos = gPictsBounds.top +(gPictsBounds.bottom - gPictsBounds.top)/2;	MoveTo(gPictsBounds.left +10, vPos +10);	DrawString((ConstStr255Param) "\pCustom PicComments");	ClosePicture();	SetRect(&gPICTRec.curPos[3], 0, 0, 0, 0);	++gPICTRec.numPICTs;/*	Restore the original clipping region.								*/	SetClip(oldClip);	DisposeRgn(oldClip);}/*------ ShowThePicts ----------------------------------------------------------------*///	ShowThePicts starts the show.//	First, we find the deepest display because the groupies are a colorful//	bunch.  Then we create a window and some pictures.  Finally, we jump into//	our main event loop./*----------------------------------------------------------------------------------------*/void ShowThePicts(){	Rect		maxRect, deepRect, wBounds;	GDHandle	deepGDH;/*	Find the bounds of the deepest device.  We'll use this to determine	where to put our window.  Passing the maximum enclosing rectangle	to GetMaxDevice assures that we find the deepest device available.	*/	SetRect(&maxRect, -32767, -32767, 32767, 32767);	deepGDH = GetMaxDevice(&maxRect);	deepRect = (*deepGDH)->gdRect;/*	Create a window for our drawing, offset onto the deepest device.	*/		SetRect(&wBounds, 40, 40, 360, 340);	OffsetRect(&wBounds, wBounds.left +deepRect.left, wBounds.top +deepRect.top);	gTheWindow = NewWindow(nil, &wBounds, (ConstStr255Param) "\pCustomPC/B", true, noGrowDocProc, (WindowPtr) -1, false, 1234);	//SetPort(gTheWindow);	SetPortWindowPort(gTheWindow);/*	Create our pictures to group, then go into the work loop.  This	loop continually groups the pictures, draws the grouped picture	in different locations, ungroups the picture, draws the ungrouped	pictures in different locations and repeats until the user quits.	*/    MakeThePicts();	EventLoop();}/*------ CompositePictures ----------------------------------------------------------------*///	CompositePictures groups all of the pictures in the global picture record//	into one "composite" picture.  It removes all the old pictures and//	stores the new one./*----------------------------------------------------------------------------------------*//*	-------------------------------------	main.	-------------------------------------	*/void main(void){    	unsigned long randSeed;	Handle		menuBar;/*	Initialize the toolbox routines.									*/	//InitGraf(&qd.thePort);	//InitFonts();	//InitWindows();	//InitMenus();	//InitDialogs(nil);	InitCursor();/*	Set up our menubar.													*/	menuBar = GetNewMBar(rMenuBar);		/*	Read menus into menu bar	*/	SetMenuBar(menuBar);				/*	and install them.			*/	DisposeHandle(menuBar);		//AppendResMenu(GetMenuHandle(mApple), 'DRVR');	DrawMenuBar();/*	Initialize the random number seed for our hopping groupies and set	our quitting flag to false.  Call the routine that runs everything,	then, quit.															*/    GetDateTime(&randSeed);    gQuitting = false;    ShowThePicts();}