//
// Model Selection using previews. For SA-MP 0.3x and above.
// - D0erfler 2013
// - Kye 2013
//
// Version 1.1 R2

/*
* Functions to be used

LoadModelSelectionMenu(const f_name[])
HideModelSelectionMenu(playerid)
ShowModelSelectionMenu(playerid, ListID, const header_text[], dialogBGcolor = 0x4A5A6BBB, previewBGcolor = 0x88888899 , tdSelectionColor = 0xFFFF00AA)
ShowModelSelectionMenuEx(playerid, const items_array[], item_amount, const header_text[], extraid, Float:Xrot = 0.0, Float:Yrot = 0.0, Float:Zrot = 0.0, Float:mZoom = 1.0, dialogBGcolor = 0x4A5A6BBB, previewBGcolor = 0x88888899 , tdSelectionColor = 0xFFFF00AA)

* Functions for Pawno
native LoadModelSelectionMenu(const f_name[]);
native HideModelSelectionMenu(playerid);
native ShowModelSelectionMenu(playerid, ListID, const header_text[], dialogBGcolor = 0x4A5A6BBB, previewBGcolor = 0x88888899 , tdSelectionColor = 0xFFFF00AA);
native ShowModelSelectionMenuEx(playerid, const items_array[], item_amount, const header_text[], extraid, Float:Xrot = 0.0, Float:Yrot = 0.0, Float:Zrot = 0.0, Float:mZoom = 1.0, dialogBGcolor = 0x4A5A6BBB, previewBGcolor = 0x88888899 , tdSelectionColor = 0xFFFF00AA)
*/


#tryinclude <a_samp>

// Callbacks
forward OnPlayerModelSelection(playerid, response, listid, modelid);
forward OnPlayerModelSelectionEx(playerid, response, extraid, modelid);

// Directives & Macros & Constants
#define mS_TOTAL_ITEMS         	(1000)
#define mS_TOTAL_LISTS			(20)
#define mS_TOTAL_ROT_ZOOM		(100)

#define mS_CUSTOM_MAX_ITEMS		(300)
static gCustomList[MAX_PLAYERS][mS_CUSTOM_MAX_ITEMS];

#define mS_INVALID_LISTID		(mS_TOTAL_LISTS)
#define mS_CUSTOM_LISTID		(mS_TOTAL_LISTS+1)

#if !defined MSELECTION_USE_SPANISH
    #define mS_NEXT_TEXT   "~g~Next"
    #define mS_PREV_TEXT   "~g~Prev"
    #define mS_CANCEL_TEXT   "~r~Exit"
#else
    #define mS_NEXT_TEXT   "~g~Prox"
    #define mS_PREV_TEXT   "~g~Ant"
    #define mS_CANCEL_TEXT   "~r~Salir"
#endif

#define mS_SELECTION_ITEMS 		(21)
#define mS_ITEMS_PER_LINE  		(7)
#define mS_DIALOG_BASE_X   	(75.0)
#define mS_DIALOG_BASE_Y   	(130.0)
#define mS_DIALOG_WIDTH    	(550.0)
#define mS_DIALOG_HEIGHT   	(180.0)
#define mS_SPRITE_DIM_X    	(60.0)
#define mS_SPRITE_DIM_Y    	(70.0)
#define mS_LIST_START			(0)
#define mS_LIST_END				(1)
#define mS_ITEM_MODEL			(0)
#define mS_ITEM_ROT_ZOOM_ID     (1)

static PlayerText:gCurrentPageTextDrawId[MAX_PLAYERS];
static PlayerText:gHeaderTextDrawId[MAX_PLAYERS];
static PlayerText:gBackgroundTextDrawId[MAX_PLAYERS];
static PlayerText:gNextButtonTextDrawId[MAX_PLAYERS];
static PlayerText:gPrevButtonTextDrawId[MAX_PLAYERS];
static PlayerText:gCancelButtonTextDrawId[MAX_PLAYERS];
static PlayerText:gSelectionItems[MAX_PLAYERS][mS_SELECTION_ITEMS];
static gSelectionItemsTag[MAX_PLAYERS][mS_SELECTION_ITEMS];
static gItemAt[MAX_PLAYERS];
static gLists[mS_TOTAL_LISTS][2];
static gItemList[mS_TOTAL_ITEMS][2];

static Float:gRotZoom[mS_TOTAL_ROT_ZOOM][4];
static gItemAmount = 0;
static gListAmount = 0;
static gRotZoomAmount = 0;

/// <summary>
/// 
/// </summary>
/// <param name="ListID"></param>
/// <returns></returns>
stock mS_GetNumberOfPages(ListID) {
    new ItemAmount = mS_GetAmountOfListItems(ListID);
    if ((ItemAmount >= mS_SELECTION_ITEMS) && (ItemAmount % mS_SELECTION_ITEMS) == 0) {
        return (ItemAmount / mS_SELECTION_ITEMS);
    } else return (ItemAmount / mS_SELECTION_ITEMS) + 1;
}

/// <summary>
/// 
/// </summary>
/// <param name="playerid"></param>
/// <returns></returns>
stock mS_GetNumberOfPagesEx(playerid) {
    new ItemAmount = mS_GetAmountOfListItemsEx(playerid);
    if ((ItemAmount >= mS_SELECTION_ITEMS) && (ItemAmount % mS_SELECTION_ITEMS) == 0) {
        return (ItemAmount / mS_SELECTION_ITEMS);
    } else return (ItemAmount / mS_SELECTION_ITEMS) + 1;
}

