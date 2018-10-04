// Downloaded from https://developer.x-plane.com/code-sample/fmsutility/


/*
 FMSUtility Example
 Written by Sandy Barbour - 21/01/2005
 
 This examples shows how to access the FMS
 */

#include "XPLMPlugin.h"
#include "XPLMUtilities.h"
#include "XPLMProcessing.h"
#include "XPLMMenus.h"
#include "XPLMGraphics.h"
#include "XPLMPlanes.h"
#include "XPLMDataAccess.h"
#include "XPLMNavigation.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#if IBM
#include <windows.h>
#endif


#define MAX_NAV_TYPES 13
#define MAX_FMS_ENTRIES 100

static int MenuItem1, NavTypeLinePosition;

/// Widgets
static XPWidgetID FMSUtilityWidget = NULL, FMSUtilityWindow1 = NULL, FMSUtilityWindow2 = NULL;
static XPWidgetID GetFMSEntryButton, ClearEntryButton;
static XPWidgetID GetEntryIndexButton, SetEntryIndexButton, GetDestinationEntryButton, SetDestinationEntryButton;
static XPWidgetID EntryIndexEdit, DestinationEntryIndexEdit;
static XPWidgetID GetNumberOfEntriesButton, GetNumberOfEntriesText;
static XPWidgetID IndexCaption, IndexEdit, SegmentCaption, SegmentCaption2, AirportIDCaption, AirportIDEdit, AltitudeCaption, AltitudeEdit;
static XPWidgetID NavTypeCaption, NavTypeEdit, NavTypeText, UpArrow, DownArrow, SetFMSEntryButton;
static XPWidgetID LatCaption, LatEdit, LonCaption, LonEdit, SetLatLonButton;


/// Structure for the nav types and a more meaningful description
typedef struct _NAVTYPE
{
    char Description[80];
    XPLMNavType EnumValue;
} NAVTYPE;

static NAVTYPE NavTypeLookup[MAX_NAV_TYPES] = {{"Unknown",            xplm_Nav_Unknown},
    {"Airport",            xplm_Nav_Airport},
    {"NDB",                xplm_Nav_NDB},
    {"VOR",                xplm_Nav_VOR},
    {"ILS",                xplm_Nav_ILS},
    {"Localizer",        xplm_Nav_Localizer},
    {"Glide Slope",        xplm_Nav_GlideSlope},
    {"Outer Marker",    xplm_Nav_OuterMarker},
    {"Middle Marker",    xplm_Nav_MiddleMarker},
    {"Inner Marker",    xplm_Nav_InnerMarker},
    {"Fix",                xplm_Nav_Fix},
    {"DME",                xplm_Nav_DME},
    {"Lat/Lon",            xplm_Nav_LatLon}};

/// Utility functions and callbacks

/// This is used to get around a sprintf float bug with codewarrior on the mac
inline    float    HACKFLOAT(float val)
{
    return val;
}
/*
 #if IBM || LIN
 inline    float    HACKFLOAT(float val)
 {
 return val;
 }
 #else
 inline long long HACKFLOAT(float val)
 {
 double    d = val;
 long long temp;
 temp = *((long long *) &d);
 return temp;
 }
 #endif
 */

static void FMSUtilityMenuHandler(void *, void *);
static void CreateFMSUtilityWidget(int x1, int y1, int w, int h);
static int GetCBIndex(int Type);

static int FMSUtilityHandler(
                             XPWidgetMessage            inMessage,
                             XPWidgetID                inWidget,
                             intptr_t                inParam1,
                             intptr_t                inParam2);


PLUGIN_API int XPluginStart(
                            char *        outName,
                            char *        outSig,
                            char *        outDesc)
{
    XPLMMenuID    id;
    int            item;
    
    strcpy(outName, "FMS Utility");
    strcpy(outSig, "xpsdk.experimental.FMSUtility");
    strcpy(outDesc, "A plug-in that accesses the FMS.");
    
    // Create our menu
    item = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "FMSUtility", NULL, 1);
    id = XPLMCreateMenu("FMSUtility", XPLMFindPluginsMenu(), item, FMSUtilityMenuHandler, NULL);
    XPLMAppendMenuItem(id, "Utility Panel", (void *)"FMSUtility", 1);
    
    // Flag to tell us if the widget is being displayed.
    MenuItem1 = 0;
    NavTypeLinePosition = 0;
    
    return 1;
}

