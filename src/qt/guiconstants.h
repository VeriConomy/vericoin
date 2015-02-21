#ifndef GUICONSTANTS_H
#define GUICONSTANTS_H

#include <QString>
#include <QTranslator>
#include <QFont>

/* Milliseconds between model updates */
static const int MODEL_UPDATE_DELAY = 500;

/* AskPassphraseDialog -- Maximum passphrase length */
static const int MAX_PASSPHRASE_SIZE = 1024;

/* Invalid field background style */
#define STYLE_INVALID "background:#FF8080"

/* Transaction list -- unconfirmed transaction */
#define COLOR_UNCONFIRMED QColor(128, 128, 128)
/* Transaction list -- negative amount */
#define COLOR_NEGATIVE QColor(165, 75, 75)
/* Transaction list -- positive amount */
#define COLOR_POSITIVE QColor(95, 140, 95)
/* Transaction list -- bare address (without label) */
#define COLOR_BAREADDRESS QColor(140, 140, 140)

/* Custom colors / fonts */
#define STRING_VERIBLUE QString("#0a3057")
#define STRING_VERIBLUE_LT QString("#418BCA")
#define STRING_VERIFONT QString("#444748")

#ifdef Q_OS_MAC
static const QFont veriFontSmaller("Lato", 11, QFont::Normal, false);
static const QFont veriFontSmall("Lato", 12, QFont::Normal, false);
static const QFont veriFontMedium("Lato", 13, QFont::Normal, false);
static const QFont veriFont("Lato", 14, QFont::Normal, false);
static const QFont veriFontLarge("Lato", 15, QFont::Normal, false);
static const QFont veriFontLarger("Lato", 16, QFont::Normal, false);
#else
static const QFont veriFontSmaller("Lato", 9, QFont::Normal, false);
static const QFont veriFontSmall("Lato", 10, QFont::Normal, false);
static const QFont veriFontMedium("Lato", 11, QFont::Normal, false);
static const QFont veriFont("Lato", 12, QFont::Normal, false);
static const QFont veriFontLarge("Lato", 13, QFont::Normal, false);
static const QFont veriFontLarger("Lato", 14, QFont::Normal, false);
#endif

/* Tooltips longer than this (in characters) are converted into rich text,
   so that they can be word-wrapped.
 */
static const int TOOLTIP_WRAP_THRESHOLD = 80;

/* Maximum allowed URI length */
static const int MAX_URI_LENGTH = 255;

/* QRCodeDialog -- size of exported QR Code image */
#define EXPORT_IMAGE_SIZE 256

#endif // GUICONSTANTS_H