/// <summary>
/// 
/// </summary>
/// <param name="ListID"></param>
/// <returns></returns>
stock mS_GetAmountOfListItems(ListID) {
    return (gLists[ListID][mS_LIST_END] - gLists[ListID][mS_LIST_START]) + 1;
}

/// <summary>
/// 
/// </summary>
/// <param name="playerid"></param>
/// <returns></returns>
stock mS_GetAmountOfListItemsEx(playerid) {
    return GetPVarInt(playerid, "mS_custom_item_amount");
}

/// <summary>
/// 
/// </summary>
/// <param name="playerid"></param>
/// <returns></returns>
stock mS_GetPlayerCurrentListID(playerid) {
    if (GetPVarInt(playerid, "mS_list_active") == 1) return GetPVarInt(playerid, "mS_list_id");
    else return mS_INVALID_LISTID;
}

/// <summary>
/// 
/// </summary>
/// <param name="playerid"></param>
/// <param name="Float:Xpos"></param>
/// <param name="Float:Ypos"></param>
/// <returns></returns>
stock PlayerText:mS_CreateCurrentPageTextDraw(playerid, Float:Xpos, Float:Ypos) {
    static PlayerText:txtInit;
    txtInit = CreatePlayerTextDraw(playerid, Xpos, Ypos, "0/0");
    PlayerTextDrawUseBox(playerid, txtInit, 0);
    PlayerTextDrawLetterSize(playerid, txtInit, 0.4, 1.1);
    PlayerTextDrawFont(playerid, txtInit, 1);
    PlayerTextDrawSetShadow(playerid, txtInit, 0);
    PlayerTextDrawSetOutline(playerid, txtInit, 1);
    PlayerTextDrawColor(playerid, txtInit, 0xACCBF1FF);
    PlayerTextDrawShow(playerid, txtInit);
    return txtInit;
}

/// <summary>
/// 
/// </summary>
/// <param name="playerid"></param>
/// <param name="Float:Xpos"></param>
/// <param name="Float:Ypos"></param>
/// <param name="Float:Width"></param>
/// <param name="Float:Height"></param>
/// <param name="const button_text[]"></param>
/// <returns></returns>
stock PlayerText:mS_CreatePlayerDialogButton(playerid, Float:Xpos, Float:Ypos, Float:Width, Float:Height, const button_text[]) {
    static PlayerText:txtInit;
    txtInit = CreatePlayerTextDraw(playerid, Xpos, Ypos, button_text);
    PlayerTextDrawUseBox(playerid, txtInit, 1);
    PlayerTextDrawBoxColor(playerid, txtInit, 0x000000FF);
    PlayerTextDrawBackgroundColor(playerid, txtInit, 0x000000FF);
    PlayerTextDrawLetterSize(playerid, txtInit, 0.4, 1.1);
    PlayerTextDrawFont(playerid, txtInit, 1);
    PlayerTextDrawSetShadow(playerid, txtInit, 0);
    PlayerTextDrawSetOutline(playerid, txtInit, 0);
    PlayerTextDrawColor(playerid, txtInit, 0x4A5A6BFF);
    PlayerTextDrawSetSelectable(playerid, txtInit, 1);
    PlayerTextDrawAlignment(playerid, txtInit, 2);
    PlayerTextDrawTextSize(playerid, txtInit, Height, Width);
    PlayerTextDrawShow(playerid, txtInit);
    return txtInit;
}

/// <summary>
/// 
/// </summary>
/// <param name="playerid"></param>
/// <param name="Float:Xpos"></param>
/// <param name="Float:Ypos"></param>
/// <param name="const header_text[]"></param>
/// <returns></returns>
stock PlayerText:mS_CreatePlayerHeaderTextDraw(playerid, Float:Xpos, Float:Ypos, const header_text[]) {
    static PlayerText:txtInit;
    txtInit = CreatePlayerTextDraw(playerid, Xpos, Ypos, header_text);
    PlayerTextDrawUseBox(playerid, txtInit, 0);
    PlayerTextDrawLetterSize(playerid, txtInit, 1.25, 3.0);
    PlayerTextDrawFont(playerid, txtInit, 0);
    PlayerTextDrawSetShadow(playerid, txtInit, 0);
    PlayerTextDrawSetOutline(playerid, txtInit, 1);
    PlayerTextDrawColor(playerid, txtInit, 0xACCBF1FF);
    PlayerTextDrawShow(playerid, txtInit);
    return txtInit;
}

