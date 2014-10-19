#ifndef TOOLTIP_H
#define TOOLTIP_H

#ifndef GUICONSTANTS_H
#include "guiconstants.h"
#endif

#define _TOOLTIP_STYLE "QToolTip { color: white; background-color: " + STRING_VERIBLUE_LT + "; border: 1px solid #eeeeee; border-radius: 3px; padding: 4px; }"

#define _TOOLTIP_INIT_THIS  this->setStyleSheet(_TOOLTIP_STYLE);

#define _TOOLTIP_INIT_QAPP  qApp->setStyleSheet(_TOOLTIP_STYLE);

#endif  // TOOLTIP_H
