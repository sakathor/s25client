// Copyright (c) 2005-2010 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Siedler II.5 RTTR.
//
// Siedler II.5 RTTR is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Siedler II.5 RTTR is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Siedler II.5 RTTR. If not, see <http://www.gnu.org/licenses/>.

#include "defines.h" // IWYU pragma: keep
#include "iwMerchandiseStatistics.h"
#include "GamePlayer.h"
#include "Loader.h"
#include "WindowManager.h"
#include "iwHelp.h"
#include "controls/ctrlMultiSelectGroup.h"
#include "controls/ctrlOptionGroup.h"
#include "controls/ctrlText.h"
#include "ogl/glArchivItem_Font.h"
#include "gameData/const_gui_ids.h"

// Farben für die einzelnen Balken
const unsigned int iwMerchandiseStatistics::BarColors[14] =
{
    0xFF00D3F7, // türkis
    0xFFFB9E49, // gelb
    0xFFDF6161, // orange
    0xFF71B63C, // hellgrün
    0xFFAE71CB, // rosa
    0xFF9A75BE, // pink
    0xFF9EA6AE, // grau
    0xFF1038A6, // blau
    0xFF714520, // braun
    0xFFAE0000, // rot
    0xFF045514, // dunkelgrün
    0xFF61245D, // dunkellila
    0xFF5D4175, // blasslila
    0xFF202420  // schwarz
};

iwMerchandiseStatistics::iwMerchandiseStatistics(const GamePlayer& player):
    IngameWindow(CGI_MERCHANDISE_STATISTICS, IngameWindow::posAtMouse,  252, 310, _("Merchandise"), LOADER.GetImageN("resource", 41)),
    player(player), currentTime(STAT_1H)
{
    // Statistikfeld
    AddImage(0, 10 + 115, 23 + 81, LOADER.GetImageN("io", 228));

    // Waren-Buttons
    // obere Reihe
    ctrlMultiSelectGroup* types = AddMultiSelectGroup(22, ctrlOptionGroup::ILLUMINATE);
    types->AddImageButton(1, 17, 192, 30, 30, TC_GREY, LOADER.GetMapImageN(2250 + GD_WOOD), _("Wood"));
    types->AddImageButton(2, 48, 192, 30, 30, TC_GREY, LOADER.GetMapImageN(2250 + GD_BOARDS), _("Boards"));
    types->AddImageButton(3, 79, 192, 30, 30, TC_GREY, LOADER.GetMapImageN(2250 + GD_STONES), _("Stones"));
    types->AddImageButton(4, 110, 192, 30, 30, TC_GREY, LOADER.GetImageN("io", 80), _("Food"));
    types->AddImageButton(5, 141, 192, 30, 30, TC_GREY, LOADER.GetMapImageN(2250 + GD_WATER), _("Water"));
    types->AddImageButton(6, 172, 192, 30, 30, TC_GREY, LOADER.GetMapImageN(2250 + GD_BEER), _("Beer"));
    types->AddImageButton(7, 203, 192, 30, 30, TC_GREY, LOADER.GetMapImageN(2250 + GD_COAL), _("Coal"));

    // untere Reihe
    types->AddImageButton(8, 17, 227, 30, 30, TC_GREY, LOADER.GetMapImageN(2250 + GD_IRONORE), _("Ironore"));
    types->AddImageButton(9, 48, 227, 30, 30, TC_GREY, LOADER.GetMapImageN(2250 + GD_GOLD), _("Gold"));
    types->AddImageButton(10, 79, 227, 30, 30, TC_GREY, LOADER.GetMapImageN(2250 + GD_IRON), _("Iron"));
    types->AddImageButton(11, 110, 227, 30, 30, TC_GREY, LOADER.GetMapImageN(2250 + GD_COINS), _("Coins"));
    types->AddImageButton(12, 141, 227, 30, 30, TC_GREY, LOADER.GetMapImageN(2250 + GD_HAMMER), _("Tools"));
    types->AddImageButton(13, 172, 227, 30, 30, TC_GREY, LOADER.GetImageN("io", 111), _("Weapons"));
    types->AddImageButton(14, 203, 227, 30, 30, TC_GREY, LOADER.GetMapImageN(2250 + GD_BOAT), _("Boats"));

    // Hilfe
    AddImageButton(16, 17, 261, 30, 32, TC_GREY, LOADER.GetImageN("io", 225), _("Help"));

    // Mülleimer
    AddImageButton(17, 49, 263, 30, 28, TC_GREY, LOADER.GetImageN("io", 106), _("Delete all"));

    // Zeiten
    ctrlOptionGroup* times = AddOptionGroup(23, ctrlOptionGroup::ILLUMINATE);
    times->AddTextButton(18, 81, 263, 36, 28, TC_GREY, _("15 m"), NormalFont);
    times->AddTextButton(19, 119, 263, 36, 28, TC_GREY, _("1 h"), NormalFont);
    times->AddTextButton(20, 155, 263, 36, 28, TC_GREY, _("4 h"), NormalFont);
    times->AddTextButton(21, 191, 263, 36, 28, TC_GREY, _("16 h"), NormalFont);
    times->SetSelection(19);


    // Zeit-Werte an der x-Achse
    timeAnnotations = std::vector<ctrlText*>(7);
    for (unsigned i = 0; i < 7; ++i)
    {
        timeAnnotations[i] = AddText(32 + i, 211 + i, 125 + i, "", MakeColor(255, 136, 96, 52),
                                     glArchivItem_Font::DF_CENTER | glArchivItem_Font::DF_TOP, LOADER.GetFontN("resource", 0));
    }

    // Aktueller Maximalwert an der y-Achse
    maxValue = AddText(31, 211, 55, "1", MakeColor(255, 136, 96, 52),
                       glArchivItem_Font::DF_RIGHT | glArchivItem_Font::DF_VCENTER, LOADER.GetFontN("resource", 0));
}