/// <summary>
/// 
/// </summary>
/// <param name="playerid"></param>
/// <param name="Float:Xpos"></param>
/// <param name="Float:Ypos"></param>
/// <param name="Float:Width"></param>
/// <param name="Float:Height"></param>
/// <param name="bgcolor"></param>
/// <returns></returns>
stock PlayerText:mS_CreatePlayerBGTextDraw(playerid, Float:Xpos, Float:Ypos, Float:Width, Float:Height, bgcolor) {
    static PlayerText:txtBackground = CreatePlayerTextDraw(playerid, Xpos, Ypos, "                                            ~n~");
    PlayerTextDrawUseBox(playerid, txtBackground, 1);
    PlayerTextDrawBoxColor(playerid, txtBackground, bgcolor);
    PlayerTextDrawLetterSize(playerid, txtBackground, 5.0, 5.0);
    PlayerTextDrawFont(playerid, txtBackground, 0);
    PlayerTextDrawSetShadow(playerid, txtBackground, 0);
    PlayerTextDrawSetOutline(playerid, txtBackground, 0);
    PlayerTextDrawColor(playerid, txtBackground, 0x000000FF);
    PlayerTextDrawTextSize(playerid, txtBackground, Width, Height);
    PlayerTextDrawBackgroundColor(playerid, txtBackground, bgcolor);
    PlayerTextDrawShow(playerid, txtBackground);
    return txtBackground;
}

/// <summary>
/// 
/// </summary>
/// <param name="playerid"></param>
/// <param name="modelindex"></param>
/// <param name="Float:Xpos"></param>
/// <param name="Float:Ypos"></param>
/// <param name="Float:Xrot"></param>
/// <param name="Float:Yrot"></param>
/// <param name="Float:Zrot"></param>
/// <param name="Float:mZoom"></param>
/// <param name="Float:width"></param>
/// <param name="Float:height"></param>
/// <param name="bgcolor"></param>
/// <returns></returns>
stock PlayerText:mS_CreateMPTextDraw(playerid, modelindex, Float:Xpos, Float:Ypos, Float:Xrot, Float:Yrot, Float:Zrot, Float:mZoom, Float:width, Float:height, bgcolor) {
    new PlayerText:txtPlayerSprite = CreatePlayerTextDraw(playerid, Xpos, Ypos, "");
    PlayerTextDrawFont(playerid, txtPlayerSprite, TEXT_DRAW_FONT_MODEL_PREVIEW);
    PlayerTextDrawColor(playerid, txtPlayerSprite, 0xFFFFFFFF);
    PlayerTextDrawBackgroundColor(playerid, txtPlayerSprite, bgcolor);
    PlayerTextDrawTextSize(playerid, txtPlayerSprite, width, height);
    PlayerTextDrawSetPreviewModel(playerid, txtPlayerSprite, modelindex);
    PlayerTextDrawSetPreviewRot(playerid, txtPlayerSprite, Xrot, Yrot, Zrot, mZoom);
    PlayerTextDrawSetSelectable(playerid, txtPlayerSprite, 1);
    PlayerTextDrawShow(playerid, txtPlayerSprite);
    return txtPlayerSprite;
}


/// <summary>
/// 
/// </summary>
/// <param name="playerid"></param>
/// <returns></returns>
stock mS_DestroyPlayerMPs(playerid) {
    static x = 0;
    while (x != mS_SELECTION_ITEMS) {
        if (gSelectionItems[playerid][x] != PlayerText:INVALID_TEXT_DRAW) {
            PlayerTextDrawDestroy(playerid, gSelectionItems[playerid][x]);
            gSelectionItems[playerid][x] = PlayerText:INVALID_TEXT_DRAW;
        }
        x++;
    }
}


/// <summary>
/// 
/// </summary>
/// <param name="playerid"></param>
/// <returns></returns>
stock mS_ShowPlayerMPs(playerid) {
    new bgcolor = GetPVarInt(playerid, "mS_previewBGcolor");
    static x = 0;
    static Float:BaseX = mS_DIALOG_BASE_X;
    new Float:BaseY = mS_DIALOG_BASE_Y - (mS_SPRITE_DIM_Y * 0.33);
    static linetracker = 0;

    new mS_listID = mS_GetPlayerCurrentListID(playerid);
    if (mS_listID == mS_CUSTOM_LISTID) {
        new itemat = (GetPVarInt(playerid, "mS_list_page") * mS_SELECTION_ITEMS);
        static Float:rotzoom[4];
        rotzoom[0] = GetPVarFloat(playerid, "mS_custom_Xrot");
        rotzoom[1] = GetPVarFloat(playerid, "mS_custom_Yrot");
        rotzoom[2] = GetPVarFloat(playerid, "mS_custom_Zrot");
        rotzoom[3] = GetPVarFloat(playerid, "mS_custom_Zoom");
        new itemamount = mS_GetAmountOfListItemsEx(playerid);
        mS_DestroyPlayerMPs(playerid);

        while (x != mS_SELECTION_ITEMS && itemat < (itemamount)) {
            if (linetracker == 0) {
                BaseX = mS_DIALOG_BASE_X + 25.0;
                BaseY += mS_SPRITE_DIM_Y + 1.0;
            }
            gSelectionItems[playerid][x] = mS_CreateMPTextDraw(playerid, gCustomList[playerid][itemat], BaseX, BaseY, rotzoom[0], rotzoom[1], rotzoom[2], rotzoom[3], mS_SPRITE_DIM_X, mS_SPRITE_DIM_Y, bgcolor);
            gSelectionItemsTag[playerid][x] = gCustomList[playerid][itemat];
            BaseX += mS_SPRITE_DIM_X + 1.0;
            linetracker++;
            if (linetracker == mS_ITEMS_PER_LINE) linetracker = 0;
            itemat++;
            x++;
        }
    } else {
        new itemat = (gLists[mS_listID][mS_LIST_START] + (GetPVarInt(playerid, "mS_list_page") * mS_SELECTION_ITEMS));

        mS_DestroyPlayerMPs(playerid);

        while (x != mS_SELECTION_ITEMS && itemat < (gLists[mS_listID][mS_LIST_END] + 1)) {
            if (linetracker == 0) {
                BaseX = mS_DIALOG_BASE_X + 25.0;
                BaseY += mS_SPRITE_DIM_Y + 1.0;
            }
            new rzID = gItemList[itemat][mS_ITEM_ROT_ZOOM_ID];
            if (rzID > -1) gSelectionItems[playerid][x] = mS_CreateMPTextDraw(playerid, gItemList[itemat][mS_ITEM_MODEL], BaseX, BaseY, gRotZoom[rzID][0], gRotZoom[rzID][1], gRotZoom[rzID][2], gRotZoom[rzID][3], mS_SPRITE_DIM_X, mS_SPRITE_DIM_Y, bgcolor);
            else gSelectionItems[playerid][x] = mS_CreateMPTextDraw(playerid, gItemList[itemat][mS_ITEM_MODEL], BaseX, BaseY, 0.0, 0.0, 0.0, 1.0, mS_SPRITE_DIM_X, mS_SPRITE_DIM_Y, bgcolor);
            gSelectionItemsTag[playerid][x] = gItemList[itemat][mS_ITEM_MODEL];
            BaseX += mS_SPRITE_DIM_X + 1.0;
            linetracker++;
            if (linetracker == mS_ITEMS_PER_LINE) linetracker = 0;
            itemat++;
            x++;
        }
    }
}