PLUGIN_API void    XPluginStop(void)
{
    if (MenuItem1 == 1)
    {
        XPDestroyWidget(FMSUtilityWidget, 1);
        MenuItem1 = 0;
    }
}

PLUGIN_API int XPluginEnable(void)
{
    return 1;
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam)
{
}


// This will create our widget from menu selection.
// MenuItem1 flag stops it from being created more than once.
void FMSUtilityMenuHandler(void * mRef, void * iRef)
{
    // If menu selected create our widget dialog
    if (!strcmp((char *) iRef, "FMSUtility"))
    {
        if (MenuItem1 == 0)
        {
            CreateFMSUtilityWidget(221, 640, 420, 290);
            MenuItem1 = 1;
        }
        else
            if(!XPIsWidgetVisible(FMSUtilityWidget))
                XPShowWidget(FMSUtilityWidget);
    }
}


// This will create our widget dialog.
// I have made all child widgets relative to the input paramter.
// This makes it easy to position the dialog
void CreateFMSUtilityWidget(int x, int y, int w, int h)
{
    int x2 = x + w;
    int y2 = y - h;
    char Buffer[255];
    
    
    strcpy(Buffer, "FMS Example by Sandy Barbour - 2005");
    
    // Create the Main Widget window
    FMSUtilityWidget = XPCreateWidget(x, y, x2, y2,
                                      1,    // Visible
                                      Buffer,    // desc
                                      1,        // root
                                      NULL,    // no container
                                      xpWidgetClass_MainWindow);
    
    // Add Close Box decorations to the Main Widget
    XPSetWidgetProperty(FMSUtilityWidget, xpProperty_MainWindowHasCloseBoxes, 1);
    
    // Create the Sub Widget1 window
    FMSUtilityWindow1 = XPCreateWidget(x+10, y-30, x+160, y2+10,
                                       1,    // Visible
                                       "",    // desc
                                       0,        // root
                                       FMSUtilityWidget,
                                       xpWidgetClass_SubWindow);
    
    // Set the style to sub window
    XPSetWidgetProperty(FMSUtilityWindow1, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);
    
    // Create the Sub Widget2 window
    FMSUtilityWindow2 = XPCreateWidget(x+170, y-30, x2-10, y2+10,
                                       1,    // Visible
                                       "",    // desc
                                       0,        // root
                                       FMSUtilityWidget,
                                       xpWidgetClass_SubWindow);
    
    // Set the style to sub window
    XPSetWidgetProperty(FMSUtilityWindow2, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);
    
    /// Entry Index
    GetEntryIndexButton = XPCreateWidget(x+20, y-40, x+110, y-62,
                                         1, " Get Entry Index", 0, FMSUtilityWidget,
                                         xpWidgetClass_Button);
    
    XPSetWidgetProperty(GetEntryIndexButton, xpProperty_ButtonType, xpPushButton);
    
    EntryIndexEdit = XPCreateWidget(x+120, y-40, x+150, y-62,
                                    1, "0", 0, FMSUtilityWidget,
                                    xpWidgetClass_TextField);
    
    XPSetWidgetProperty(EntryIndexEdit, xpProperty_TextFieldType, xpTextEntryField);
    XPSetWidgetProperty(EntryIndexEdit, xpProperty_Enabled, 1);
    
    SetEntryIndexButton = XPCreateWidget(x+20, y-70, x+110, y-92,
                                         1, " Set Entry Index", 0, FMSUtilityWidget,
                                         xpWidgetClass_Button);
    
    XPSetWidgetProperty(SetEntryIndexButton, xpProperty_ButtonType, xpPushButton);
    
    /// Destination Index
    GetDestinationEntryButton = XPCreateWidget(x+20, y-100, x+110, y-122,
                                               1, " Get Dest Index", 0, FMSUtilityWidget,
                                               xpWidgetClass_Button);
    
    XPSetWidgetProperty(GetDestinationEntryButton, xpProperty_ButtonType, xpPushButton);
    
    DestinationEntryIndexEdit = XPCreateWidget(x+120, y-100, x+150, y-122,
                                               1, "0", 0, FMSUtilityWidget,
                                               xpWidgetClass_TextField);
    
    XPSetWidgetProperty(DestinationEntryIndexEdit, xpProperty_TextFieldType, xpTextEntryField);
    XPSetWidgetProperty(DestinationEntryIndexEdit, xpProperty_Enabled, 1);
    
    SetDestinationEntryButton = XPCreateWidget(x+20, y-130, x+110, y-152,
                                               1, " Set Dest Index", 0, FMSUtilityWidget,
                                               xpWidgetClass_Button);
    
    XPSetWidgetProperty(SetDestinationEntryButton, xpProperty_ButtonType, xpPushButton);
    
    /// Number of Entries
    GetNumberOfEntriesButton = XPCreateWidget(x+20, y-160, x+110, y-182,
                                              1, " Get No. Entries", 0, FMSUtilityWidget,
                                              xpWidgetClass_Button);
    
    XPSetWidgetProperty(GetNumberOfEntriesButton, xpProperty_ButtonType, xpPushButton);
    
    GetNumberOfEntriesText = XPCreateWidget(x+120, y-160, x+150, y-182,
                                            1, "", 0, FMSUtilityWidget,
                                            xpWidgetClass_TextField);
    
    XPSetWidgetProperty(GetNumberOfEntriesText, xpProperty_TextFieldType, xpTextEntryField);
    XPSetWidgetProperty(GetNumberOfEntriesText, xpProperty_Enabled, 0);
    
    /// Clear Entry
    ClearEntryButton = XPCreateWidget(x+20, y-190, x+110, y-212,
                                      1, " Clear Entry", 0, FMSUtilityWidget,
                                      xpWidgetClass_Button);
    
    XPSetWidgetProperty(ClearEntryButton, xpProperty_ButtonType, xpPushButton);
    
    /// Index (Segment - 1)
    IndexCaption = XPCreateWidget(x+180, y-40, x+230, y-62,
                                  1, "Index", 0, FMSUtilityWidget,
                                  xpWidgetClass_Caption);
    
    IndexEdit = XPCreateWidget(x+240, y-40, x+290, y-62,
                               1, "0", 0, FMSUtilityWidget,
                               xpWidgetClass_TextField);
    
    XPSetWidgetProperty(IndexEdit, xpProperty_TextFieldType, xpTextEntryField);
    XPSetWidgetProperty(IndexEdit, xpProperty_Enabled, 1);
    
    SegmentCaption = XPCreateWidget(x+300, y-40, x+350, y-62,
                                    1, "Segment", 0, FMSUtilityWidget,
                                    xpWidgetClass_Caption);
    
    SegmentCaption2 = XPCreateWidget(x+360, y-40, x+410, y-62,
                                     1, "1", 0, FMSUtilityWidget,
                                     xpWidgetClass_Caption);
    
    /// Airport ID
    AirportIDCaption = XPCreateWidget(x+180, y-70, x+230, y-92,
                                      1, "Airport ID", 0, FMSUtilityWidget,
                                      xpWidgetClass_Caption);
    
    AirportIDEdit = XPCreateWidget(x+240, y-70, x+290, y-92,
                                   1, "----", 0, FMSUtilityWidget,
                                   xpWidgetClass_TextField);
    
    XPSetWidgetProperty(AirportIDEdit, xpProperty_TextFieldType, xpTextEntryField);
    XPSetWidgetProperty(AirportIDEdit, xpProperty_Enabled, 1);
    
    /// Altitude
    AltitudeCaption = XPCreateWidget(x+180, y-100, x+230, y-122,
                                     1, "Altitude", 0, FMSUtilityWidget,
                                     xpWidgetClass_Caption);
    
    AltitudeEdit = XPCreateWidget(x+240, y-100, x+290, y-122,
                                  1, "0", 0, FMSUtilityWidget,
                                  xpWidgetClass_TextField);
    
    XPSetWidgetProperty(AltitudeEdit, xpProperty_TextFieldType, xpTextEntryField);
    XPSetWidgetProperty(AltitudeEdit, xpProperty_Enabled, 1);
    
    /// Nav Type
    NavTypeCaption = XPCreateWidget(x+180, y-130, x+230, y-152,
                                    1, "Nav Type", 0, FMSUtilityWidget,
                                    xpWidgetClass_Caption);
    
    sprintf(Buffer, "%s", NavTypeLookup[0].Description);
    NavTypeEdit = XPCreateWidget(x+240, y-130, x+340, y-152,
                                 1, Buffer, 0, FMSUtilityWidget,
                                 xpWidgetClass_TextField);
    
    XPSetWidgetProperty(NavTypeEdit, xpProperty_TextFieldType, xpTextEntryField);
    XPSetWidgetProperty(NavTypeEdit, xpProperty_Enabled, 0);
    
    // Used for selecting Nav Type
    UpArrow = XPCreateWidget(x+340, y-130, x+362, y-141,
                             1, "", 0, FMSUtilityWidget,
                             xpWidgetClass_Button);
    
    XPSetWidgetProperty(UpArrow, xpProperty_ButtonType, xpLittleUpArrow);
    
    // Used for selecting Nav Type
    DownArrow = XPCreateWidget(x+340, y-141, x+362, y-152,
                               1, "", 0, FMSUtilityWidget,
                               xpWidgetClass_Button);
    
    XPSetWidgetProperty(DownArrow, xpProperty_ButtonType, xpLittleDownArrow);
    
    NavTypeText = XPCreateWidget(x+362, y-130, x+400, y-152,
                                 1, "0", 0, FMSUtilityWidget,
                                 xpWidgetClass_TextField);
    
    XPSetWidgetProperty(NavTypeText, xpProperty_TextFieldType, xpTextEntryField);
    XPSetWidgetProperty(NavTypeText, xpProperty_Enabled, 0);
    
    /// Get FMS Entry Info
    GetFMSEntryButton = XPCreateWidget(x+180, y-160, x+270, y-182,
                                       1, " Get FMS Entry", 0, FMSUtilityWidget,
                                       xpWidgetClass_Button);
    
    XPSetWidgetProperty(GetFMSEntryButton, xpProperty_ButtonType, xpPushButton);
    
    /// Set FMS Entry Info
    SetFMSEntryButton = XPCreateWidget(x+280, y-160, x+370, y-182,
                                       1, " Set FMS Entry", 0, FMSUtilityWidget,
                                       xpWidgetClass_Button);
    
    XPSetWidgetProperty(SetFMSEntryButton, xpProperty_ButtonType, xpPushButton);
    
    /// Lat / Lon
    LatCaption = XPCreateWidget(x+180, y-190, x+230, y-212,
                                1, "Latitude", 0, FMSUtilityWidget,
                                xpWidgetClass_Caption);
    
    LatEdit = XPCreateWidget(x+240, y-190, x+310, y-212,
                             1, "0", 0, FMSUtilityWidget,
                             xpWidgetClass_TextField);
    
    XPSetWidgetProperty(LatEdit, xpProperty_TextFieldType, xpTextEntryField);
    XPSetWidgetProperty(LatEdit, xpProperty_Enabled, 1);
    
    LonCaption = XPCreateWidget(x+180, y-220, x+230, y-242,
                                1, "Longitude", 0, FMSUtilityWidget,
                                xpWidgetClass_Caption);
    
    LonEdit = XPCreateWidget(x+240, y-220, x+310, y-242,
                             1, "0", 0, FMSUtilityWidget,
                             xpWidgetClass_TextField);
    
    XPSetWidgetProperty(LonEdit, xpProperty_TextFieldType, xpTextEntryField);
    XPSetWidgetProperty(LonEdit, xpProperty_Enabled, 1);
    
    SetLatLonButton = XPCreateWidget(x+180, y-250, x+270, y-272,
                                     1, " Set Lat/Lon", 0, FMSUtilityWidget,
                                     xpWidgetClass_Button);
    
    XPSetWidgetProperty(SetLatLonButton, xpProperty_ButtonType, xpPushButton);
    
    // Register our widget handler
    XPAddWidgetCallback(FMSUtilityWidget, FMSUtilityHandler);
}

