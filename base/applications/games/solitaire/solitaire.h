
#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"
#include "cardlib.h"

extern CardWindow SolWnd;
extern TCHAR szAppName[];
extern bool fGameStarted;

#define OPTION_SHOW_STATUS       4
#define OPTION_THREE_CARDS       8
#define CARDBACK_START           IDC_CARDBACK1
#define CARDBACK_END             IDC_CARDBACK12
#define NUM_CARDBACKS            (CARDBACK_END - CARDBACK_START + 1)
#define CARDBACK_RES_START       53
/* Display option cards with half the size */
#define CARDBACK_OPTIONS_WIDTH   36
#define CARDBACK_OPTIONS_HEIGHT  48

extern DWORD dwOptions;

void CreateSol(void);
void NewGame(void);

#define NUM_ROW_STACKS     7
#define DECK_ID            1
#define PILE_ID            2
#define SUIT_ID            4
#define ROW_ID             10

// Various metrics used for placing the objects and computing the minimum window size
#define X_BORDER                 20
#define X_PILE_BORDER            18
#define X_ROWSTACK_BORDER        10
#define X_SUITSTACK_BORDER       10
#define Y_BORDER                 20
#define Y_ROWSTACK_BORDER        32
extern int yRowStackCardOffset;

extern CardRegion *pDeck;
extern CardRegion *pPile;
extern CardRegion *pSuitStack[];
extern CardRegion *pRowStack[];


bool CARDLIBPROC RowStackDragProc(CardRegion &stackobj, int iNumCards);
bool CARDLIBPROC RowStackDropProc(CardRegion &stackobj,  CardStack &dragcards);

bool CARDLIBPROC SuitStackDropProc(CardRegion &stackobj, CardStack &dragcards);
void CARDLIBPROC SuitStackAddProc(CardRegion &stackobj, const CardStack &added);

void CARDLIBPROC RowStackClickProc(CardRegion &stackobj, int iNumClicked);
void CARDLIBPROC RowStackDblClickProc(CardRegion &stackobj, int iNumClicked);

void CARDLIBPROC DeckClickProc(CardRegion &stackobj, int iNumClicked);
void CARDLIBPROC PileDblClickProc(CardRegion &stackobj, int iNumClicked);

void CARDLIBPROC PileRemoveProc(CardRegion &stackobj, int iRemoved);