/// <summary>
/// 
/// </summary>
/// <param name="playerid"></param>
/// <returns></returns>
stock mS_UpdatePageTextDraw(playerid) {
    static PageText[64 + 1];
    new listID = mS_GetPlayerCurrentListID(playerid);
    if (listID == mS_CUSTOM_LISTID) {
        format(PageText, 64, "%d/%d", GetPVarInt(playerid, "mS_list_page") + 1, mS_GetNumberOfPagesEx(playerid));
        PlayerTextDrawSetString(playerid, gCurrentPageTextDrawId[playerid], PageText);
    } else {
        format(PageText, 64, "%d/%d", GetPVarInt(playerid, "mS_list_page") + 1, mS_GetNumberOfPages(listID));
        PlayerTextDrawSetString(playerid, gCurrentPageTextDrawId[playerid], PageText);
    }
}


/// <summary>
/// Shows a preview model selection menu for a player.
/// </summary>
/// <param name="playerid">The ID of the player for whom to show the selection menu</param>
/// <param name="ListID">The ID of the menu to be shown</param>
/// <param name="const header_text[]">The heading text of the selection menu</param>
/// <param name="dialogBGcolor = 0x4A5A6BBB">The background color of the menu (default is 0x4A5A6BBB)</param>
/// <param name="previewBGcolor = 0x88888899">The background color of the preview model (default is 0x88888899)</param>
/// <param name="tdSelectionColor = 0xFFFF00AA">The hover color of the items (default is 0xFFFF00AA)</param>
/// <returns></returns>
stock ShowModelSelectionMenu(playerid, ListID, const header_text[], dialogBGcolor = 0x4A5A6BBB, previewBGcolor = 0x88888899, tdSelectionColor = 0xFFFF00AA) {
    if (!(0 <= ListID < mS_TOTAL_LISTS && gLists[ListID][mS_LIST_START] != gLists[ListID][mS_LIST_END])) return 0;
    mS_DestroySelectionMenu(playerid);
    SetPVarInt(playerid, "mS_list_page", 0);
    SetPVarInt(playerid, "mS_list_id", ListID);
    SetPVarInt(playerid, "mS_list_active", 1);
    SetPVarInt(playerid, "mS_list_time", GetTickCount());

    gBackgroundTextDrawId[playerid] = mS_CreatePlayerBGTextDraw(playerid, mS_DIALOG_BASE_X, mS_DIALOG_BASE_Y + 20.0, mS_DIALOG_WIDTH, mS_DIALOG_HEIGHT, dialogBGcolor);
    gHeaderTextDrawId[playerid] = mS_CreatePlayerHeaderTextDraw(playerid, mS_DIALOG_BASE_X, mS_DIALOG_BASE_Y, header_text);
    gCurrentPageTextDrawId[playerid] = mS_CreateCurrentPageTextDraw(playerid, mS_DIALOG_WIDTH - 30.0, mS_DIALOG_BASE_Y + 15.0);
    gNextButtonTextDrawId[playerid] = mS_CreatePlayerDialogButton(playerid, mS_DIALOG_WIDTH - 30.0, mS_DIALOG_BASE_Y + mS_DIALOG_HEIGHT + 100.0, 50.0, 16.0, mS_NEXT_TEXT);
    gPrevButtonTextDrawId[playerid] = mS_CreatePlayerDialogButton(playerid, mS_DIALOG_WIDTH - 90.0, mS_DIALOG_BASE_Y + mS_DIALOG_HEIGHT + 100.0, 50.0, 16.0, mS_PREV_TEXT);
    gCancelButtonTextDrawId[playerid] = mS_CreatePlayerDialogButton(playerid, mS_DIALOG_WIDTH - 150.0, mS_DIALOG_BASE_Y + mS_DIALOG_HEIGHT + 100.0, 50.0, 16.0, mS_CANCEL_TEXT);

    SetPVarInt(playerid, "mS_previewBGcolor", previewBGcolor);
    mS_ShowPlayerMPs(playerid);
    mS_UpdatePageTextDraw(playerid);

    SelectTextDraw(playerid, tdSelectionColor);
    return 1;
}