// This is the handler for our widget
// It can be used to process button presses etc.
// In this example we are only interested when the close box is pressed
int    FMSUtilityHandler(
                         XPWidgetMessage            inMessage,
                         XPWidgetID                inWidget,
                         intptr_t                inParam1,
                         intptr_t                inParam2)
{
    void *Param;
    Param = 0;
    char Buffer[255];
    
    // Close button will get rid of our main widget
    // All child widgets will get the bullet as well
    if (inMessage == xpMessage_CloseButtonPushed)
    {
        if (MenuItem1 == 1)
        {
            XPHideWidget(FMSUtilityWidget);
        }
        return 1;
    }
    
    // Handle any button pushes
    if (inMessage == xpMsg_PushButtonPressed)
    {
        /// Most of these handlers get a value.
        /// It then has to be converted to a string using sprintf.
        /// This is because "XPSetWidgetDescriptor" expects a string as its second parameter.
        if (inParam1 == (intptr_t)ClearEntryButton)
        {
            XPLMClearFMSEntry(XPLMGetDisplayedFMSEntry());
            return 1;
        }
        
        if (inParam1 == (intptr_t)GetEntryIndexButton)
        {
            int Index = XPLMGetDisplayedFMSEntry();
            sprintf(Buffer, "%d", Index);
            XPSetWidgetDescriptor(EntryIndexEdit, Buffer);
            return 1;
        }
        
        if (inParam1 == (intptr_t)SetEntryIndexButton)
        {
            XPGetWidgetDescriptor(EntryIndexEdit, Buffer, sizeof(Buffer));
            int Index = atoi(Buffer);
            XPLMSetDisplayedFMSEntry(Index);
            return 1;
        }
        
        if (inParam1 == (intptr_t)GetDestinationEntryButton)
        {
            int Index = XPLMGetDestinationFMSEntry();
            sprintf(Buffer, "%d", Index);
            XPSetWidgetDescriptor(DestinationEntryIndexEdit, Buffer);
            return 1;
        }
        
        if (inParam1 == (intptr_t)SetDestinationEntryButton)
        {
            XPGetWidgetDescriptor(DestinationEntryIndexEdit, Buffer, sizeof(Buffer));
            int Index = atoi(Buffer);
            XPLMSetDestinationFMSEntry(Index);
            return 1;
        }
        
        if (inParam1 == (intptr_t)GetNumberOfEntriesButton)
        {
            int Count = XPLMCountFMSEntries();
            sprintf(Buffer, "%d", Count);
            XPSetWidgetDescriptor(GetNumberOfEntriesText, Buffer);
            return 1;
        }
        
        if (inParam1 == (intptr_t)GetFMSEntryButton)
        {
            int Index = XPLMGetDisplayedFMSEntry();
            XPLMNavType outType;
            char outID[80];
            XPLMNavRef outRef;
            int outAltitude;
            float outLat;
            float outLon;
            
            XPLMGetFMSEntryInfo(Index, &outType, outID, &outRef, &outAltitude, &outLat, &outLon);
            sprintf(Buffer, "%d", Index);
            XPSetWidgetDescriptor(IndexEdit, Buffer);
            sprintf(Buffer, "%d", Index+1);
            XPSetWidgetDescriptor(SegmentCaption2, Buffer);
            if (outType == xplm_Nav_LatLon)
                XPSetWidgetDescriptor(AirportIDEdit, "----");
            else
                XPSetWidgetDescriptor(AirportIDEdit, outID);
            sprintf(Buffer, "%d", outAltitude);
            XPSetWidgetDescriptor(AltitudeEdit, Buffer);
            XPSetWidgetDescriptor(NavTypeEdit, NavTypeLookup[GetCBIndex(outType)].Description);
            sprintf(Buffer, "%d", NavTypeLookup[GetCBIndex(outType)].EnumValue);
            XPSetWidgetDescriptor(NavTypeText, Buffer);
            sprintf(Buffer, "%+05.2f", HACKFLOAT(outLat));
            XPSetWidgetDescriptor(LatEdit, Buffer);
            sprintf(Buffer, "%+05.2f", HACKFLOAT(outLon));
            XPSetWidgetDescriptor(LonEdit, Buffer);
            
            return 1;
        }
        
        if (inParam1 == (intptr_t)SetFMSEntryButton)
        {
            XPGetWidgetDescriptor(IndexEdit, Buffer, sizeof(Buffer));
            int Index = atoi(Buffer);
            sprintf(Buffer, "%d", Index+1);
            XPSetWidgetDescriptor(SegmentCaption2, Buffer);
            XPGetWidgetDescriptor(AltitudeEdit, Buffer, sizeof(Buffer));
            int Altitude = atoi(Buffer);
            XPGetWidgetDescriptor(NavTypeText, Buffer, sizeof(Buffer));
            int NavType = atoi(Buffer);
            XPGetWidgetDescriptor(AirportIDEdit, Buffer, sizeof(Buffer));
            XPLMSetFMSEntryInfo(Index, XPLMFindNavAid(NULL, Buffer, NULL, NULL, NULL, NavType), Altitude);
            
            return 1;
        }
        
        if (inParam1 == (intptr_t)SetLatLonButton)
        {
            XPGetWidgetDescriptor(IndexEdit, Buffer, sizeof(Buffer));
            int Index = atoi(Buffer);
            sprintf(Buffer, "%d", Index+1);
            XPSetWidgetDescriptor(SegmentCaption2, Buffer);
            XPGetWidgetDescriptor(AltitudeEdit, Buffer, sizeof(Buffer));
            int Altitude = atoi(Buffer);
            XPGetWidgetDescriptor(LatEdit, Buffer, sizeof(Buffer));
            float Lat = atof(Buffer);
            XPGetWidgetDescriptor(LonEdit, Buffer, sizeof(Buffer));
            float Lon = atof(Buffer);
            XPLMSetFMSEntryLatLon(Index, Lat, Lon, Altitude);
            
            return 1;
        }
        
        // Up Arrow is used to modify the NavTypeLookup Array Index
        if (inParam1 == (intptr_t)UpArrow)
        {
            NavTypeLinePosition--;
            if (NavTypeLinePosition < 0)
                NavTypeLinePosition = MAX_NAV_TYPES-1;
            XPSetWidgetDescriptor(NavTypeEdit, NavTypeLookup[NavTypeLinePosition].Description);
            sprintf(Buffer, "%d", NavTypeLookup[NavTypeLinePosition].EnumValue);
            XPSetWidgetDescriptor(NavTypeText, Buffer);
            
            return 1;
        }
        
        // Down Arrow is used to modify the NavTypeLookup Array Index
        if (inParam1 == (intptr_t)DownArrow)
        {
            NavTypeLinePosition++;
            if (NavTypeLinePosition > MAX_NAV_TYPES-1)
                NavTypeLinePosition = 0;
            XPSetWidgetDescriptor(NavTypeEdit, NavTypeLookup[NavTypeLinePosition].Description);
            sprintf(Buffer, "%d", NavTypeLookup[NavTypeLinePosition].EnumValue);
            XPSetWidgetDescriptor(NavTypeText, Buffer);
            
            return 1;
        }
        
    }
    
    return 0;
}

/// This function takes an XPLMNavType and
/// returns the index into the NavTypeLookup array.
/// We can then use that index to access the description or enum.
int GetCBIndex(int Type)
{
    int CBIndex = 0;
    
    for (int Index=0; Index<MAX_NAV_TYPES; Index++)
    {
        if (NavTypeLookup[Index].EnumValue == Type)
        {
            CBIndex = Index;
            break;
        }
    }
    return CBIndex;
}