iwMerchandiseStatistics::~iwMerchandiseStatistics()
{

}

void iwMerchandiseStatistics::Msg_ButtonClick(const unsigned int ctrl_id)
{
    switch (ctrl_id)
    {
        case 16: // Hilfe
        {
            WINDOWMANAGER.Show(new iwHelp(GUI_ID(CGI_HELP),
                _("The merchandise statistics window allows you to check the quantities "
                  "of your merchandise. By clicking the left mouse button you can switch "
                  "the display of individual goods on and off. These can displayed over "
                  "four different time periods. To delete all displays, click on the wastebasket button.")));
        } break;
        case 17: // Alle abwählen
        {
            const std::set<unsigned short>& active = GetCtrl<ctrlMultiSelectGroup>(22)->GetSelection();
            for(std::set<unsigned short>::const_iterator it = active.begin(); it != active.end();)
            {
                std::set<unsigned short>::const_iterator curIt = it++;
                GetCtrl<ctrlMultiSelectGroup>(22)->RemoveSelection(*curIt);
            }
        } break;
    }
}

void iwMerchandiseStatistics::Msg_OptionGroupChange(const unsigned int ctrl_id, const int selection)
{
    switch(ctrl_id)
    {
        case 23: // Zeitbereich wählen
            switch(selection)
            {
                case 18:
                    currentTime = STAT_15M;
                    break;
                case 19:
                    currentTime = STAT_1H;
                    break;
                case 20:
                    currentTime = STAT_4H;
                    break;
                case 21:
                    currentTime = STAT_16H;
                    break;
            }
            break;
    }
}


void iwMerchandiseStatistics::Msg_PaintAfter()
{
    DrawRectangles();
    DrawAxis();
    DrawStatistic();
}

// TODO
void iwMerchandiseStatistics::DrawStatistic()
{
    // Ein paar benötigte Werte...
    const int sizeX = 180;
    const int sizeY = 80;
    const DrawPoint topLeft = pos_ + DrawPoint(34, 64);
    const int stepX = sizeX / STAT_STEP_COUNT; // 6

    // Aktive Buttons holen (Achtung ID == BarColor + 1)
    const std::set<unsigned short>& active = GetCtrl<ctrlMultiSelectGroup>(22)->GetSelection();

    // Statistik holen
    const GamePlayer::Statistic stat = player.GetStatistic(currentTime);


    // Maximalwert suchen
    unsigned short max = 1;
    for(std::set<unsigned short>::const_iterator it = active.begin(); it != active.end(); ++it)
    {
        for (unsigned int i = 0; i < STAT_STEP_COUNT; ++i)
        {
            if (max < stat.merchandiseData[(*it) - 1][i])
            {
                max = stat.merchandiseData[(*it) - 1][i];
            }
        }
    }

    // Maximalen Wert an die Achse schreiben
    std::stringstream ss;
    ss << max;
    maxValue->SetText(ss.str());

    DrawPoint previous(0, 0);
    unsigned short currentIndex = stat.currentIndex;

    for(std::set<unsigned short>::const_iterator it = active.begin(); it != active.end(); ++it)
    {
        // Testing only:
        //DrawLine(topLeft.x, topLeft.y + 3 * (*it), topLeft.x + sizeX, topLeft.y + 3 * (*it), 2, BarColors[(*it) - 1]);


        for (unsigned int i = 0; i < STAT_STEP_COUNT; ++i)
        {
            DrawPoint drawPos = topLeft;
            drawPos.x += (STAT_STEP_COUNT - i) * stepX;
            drawPos.y += sizeY - ((stat.merchandiseData[(*it) - 1][(currentIndex >= i) ? (currentIndex - i) : (STAT_STEP_COUNT - i + currentIndex)]) * sizeY) / max;
            if (i != 0)
            {
                DrawLine(drawPos.x, drawPos.y,
                         previous.x, previous.y, 2, BarColors[(*it) - 1]);
            }
            previous = drawPos;
        }



    }
}