/// <summary>
/// Shows a preview model selection menu for a player with extra parameters.
/// </summary>
/// <param name="playerid">The ID of the player for whom to show the selection menu.</param>
/// <param name="const items_array[]">The array of items/model IDs.</param>
/// <param name="item_amount">The number of items to be shown to the player.</param>
/// <param name="const header_text[]">The heading text of the selection menu.</param>
/// <param name="extraid">extraid which gets passed to OnPlayerModelSelectionEx after selection</param>
/// <param name="Float:Xrot = 0.0">The X rotation of the preview model.</param>
/// <param name="Float:Yrot = 0.0">The Y rotation of the preview model.</param>
/// <param name="Float:Zrot = 0.0">The Z rotation of the preview model.</param>
/// <param name="Float:mZoom = 1.0">The zoom of the preview model (default is 1.0).</param>
/// <param name="dialogBGcolor = 0x4A5A6BBB">The background color of the menu (default is 0x4A5A6BBB).</param>
/// <param name="previewBGcolor = 0x88888899">The background color of the preview model (default is 0x88888899).</param>
/// <param name="tdSelectionColor = 0xFFFF00AA">The hover color of the items (default is 0xFFFF00AA).</param>
/// <returns></returns>
stock ShowModelSelectionMenuEx(playerid, const items_array[], item_amount, const header_text[], extraid, Float:Xrot = 0.0, Float:Yrot = 0.0, Float:Zrot = 0.0, Float:mZoom = 1.0, dialogBGcolor = 0x4A5A6BBB, previewBGcolor = 0x88888899, tdSelectionColor = 0xFFFF00AA) {
    mS_DestroySelectionMenu(playerid);
    if (item_amount > mS_CUSTOM_MAX_ITEMS) {
        item_amount = mS_CUSTOM_MAX_ITEMS;
        print("-mSelection- WARNING: Too many items given to \"ShowModelSelectionMenuEx\", increase \"mS_CUSTOM_MAX_ITEMS\" to fix this");
    }
    if (item_amount > 0) {
        for (static i = 0; i < item_amount; i++) {
            gCustomList[playerid][i] = items_array[i];
        }
        SetPVarInt(playerid, "mS_list_page", 0);
        SetPVarInt(playerid, "mS_list_id", mS_CUSTOM_LISTID);
        SetPVarInt(playerid, "mS_list_active", 1);
        SetPVarInt(playerid, "mS_list_time", GetTickCount());

        SetPVarInt(playerid, "mS_custom_item_amount", item_amount);
        SetPVarFloat(playerid, "mS_custom_Xrot", Xrot);
        SetPVarFloat(playerid, "mS_custom_Yrot", Yrot);
        SetPVarFloat(playerid, "mS_custom_Zrot", Zrot);
        SetPVarFloat(playerid, "mS_custom_Zoom", mZoom);
        SetPVarInt(playerid, "mS_custom_extraid", extraid);


        gBackgroundTextDrawId[playerid] = mS_CreatePlayerBGTextDraw(playerid, mS_DIALOG_BASE_X, mS_DIALOG_BASE_Y + 20.0, mS_DIALOG_WIDTH, mS_DIALOG_HEIGHT, dialogBGcolor);
        gHeaderTextDrawId[playerid] = mS_CreatePlayerHeaderTextDraw(playerid, mS_DIALOG_BASE_X, mS_DIALOG_BASE_Y, header_text);
        gCurrentPageTextDrawId[playerid] = mS_CreateCurrentPageTextDraw(playerid, mS_DIALOG_WIDTH - 30.0, mS_DIALOG_BASE_Y + 15.0);
        gNextButtonTextDrawId[playerid] = mS_CreatePlayerDialogButton(playerid, mS_DIALOG_WIDTH - 30.0, mS_DIALOG_BASE_Y + mS_DIALOG_HEIGHT + 100.0, 50.0, 16.0, mS_NEXT_TEXT);
        gPrevButtonTextDrawId[playerid] = mS_CreatePlayerDialogButton(playerid, mS_DIALOG_WIDTH - 90.0, mS_DIALOG_BASE_Y + mS_DIALOG_HEIGHT + 100.0, 50.0, 16.0, mS_PREV_TEXT);
        gCancelButtonTextDrawId[playerid] = mS_CreatePlayerDialogButton(playerid, mS_DIALOG_WIDTH - 150.0, mS_DIALOG_BASE_Y + mS_DIALOG_HEIGHT + 100.0, 50.0, 16.0, mS_CANCEL_TEXT);

        SetPVarInt(playerid, "mS_previewBGcolor", previewBGcolor);
        mS_ShowPlayerMPs(playerid);
        mS_UpdatePageTextDraw(playerid);

        SelectTextDraw(playerid, tdSelectionColor);
        return 1;
    }
    return 0;
}

/// <summary>
/// Hides the selection menu for a specific player.
/// </summary>
/// <param name="playerid">The ID of the player for whom to hide the selection menu.</param>
/// <returns></returns>
stock HideModelSelectionMenu(playerid) {
    mS_DestroySelectionMenu(playerid);
    SetPVarInt(playerid, "mS_ignore_next_esc", 1);
    CancelSelectTextDraw(playerid);
    return 1;
}

