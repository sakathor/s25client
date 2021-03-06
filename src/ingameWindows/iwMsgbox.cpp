// Copyright (c) 2005 - 2015 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#include "defines.h" // IWYU pragma: keep
#include "iwMsgbox.h"

#include "drivers/VideoDriverWrapper.h"
#include "Loader.h"
#include "ogl/glArchivItem_Font.h"
#include "controls/ctrlImage.h"
#include "gameData/const_gui_ids.h"
#include "controls/ctrlMultiline.h"

namespace{
    enum IDS{
        ID_ICON = 0,
        ID_TEXT,
        ID_BT_0
    };
    const unsigned short btHeight = 20;
    const unsigned short paddingX = 15; /// Padding in X/to image
    const unsigned short minTextWidth = 150;
    const unsigned short maxTextHeight = 200;
}

iwMsgbox::iwMsgbox(const std::string& title, const std::string& text, Window* msgHandler, MsgboxButton button, MsgboxIcon icon, unsigned msgboxid)
    : IngameWindow(CGI_MSGBOX, IngameWindow::posLastOrCenter, 420, 140, title, LOADER.GetImageN("resource", 41), true, false), button(button), msgboxid(msgboxid), msgHandler_(msgHandler)
{
    Init(text, "io", icon);
}

iwMsgbox::iwMsgbox(const std::string& title, const std::string& text, Window* msgHandler, MsgboxButton button, const std::string& iconFile, unsigned iconIdx, unsigned msgboxid /* = 0 */)
    : IngameWindow(CGI_MSGBOX, IngameWindow::posLastOrCenter, 420, 140, title, LOADER.GetImageN("resource", 41), true, false), button(button), msgboxid(msgboxid), msgHandler_(msgHandler)
{
    Init(text, iconFile, iconIdx);
}

void iwMsgbox::Init(const std::string& text, const std::string& iconFile, unsigned iconIdx)
{
    glArchivItem_Bitmap* icon = LOADER.GetImageN(iconFile, iconIdx);
    if(icon)
        AddImage(ID_ICON, contentOffset.x + 30, contentOffset.y + 20, icon);
    unsigned short textX = icon ? icon->getWidth() - icon->getNx() + GetCtrl<Window>(ID_ICON)->GetX(false) + paddingX : contentOffset.x + paddingX;
    ctrlMultiline* multiline = AddMultiline(ID_TEXT, textX, contentOffset.y + 5, std::max<int>(minTextWidth, GetIwRightBoundary() - textX - paddingX), maxTextHeight, TC_GREEN2, NormalFont);
    multiline->ShowBackground(false);
    multiline->AddString(text, COLOR_YELLOW);
    multiline->Resize(multiline->GetContentWidth(), multiline->GetContentHeight());
    // Increase window size if required
    SetIwWidth(std::max<unsigned short>(multiline->GetX(false) + multiline->GetWidth() + paddingX, GetIwWidth()));
    SetIwHeight(std::max<unsigned short>(multiline->GetY(false) + multiline->GetHeight() + 10 + btHeight * 2, GetIwHeight())); // 10 padding, button/button padding


    unsigned defaultBt = 0;
    // Buttons erstellen
    switch(button)
    {
    case MSB_OK:
        AddButton(0, width_ / 2 - 45, _("OK"), TC_GREEN2);
        defaultBt = 0;
        break;

    case MSB_OKCANCEL:
        AddButton(0, width_ / 2 - 3 - 90, _("OK"), TC_GREEN2);
        AddButton(1, width_ / 2 + 3, _("Cancel"), TC_RED1);
        defaultBt = 1;
        break;

    case MSB_YESNO:
        AddButton(0, width_ / 2 - 3 - 90, _("Yes"), TC_GREEN2);
        AddButton(1, width_ / 2 + 3, _("No"), TC_RED1);
        defaultBt = 1;
        break;

    case MSB_YESNOCANCEL:
        AddButton(0, width_ / 2 - 45 - 6 - 90, _("Yes"), TC_GREEN2);
        AddButton(1, width_ / 2 - 45, _("No"), TC_RED1);
        AddButton(2, width_ / 2 + 45 + 6, _("Cancel"), TC_GREY);
        defaultBt = 2;
        break;
    }
    const Window* defBt = GetCtrl<Window>(defaultBt + ID_BT_0);
    if(defBt)
        VIDEODRIVER.SetMousePos(defBt->GetX() + defBt->GetWidth() / 2, defBt->GetY() + defBt->GetHeight() / 2);
}

