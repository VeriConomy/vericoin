#ifndef BITCOIN_BOOTSTRAP_H
#define BITCOIN_BOOTSTRAP_H

#include <string>

/** Bootstrap downloaded from vericonomy servers **/
// XXX - TODO: Permit to change the URL in verium config file
const std::string BOOTSTRAP_URL("https://cdn.vericonomy.com/016-bootstrap/bootstrap_VRM.zip");

void DownloadBootstrap();
void ApplyBootstrap();

#endif // BITCOIN_BOOTSTRAP_H