/// <summary>
/// Destroys the selection menu.
/// </summary>
/// <param name="playerid"></param>
/// <returns></returns>
stock mS_DestroySelectionMenu(playerid) {
    if (GetPVarInt(playerid, "mS_list_active") == 1) {
        if (mS_GetPlayerCurrentListID(playerid) == mS_CUSTOM_LISTID) {
            DeletePVar(playerid, "mS_custom_Xrot");
            DeletePVar(playerid, "mS_custom_Yrot");
            DeletePVar(playerid, "mS_custom_Zrot");
            DeletePVar(playerid, "mS_custom_Zoom");
            DeletePVar(playerid, "mS_custom_extraid");
            DeletePVar(playerid, "mS_custom_item_amount");
        }
        DeletePVar(playerid, "mS_list_time");
        SetPVarInt(playerid, "mS_list_active", 0);
        mS_DestroyPlayerMPs(playerid);

        PlayerTextDrawDestroy(playerid, gHeaderTextDrawId[playerid]);
        PlayerTextDrawDestroy(playerid, gBackgroundTextDrawId[playerid]);
        PlayerTextDrawDestroy(playerid, gCurrentPageTextDrawId[playerid]);
        PlayerTextDrawDestroy(playerid, gNextButtonTextDrawId[playerid]);
        PlayerTextDrawDestroy(playerid, gPrevButtonTextDrawId[playerid]);
        PlayerTextDrawDestroy(playerid, gCancelButtonTextDrawId[playerid]);

        gHeaderTextDrawId[playerid] = PlayerText:INVALID_TEXT_DRAW;
        gBackgroundTextDrawId[playerid] = PlayerText:INVALID_TEXT_DRAW;
        gCurrentPageTextDrawId[playerid] = PlayerText:INVALID_TEXT_DRAW;
        gNextButtonTextDrawId[playerid] = PlayerText:INVALID_TEXT_DRAW;
        gPrevButtonTextDrawId[playerid] = PlayerText:INVALID_TEXT_DRAW;
        gCancelButtonTextDrawId[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }
}

// <library>a_samp</library>
/// <summary>
/// This callback is called when a player connects to the server.
/// </summary>
/// <param name="playerid">The ID of the player that connected</param>
/// <returns>
///   <b><c>0</c></b> - Will prevent other filterscripts from receiving this callback.<br />
///   <b><c>1</c></b> - Indicates that this callback will be passed to the next filterscript.<br />
///   It is always called first in filterscripts.
/// </returns>
public OnPlayerConnect(playerid) {
    gHeaderTextDrawId[playerid] = PlayerText:INVALID_TEXT_DRAW;
    gBackgroundTextDrawId[playerid] = PlayerText:INVALID_TEXT_DRAW;
    gCurrentPageTextDrawId[playerid] = PlayerText:INVALID_TEXT_DRAW;
    gNextButtonTextDrawId[playerid] = PlayerText:INVALID_TEXT_DRAW;
    gPrevButtonTextDrawId[playerid] = PlayerText:INVALID_TEXT_DRAW;
    gCancelButtonTextDrawId[playerid] = PlayerText:INVALID_TEXT_DRAW;

    for (new x = 0; x < mS_SELECTION_ITEMS; x++) {
        gSelectionItems[playerid][x] = PlayerText:INVALID_TEXT_DRAW;
    }

    gItemAt[playerid] = 0;

    return CallLocalFunction("MP_OPC", "i", playerid);
}

#if defined _ALS_OnPlayerConnect
	#undef OnPlayerConnect
#else
	#define _ALS_OnPlayerConnect
#endif
#define OnPlayerConnect MP_OPC

forward MP_OPC(playerid);

// <library>a_samp</library>
/// <summary>
/// This callback is called when a player clicks on a textdraw or cancels the select mode with
/// the Escape (ESC) key.
/// </summary>
/// <param name="playerid">The ID of the player that clicked on the textdraw</param>
/// <param name="Text:clickedid">The ID of the clicked textdraw. <b><c>INVALID_TEXT_DRAW</c></b> if selection
/// was cancelled</param>
/// <returns></returns>
public OnPlayerClickTextDraw(playerid, Text:clickedid) {
    if (GetPVarInt(playerid, "mS_ignore_next_esc") == 1) {
        SetPVarInt(playerid, "mS_ignore_next_esc", 0);
        return CallLocalFunction("MP_OPCTD", "ii", playerid, _:clickedid);
    }
    if (GetPVarInt(playerid, "mS_list_active") == 0) return CallLocalFunction("MP_OPCTD", "ii", playerid, _:clickedid);

    if (clickedid == Text:INVALID_TEXT_DRAW) {
        new listid = mS_GetPlayerCurrentListID(playerid);
        if (listid == mS_CUSTOM_LISTID) {
            new extraid = GetPVarInt(playerid, "mS_custom_extraid");
            mS_DestroySelectionMenu(playerid);
            CallLocalFunction("OnPlayerModelSelectionEx", "dddd", playerid, 0, extraid, -1);
            PlayerPlaySound(playerid, 1085, 0.0, 0.0, 0.0);
        } else {
            mS_DestroySelectionMenu(playerid);
            CallLocalFunction("OnPlayerModelSelection", "dddd", playerid, 0, listid, -1);
            PlayerPlaySound(playerid, 1085, 0.0, 0.0, 0.0);
        }
        return 1;
    }

    return CallLocalFunction("MP_OPCTD", "ii", playerid, _:clickedid);
}

#if defined _ALS_OnPlayerClickTextDraw
	#undef OnPlayerClickTextDraw
#else
	#define _ALS_OnPlayerClickTextDraw
#endif
#define OnPlayerClickTextDraw MP_OPCTD

forward MP_OPCTD(playerid, Text:clickedid);

// <library>a_samp</library>
/// <summary>
/// Called whenever a player clicks a clickable textdraw.
/// </summary>
/// <param name="playerid">The ID of the player who clicked the textdraw.</param>
/// <param name="PlayerText:playertextid">The ID of the clicked textdraw.</param>
/// <returns>
///    Returning <b><c>1</c></b> in this callback will prevent it from being called in other scripts. This
///    should be used to signal that the textdraw on which they clicked was 'found' and no further processing
///    is needed. You should return <b><c>0</c></b> if the textdraw on which they clicked wasn't found,
///    just like in <a href="#OnPlayerCommandText">OnPlayerCommandText</a>.<br />
///    It is always called first in filterscripts, so returning <b><c>1</c></b> there also blocks other
///    scripts from seeing it.
/// </returns>
public OnPlayerClickPlayerTextDraw(playerid, PlayerText:playertextid) {
    if (GetPVarInt(playerid, "mS_list_active") == 0 || (GetTickCount() - GetPVarInt(playerid, "mS_list_time")) < 200 /* Disable instant selection */) return CallLocalFunction("MP_OPCPTD", "ii", playerid, _:playertextid);

    new curpage = GetPVarInt(playerid, "mS_list_page");
    if (playertextid == gCancelButtonTextDrawId[playerid]) {
        new listID = mS_GetPlayerCurrentListID(playerid);
        if (listID == mS_CUSTOM_LISTID) {
            new extraid = GetPVarInt(playerid, "mS_custom_extraid");
            HideModelSelectionMenu(playerid);
            CallLocalFunction("OnPlayerModelSelectionEx", "dddd", playerid, 0, extraid, -1);
            PlayerPlaySound(playerid, 1085, 0.0, 0.0, 0.0);
        } else {
            HideModelSelectionMenu(playerid);
            CallLocalFunction("OnPlayerModelSelection", "dddd", playerid, 0, listID, -1);
            PlayerPlaySound(playerid, 1085, 0.0, 0.0, 0.0);
        }
        return 1;
    }

    if (playertextid == gNextButtonTextDrawId[playerid]) {
        new listID = mS_GetPlayerCurrentListID(playerid);
        if (listID == mS_CUSTOM_LISTID) {
            if (curpage < (mS_GetNumberOfPagesEx(playerid) - 1)) {
                SetPVarInt(playerid, "mS_list_page", curpage + 1);
                mS_ShowPlayerMPs(playerid);
                mS_UpdatePageTextDraw(playerid);
                PlayerPlaySound(playerid, 1083, 0.0, 0.0, 0.0);
            } else {
                PlayerPlaySound(playerid, 1085, 0.0, 0.0, 0.0);
            }
        } else {
            if (curpage < (mS_GetNumberOfPages(listID) - 1)) {
                SetPVarInt(playerid, "mS_list_page", curpage + 1);
                mS_ShowPlayerMPs(playerid);
                mS_UpdatePageTextDraw(playerid);
                PlayerPlaySound(playerid, 1083, 0.0, 0.0, 0.0);
            } else {
                PlayerPlaySound(playerid, 1085, 0.0, 0.0, 0.0);
            }
        }
        return 1;
    }

    if (playertextid == gPrevButtonTextDrawId[playerid]) {
        if (curpage > 0) {
            SetPVarInt(playerid, "mS_list_page", curpage - 1);
            mS_ShowPlayerMPs(playerid);
            mS_UpdatePageTextDraw(playerid);
            PlayerPlaySound(playerid, 1084, 0.0, 0.0, 0.0);
        } else {
            PlayerPlaySound(playerid, 1085, 0.0, 0.0, 0.0);
        }
        return 1;
    }

    static x = 0;
    while (x != mS_SELECTION_ITEMS) {
        if (playertextid == gSelectionItems[playerid][x]) {
            new listID = mS_GetPlayerCurrentListID(playerid);
            if (listID == mS_CUSTOM_LISTID) {
                PlayerPlaySound(playerid, 1083, 0.0, 0.0, 0.0);
                new item_id = gSelectionItemsTag[playerid][x], extraid = GetPVarInt(playerid, "mS_custom_extraid");
                HideModelSelectionMenu(playerid);
                CallLocalFunction("OnPlayerModelSelectionEx", "dddd", playerid, 1, extraid, item_id);
                return 1;
            }
            else {
                PlayerPlaySound(playerid, 1083, 0.0, 0.0, 0.0);
                new item_id = gSelectionItemsTag[playerid][x];
                HideModelSelectionMenu(playerid);
                CallLocalFunction("OnPlayerModelSelection", "dddd", playerid, 1, listID, item_id);
                return 1;
            }
        }
        x++;
    }

    return CallLocalFunction("MP_OPCPTD", "ii", playerid, _:playertextid);
}

#if defined _ALS_OnPlayerClickPlayerTD
	#undef OnPlayerClickPlayerTextDraw
#else
	#define _ALS_OnPlayerClickPlayerTD
#endif
#define OnPlayerClickPlayerTextDraw MP_OPCPTD

forward MP_OPCPTD(playerid, PlayerText:playertextid);


/// <summary>
/// Loads a menu with model previews from a file.
/// </summary>
/// <param name="f_name">The path of the file in the scriptfiles folder.</param>
/// <returns>
///   <b><c>1</c></b>: The function executed successfully.<br />
///   <b><c>0</c></b>: The function failed to execute. The specified file does not exist.
/// </returns>
stock LoadModelSelectionMenu(const f_name[]) {
    static File:f, str[75];
    format(str, sizeof(str), "%s", f_name);
    f = fopen(str, io_read);
    if (!f) {
        printf("-mSelection- WARNING: Failed to load list: \"%s\"", f_name);
        return mS_INVALID_LISTID;
    }

    if (gListAmount >= mS_TOTAL_LISTS) {
        printf("-mSelection- WARNING: Reached maximum amount of lists, increase \"mS_TOTAL_LISTS\"", f_name);
        return mS_INVALID_LISTID;
    }
    static tmp_ItemAmount = gItemAmount;


    static line[128], idxx;
    while (fread(f, line, sizeof(line), false)) {
        if (tmp_ItemAmount >= mS_TOTAL_ITEMS) {
            printf("-mSelection- WARNING: Reached maximum amount of items, increase \"mS_TOTAL_ITEMS\"", f_name);
            break;
        }
        idxx = 0;
        if (!line[0]) continue;
        static mID = strval(mS_strtok(line, idxx));
        if (0 <= mID < 20000) {
            gItemList[tmp_ItemAmount][mS_ITEM_MODEL] = mID;

            static tmp_mS_strtok[20];
            static Float:mRotation[3], Float:mZoom = 1.0;
            static bool:useRotation = false;

            tmp_mS_strtok = mS_strtok(line, idxx);
            if (tmp_mS_strtok[0]) {
                useRotation = true;
                mRotation[0] = floatstr(tmp_mS_strtok);
            }
            tmp_mS_strtok = mS_strtok(line, idxx);
            if (tmp_mS_strtok[0]) {
                useRotation = true;
                mRotation[1] = floatstr(tmp_mS_strtok);
            }
            tmp_mS_strtok = mS_strtok(line, idxx);
            if (tmp_mS_strtok[0]) {
                useRotation = true;
                mRotation[2] = floatstr(tmp_mS_strtok);
            }
            tmp_mS_strtok = mS_strtok(line, idxx);
            if (tmp_mS_strtok[0]) {
                useRotation = true;
                mZoom = floatstr(tmp_mS_strtok);
            }
            if (useRotation) {
                static bool:foundRotZoom = false;
                for (static i = 0; i < gRotZoomAmount; i++) {
                    if (gRotZoom[i][0] == mRotation[0] && gRotZoom[i][1] == mRotation[1] && gRotZoom[i][2] == mRotation[2] && gRotZoom[i][3] == mZoom) {
                        foundRotZoom = true;
                        gItemList[tmp_ItemAmount][mS_ITEM_ROT_ZOOM_ID] = i;
                        break;
                    }
                }
                if (gRotZoomAmount < mS_TOTAL_ROT_ZOOM) {
                    if (!foundRotZoom) {
                        gRotZoom[gRotZoomAmount][0] = mRotation[0];
                        gRotZoom[gRotZoomAmount][1] = mRotation[1];
                        gRotZoom[gRotZoomAmount][2] = mRotation[2];
                        gRotZoom[gRotZoomAmount][3] = mZoom;
                        gItemList[tmp_ItemAmount][mS_ITEM_ROT_ZOOM_ID] = gRotZoomAmount;
                        gRotZoomAmount++;
                    }
                }
                else print("-mSelection- WARNING: Not able to save rotation/zoom information. Reached maximum rotation/zoom information count. Increase '#define mS_TOTAL_ROT_ZOOM' to fix the issue");
            }
            else gItemList[tmp_ItemAmount][mS_ITEM_ROT_ZOOM_ID] = -1;
            tmp_ItemAmount++;
        }
    }
    if (tmp_ItemAmount > gItemAmount) {
        gLists[gListAmount][mS_LIST_START] = gItemAmount;
        gItemAmount = tmp_ItemAmount;
        gLists[gListAmount][mS_LIST_END] = (gItemAmount - 1);

        gListAmount++;
        return (gListAmount - 1);
    }
    printf("-mSelection- WARNING: No Items found in file: %s", f_name);
    return mS_INVALID_LISTID;
}



/// <summary>
/// 
/// </summary>
/// <param name="const string[]"></param>
/// <param name="&index"></param>
/// <returns></returns>
stock mS_strtok(const string[], &index) {
    static length = strlen(string);
    while ((index < length) && (string[index] <= ' ')) {
        index++;
    }

    static offset = index;
    static result[20];
    while ((index < length) && (string[index] > ' ') && ((index - offset) < (sizeof(result) - 1))) {
        result[index - offset] = string[index];
        index++;
    }
    result[index - offset] = EOS;
    return result;
}