iwMsgbox::~iwMsgbox()
{}


void iwMsgbox::MoveIcon(int x, int y)
{
    ctrlImage* icon = GetCtrl<ctrlImage>(ID_ICON);
    if(icon){
        icon->Move(std::max(0, x), std::max(0, y));
        DrawPoint iconPos(icon->GetX(false) - icon->GetImage()->getNx(), icon->GetY(false) - icon->GetImage()->getNy());
        DrawPoint textPos, textMaxSize;
        if(iconPos.x < 100){
            // icon left
            textPos.x = iconPos.x + icon->GetImage()->getWidth() + paddingX;
            textPos.y = contentOffset.y + 5;
            textMaxSize.x = std::max<int>(minTextWidth, 400 - textPos.x - paddingX);
            textMaxSize.y = maxTextHeight;
        }else if(iconPos.x > 300){
            // icon right
            textPos.x = contentOffset.x + paddingX;
            textPos.y = contentOffset.y + 5;
            textMaxSize.x = iconPos.x - 2 * paddingX;
            textMaxSize.y = maxTextHeight;
        }else if(iconPos.y + icon->GetImage()->getHeight() < 50){
            // icon top
            textPos.x = contentOffset.x + paddingX;
            textPos.y = iconPos.y + icon->GetImage()->getHeight() + paddingX;
            textMaxSize.x = 400 - 2 * paddingX;
            textMaxSize.y = maxTextHeight;
        }else if(iconPos.y > 150){
            // icon bottom
            textPos.x = contentOffset.x + paddingX;
            textPos.y = contentOffset.y + 5;
            textMaxSize.x = 400 - 2 * paddingX;
            textMaxSize.y = iconPos.y - paddingX - textPos.y;
        }else{
            // Icon middle -> Overlay text
            textPos.x = contentOffset.x + paddingX;
            textPos.y = contentOffset.y + 5;
            textMaxSize.x = 400 - 2 * paddingX;
            textMaxSize.y = maxTextHeight;
        }
        ctrlMultiline* multiline = GetCtrl<ctrlMultiline>(ID_TEXT);
        multiline->Move(textPos);
        multiline->Resize(textMaxSize.x, textMaxSize.y);
        multiline->Resize(multiline->GetContentWidth(), multiline->GetContentHeight());

        int newW = std::max(iconPos.x + icon->GetImage()->getWidth(), multiline->GetX(false) + multiline->GetWidth() + paddingX) + contentOffsetEnd.x;
        int newH = std::max(iconPos.y + icon->GetImage()->getHeight(), multiline->GetY(false) + multiline->GetHeight() + paddingX) + 10 + btHeight * 2 + contentOffsetEnd.y;
        newW = std::max<int>(newW, GetWidth());
        newH = std::max<int>(newH, GetHeight());
        for(unsigned i = 0; i < 3; i++){
            Window* bt = GetCtrl<Window>(i + ID_BT_0);
            if(bt)
                bt->Move((newW - GetWidth()) / 2, newH - GetHeight(), false);
        }
        SetWidth(newW);
        SetHeight(newH);
    }
}

const MsgboxResult RET_IDS[MSB_YESNOCANCEL + 1][3] =
{
    {MSR_OK,  MSR_NOTHING, MSR_NOTHING},
    {MSR_OK,  MSR_CANCEL,  MSR_NOTHING},
    {MSR_YES, MSR_NO,      MSR_NOTHING},
    {MSR_YES, MSR_NO,      MSR_CANCEL}
};

void iwMsgbox::Msg_ButtonClick(const unsigned int ctrl_id)
{
    if(msgHandler_)
        msgHandler_->Msg_MsgBoxResult(msgboxid, RET_IDS[button][ctrl_id - ID_BT_0]);
    Close();
}

void iwMsgbox::AddButton(unsigned short id, int x, const std::string& text, const TextureColor tc)
{
    Window::AddTextButton(ID_BT_0 + id, x, GetIwBottomBoundary() - btHeight * 2, 90, btHeight, tc, text, NormalFont);
}
