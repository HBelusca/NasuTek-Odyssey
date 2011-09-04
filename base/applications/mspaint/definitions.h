/*
 * PROJECT:     PAINT for Odyssey
 * LICENSE:     LGPL
 * FILE:        base/applications/paint/definitions.h
 * PURPOSE:     Defines the resource ids and other stuff
 * PROGRAMMERS: Benedikt Freisen
 */
 
/* DEFINES **********************************************************/

#define HISTORYSIZE 11
/* HISTORYSIZE = number of possible undo-steps + 1 */

#define SIZEOF(a)  (sizeof(a) / sizeof((a)[0]))
/* sizeof for string constants; equals max. number of characters */

#define IDI_APPICON 500

#define IDB_TOOLBARICONS    510

#define IDI_TRANSPARENT     526
#define IDI_NONTRANSPARENT  527

#define IDC_FILL        530
#define IDC_COLOR       531
#define IDC_ZOOM        532
#define IDC_PEN         533
#define IDC_AIRBRUSH    534

#define IDI_HORZSTRETCH 535
#define IDI_VERTSTRETCH 536

#define ID_MENU 501

#define IDM_FILENEW     201
#define IDM_FILEOPEN    202
#define IDM_FILESAVE    203
#define IDM_FILESAVEAS  204
#define IDM_FILEASWALLPAPERPLANE        210
#define IDM_FILEASWALLPAPERCENTERED     211
#define IDM_FILEASWALLPAPERSTRETCHED    212
#define IDM_FILEEXIT    205

#define IDM_EDITUNDO    220
#define IDM_EDITREDO    221
#define IDM_EDITCUT     222
#define IDM_EDITCOPY    223
#define IDM_EDITPASTE   224
#define IDM_EDITDELETESELECTION 225
#define IDM_EDITINVERTSELECTION 226
#define IDM_EDITSELECTALL   227
#define IDM_EDITCOPYTO      228
#define IDM_EDITPASTEFROM   229

#define IDM_VIEWTOOLBOX         230
#define IDM_VIEWCOLORPALETTE    231
#define IDM_VIEWSTATUSBAR       232
#define IDM_FORMATICONBAR       233
#define IDM_VIEWZOOM125         234
#define IDM_VIEWZOOM25          235
#define IDM_VIEWZOOM50          236
#define IDM_VIEWZOOM100         237
#define IDM_VIEWZOOM200         238
#define IDM_VIEWZOOM400         239
#define IDM_VIEWZOOM800         240
#define IDM_VIEWFULLSCREEN      241
#define IDM_VIEWSHOWGRID        242
#define IDM_VIEWSHOWMINIATURE   243

#define IDM_IMAGEROTATEMIRROR   250
#define IDM_IMAGECHANGESIZE     251
#define IDM_IMAGECROP           252
#define IDM_IMAGEINVERTCOLORS   253
#define IDM_IMAGEATTRIBUTES     254
#define IDM_IMAGEDELETEIMAGE    255
#define IDM_IMAGEDRAWOPAQUE     256

#define IDM_COLORSEDITPALETTE   260

#define IDM_HELPHELPTOPICS  270
#define IDM_HELPINFO        271

//the following 16 numbers need to be in order, increasing by 1
#define ID_FREESEL  600
#define ID_RECTSEL  601
#define ID_RUBBER   602
#define ID_FILL     603
#define ID_COLOR    604
#define ID_ZOOM     605
#define ID_PEN      606
#define ID_BRUSH    607
#define ID_AIRBRUSH 608
#define ID_TEXT     609
#define ID_LINE     610
#define ID_BEZIER   611
#define ID_RECT     612
#define ID_SHAPE    613
#define ID_ELLIPSE  614
#define ID_RRECT    615

//the following 16 numbers need to be in order, increasing by 1
#define TOOL_FREESEL  1
#define TOOL_RECTSEL  2
#define TOOL_RUBBER   3
#define TOOL_FILL     4
#define TOOL_COLOR    5
#define TOOL_ZOOM     6
#define TOOL_PEN      7
#define TOOL_BRUSH    8
#define TOOL_AIRBRUSH 9
#define TOOL_TEXT     10
#define TOOL_LINE     11
#define TOOL_BEZIER   12
#define TOOL_RECT     13
#define TOOL_SHAPE    14
#define TOOL_ELLIPSE  15
#define TOOL_RRECT    16

#define ID_ACCELERATORS 800

#define IDD_MIRRORROTATE        700
#define IDD_MIRRORROTATEGROUP   701
#define IDD_MIRRORROTATERB1     702
#define IDD_MIRRORROTATERB2     703
#define IDD_MIRRORROTATERB3     704
#define IDD_MIRRORROTATERB4     705
#define IDD_MIRRORROTATERB5     706
#define IDD_MIRRORROTATERB6     707

#define IDD_ATTRIBUTES          710
#define IDD_ATTRIBUTESEDIT1     711
#define IDD_ATTRIBUTESEDIT2     712
#define IDD_ATTRIBUTESTEXT1     715
#define IDD_ATTRIBUTESTEXT2     716
#define IDD_ATTRIBUTESTEXT3     717
#define IDD_ATTRIBUTESTEXT4     718
#define IDD_ATTRIBUTESTEXT5     719
#define IDD_ATTRIBUTESTEXT6     720
#define IDD_ATTRIBUTESTEXT7     721
#define IDD_ATTRIBUTESTEXT8     722
#define IDD_ATTRIBUTESSTANDARD  723
#define IDD_ATTRIBUTESGROUP1    724
#define IDD_ATTRIBUTESGROUP2    725
#define IDD_ATTRIBUTESRB1       726
#define IDD_ATTRIBUTESRB2       727
#define IDD_ATTRIBUTESRB3       728
#define IDD_ATTRIBUTESRB4       729
#define IDD_ATTRIBUTESRB5       730

#define IDD_CHANGESIZE      740
#define IDD_CHANGESIZEGROUP 741
#define IDD_CHANGESIZEICON1 742
#define IDD_CHANGESIZETEXT1 743
#define IDD_CHANGESIZEEDIT1 744
#define IDD_CHANGESIZETEXT2 745
#define IDD_CHANGESIZEICON2 746
#define IDD_CHANGESIZETEXT3 747
#define IDD_CHANGESIZEEDIT2 748
#define IDD_CHANGESIZETEXT4 749

#define IDS_PROGRAMNAME 900
#define IDS_WINDOWTITLE 901
#define IDS_INFOTITLE   902
#define IDS_INFOTEXT    903
#define IDS_SAVEPROMPTTEXT  904
#define IDS_DEFAULTFILENAME 905
#define IDS_MINIATURETITLE  906
#define IDS_TOOLTIP1    910
#define IDS_TOOLTIP2    911
#define IDS_TOOLTIP3    912
#define IDS_TOOLTIP4    913
#define IDS_TOOLTIP5    914
#define IDS_TOOLTIP6    915
#define IDS_TOOLTIP7    916
#define IDS_TOOLTIP8    917
#define IDS_TOOLTIP9    918
#define IDS_TOOLTIP10   919
#define IDS_TOOLTIP11   920
#define IDS_TOOLTIP12   921
#define IDS_TOOLTIP13   922
#define IDS_TOOLTIP14   923
#define IDS_TOOLTIP15   924
#define IDS_TOOLTIP16   925

#define IDS_OPENFILTER  926
#define IDS_SAVEFILTER  927
#define IDS_FILESIZE    928
#define IDS_PRINTRES    929