// Lustige bunte Kästchen über den Buttons malen
void iwMerchandiseStatistics::DrawRectangles()
{
    const unsigned sizeX = 30;
    const unsigned sizeY = 4;
    const unsigned stepX = 31;
    const unsigned stepY = 35;

    const DrawPoint pos = GetDrawPos();
    DrawPoint curOffset(17, 187);

    for (unsigned i = 0; i < 14; ++i)
    {
        DrawRectangle(pos + curOffset, sizeX, sizeY, BarColors[i]);
        if (i == 6)
        {
            curOffset.x = 17;
            curOffset.y += stepY;
        } else
            curOffset.x += stepX;
    }
}

void iwMerchandiseStatistics::DrawAxis()
{
    // Ein paar benötigte Werte...
    const int sizeX = 180;
    const int sizeY = 80;
    const DrawPoint topLeft = pos_ + DrawPoint(34, 64);
    const DrawPoint topLeftRel(37, 64);

    // X-Achse, horizontal, war irgendwie zu lang links :S
    DrawLine(topLeft.x + 6, topLeft.y + sizeY + 2, // bisschen tiefer, damit man nulllinien noch sieht
             topLeft.x + sizeX, topLeft.y + sizeY + 1, 1, MakeColor(255, 88, 44, 16));

    // Y-Achse, vertikal
    DrawLine(topLeft.x + sizeX, topLeft.y,
             topLeft.x + sizeX, topLeft.y + sizeY + 5, 1, MakeColor(255, 88, 44, 16));

    // Striche an der Y-Achse
    DrawLine(topLeft.x + sizeX - 3, topLeft.y, topLeft.x + sizeX + 4, topLeft.y, 1, MakeColor(255, 88, 44, 16));
    DrawLine(topLeft.x + sizeX - 3, topLeft.y + sizeY / 2, topLeft.x + sizeX + 4, topLeft.y + sizeY / 2, 1, MakeColor(255, 88, 44, 16));

    // Striche an der X-Achse + Beschriftung
    // Zunächst die 0, die haben alle
    timeAnnotations[6]->Move(topLeftRel.x + 180, topLeftRel.y + sizeY + 6);
    timeAnnotations[6]->SetText("0");
    timeAnnotations[6]->SetVisible(true);

    switch(currentTime)
    {
        case STAT_15M:
            // -15
            DrawLine(topLeft.x + 6, topLeft.y + sizeY + 2,
                     topLeft.x + 6, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[0]->Move(topLeftRel.x + 6, topLeftRel.y + sizeY + 6);
            timeAnnotations[0]->SetText("-15");
            timeAnnotations[0]->SetVisible(true);

            // -12
            DrawLine(topLeft.x + 40, topLeft.y + sizeY + 2,
                     topLeft.x + 40, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[1]->Move(topLeftRel.x + 40, topLeftRel.y + sizeY + 6);
            timeAnnotations[1]->SetText("-12");
            timeAnnotations[1]->SetVisible(true);

            // -9
            DrawLine(topLeft.x + 75, topLeft.y + sizeY + 2,
                     topLeft.x + 75, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[2]->Move(topLeftRel.x + 75, topLeftRel.y + sizeY + 6);
            timeAnnotations[2]->SetText("-9");
            timeAnnotations[2]->SetVisible(true);

            // -6
            DrawLine(topLeft.x + 110, topLeft.y + sizeY + 2,
                     topLeft.x + 110, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[3]->Move(topLeftRel.x + 110, topLeftRel.y + sizeY + 6);
            timeAnnotations[3]->SetText("-6");
            timeAnnotations[3]->SetVisible(true);

            // -3
            DrawLine(topLeft.x + 145, topLeft.y + sizeY + 2,
                     topLeft.x + 145, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[4]->Move(topLeftRel.x + 145, topLeftRel.y + sizeY + 6);
            timeAnnotations[4]->SetText("-3");
            timeAnnotations[4]->SetVisible(true);

            timeAnnotations[5]->SetVisible(false);
            break;
        case STAT_1H:
            // -60
            DrawLine(topLeft.x + 6, topLeft.y + sizeY + 2,
                     topLeft.x + 6, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[0]->Move(topLeftRel.x + 6, topLeftRel.y + sizeY + 6);
            timeAnnotations[0]->SetText("-60");
            timeAnnotations[0]->SetVisible(true);

            // -50
            DrawLine(topLeft.x + 35, topLeft.y + sizeY + 2,
                     topLeft.x + 35, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[1]->Move(topLeftRel.x + 35, topLeftRel.y + sizeY + 6);
            timeAnnotations[1]->SetText("-50");
            timeAnnotations[1]->SetVisible(true);

            // -40
            DrawLine(topLeft.x + 64, topLeft.y + sizeY + 2,
                     topLeft.x + 64, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[2]->Move(topLeftRel.x + 64, topLeftRel.y + sizeY + 6);
            timeAnnotations[2]->SetText("-40");
            timeAnnotations[2]->SetVisible(true);

            // -30
            DrawLine(topLeft.x + 93, topLeft.y + sizeY + 2,
                     topLeft.x + 93, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[3]->Move(topLeftRel.x + 93, topLeftRel.y + sizeY + 6);
            timeAnnotations[3]->SetText("-30");
            timeAnnotations[3]->SetVisible(true);

            // -20
            DrawLine(topLeft.x + 122, topLeft.y + sizeY + 2,
                     topLeft.x + 122, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[4]->Move(topLeftRel.x + 122, topLeftRel.y + sizeY + 6);
            timeAnnotations[4]->SetText("-20");
            timeAnnotations[4]->SetVisible(true);

            // -10
            DrawLine(topLeft.x + 151, topLeft.y + sizeY + 2,
                     topLeft.x + 151, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[5]->Move(topLeftRel.x + 151, topLeftRel.y + sizeY + 6);
            timeAnnotations[5]->SetText("-10");
            timeAnnotations[5]->SetVisible(true);
            break;
        case STAT_4H:
            // -240
            DrawLine(topLeft.x + 6, topLeft.y + sizeY + 2,
                     topLeft.x + 6, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[0]->Move(topLeftRel.x + 6, topLeftRel.y + sizeY + 6);
            timeAnnotations[0]->SetText("-240");
            timeAnnotations[0]->SetVisible(true);

            // -180
            DrawLine(topLeft.x + 49, topLeft.y + sizeY + 2,
                     topLeft.x + 49, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[1]->Move(topLeftRel.x + 49, topLeftRel.y + sizeY + 6);
            timeAnnotations[1]->SetText("-180");
            timeAnnotations[1]->SetVisible(true);

            // -120
            DrawLine(topLeft.x + 93, topLeft.y + sizeY + 2,
                     topLeft.x + 93, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[2]->Move(topLeftRel.x + 93, topLeftRel.y + sizeY + 6);
            timeAnnotations[2]->SetText("-120");
            timeAnnotations[2]->SetVisible(true);

            // -60
            DrawLine(topLeft.x + 136, topLeft.y + sizeY + 2,
                     topLeft.x + 136, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[3]->Move(topLeftRel.x + 136, topLeftRel.y + sizeY + 6);
            timeAnnotations[3]->SetText("-60");
            timeAnnotations[3]->SetVisible(true);

            timeAnnotations[4]->SetVisible(false);
            timeAnnotations[5]->SetVisible(false);
            break;
        case STAT_16H:
            // -960
            DrawLine(topLeft.x + 6, topLeft.y + sizeY + 2,
                     topLeft.x + 6, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[0]->Move(topLeftRel.x + 6, topLeftRel.y + sizeY + 6);
            timeAnnotations[0]->SetText("-960");
            timeAnnotations[0]->SetVisible(true);

            // -720
            DrawLine(topLeft.x + 49, topLeft.y + sizeY + 2,
                     topLeft.x + 49, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[1]->Move(topLeftRel.x + 49, topLeftRel.y + sizeY + 6);
            timeAnnotations[1]->SetText("-720");
            timeAnnotations[1]->SetVisible(true);

            // -480
            DrawLine(topLeft.x + 93, topLeft.y + sizeY + 2,
                     topLeft.x + 93, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[2]->Move(topLeftRel.x + 93, topLeftRel.y + sizeY + 6);
            timeAnnotations[2]->SetText("-480");
            timeAnnotations[2]->SetVisible(true);

            // -240
            DrawLine(topLeft.x + 136, topLeft.y + sizeY + 2,
                     topLeft.x + 136, topLeft.y + sizeY + 4, 1, MakeColor(255, 88, 44, 16));
            timeAnnotations[3]->Move(topLeftRel.x + 136, topLeftRel.y + sizeY + 6);
            timeAnnotations[3]->SetText("-240");
            timeAnnotations[3]->SetVisible(true);

            timeAnnotations[4]->SetVisible(false);
            timeAnnotations[5]->SetVisible(false);
            break;
    }
}
