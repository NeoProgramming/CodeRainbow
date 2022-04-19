#pragma once

namespace CR {
namespace Constants {

const int  TOOLBAR_ICON_SIZE = 24;
const int  CR_UID = 8;
const char CR_HIGHLIGHT_ID[]   = "CR.Highlighting.Mark";

const char ID_CR_CLEAR[] = "CR.Clear";
const char ID_CR_OPEN[] = "CR.Open";
const char ID_CR_SAVE[] = "CR.Save";
const char ID_CR_SAVEAS[] = "CR.SaveAs";
const char ID_CR_GENPRJ[] = "CR.CreatePrj";
const char ID_CR_UPDPRJ[] = "CR.UpdatePrj";
const char ID_CR_GENDIR[] = "CR.CreateDir";
const char ID_CR_UPDDIR[] = "CR.UpdateDir";
const char ID_CR_EXPAND[] = "CR.ExpandTree";
const char ID_CR_OPTIONS[] = "CR.Options";
const char ID_CR_ABOUT[] = "CR.About";

const char MENU_ID[] = "CR.Menu";

const char PREV_TAB_ID[] = "CR.PrevTab";
const char NEXT_TAB_ID[] = "CR.NextTab";
const char CPP_SOURCE_FILES_REGEXP_PATTERN[]= ".*\\.(c|cc|cpp|h|hpp|dox)$";

const char OUTPUT_PANE_TITLE[] = "CR Output";
const char INFO_PANE_TITLE[] = "CR Info";
const char ICON_CRPLUGIN[] = ":/res/cr.png";

const char CORE_SETTINGS_GROUP[]      = "CRPlugin";
const char SETTING_SHOW_TABS[]      = "ShowTabs";
const char SETTING_SHOW_HL[]        = "Highlight";
const char SETTING_SPLIT_FILES[]    = "SplitByFiles";

} // namespace CR
} // namespace Constants
