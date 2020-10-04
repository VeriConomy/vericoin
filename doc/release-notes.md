Vericoin Vault version 1.3.2 is now available from:

  https://vericoin.info

This is a new minor version release, bringing both new features and
bug fixes.

Please report bugs using the issue tracker at github:

  https://github.com/VericoinReserve/Vericoin

Upgrading and downgrading
=========================

How to Upgrade
--------------

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), uninstall all
earlier versions of Vericoin, then run the installer.

We recommand before any upgrade that you backup your wallet.

If you are upgrading from version 1.2 or earlier, the first time you run
1.3.2 your blockchain files will be re-indexed, which will take anywhere from
5 minutes to several hours, depending on the speed of your machine.

Downgrading warnings
--------------------

The 'chainstate' for this release is not always compatible with previous
releases, so if you run 1.3.x and then decide to switch back to a
1.2.x release you might get a blockchain validation error when starting the
old release (due to 'pruned outputs' being omitted from the index of
unspent transaction outputs).

Running the old release with the -reindex option will rebuild the chainstate
data structures and correct the problem.

Also, the first time you run a 1.2.x release on a 1.3.x wallet it will rescan
the blockchain for missing spent coins, which will take a long time (tens
of minutes on a typical machine).

Notable changes
===============

Updater: Check new version
-----------------------

From this version, the GUI will automatically check for a new Vericoin Vault version at the launch of the application.
This feature will make easier the deployment of new vault version on the network.

Bootstrap
-----------------------

The synchronization modal will now show up a bootstrap button if the wallet have a chain older than a week.

1.3.2 Change log
=======================

RPC:

- Fix decoderawtransaction command

Block and transaction handling:

- Check time of transaction
- Check coinbase time

Mining:

- coinbase will have the block time at creation during mining process

Protocol and network:

- Protocol 80004

Validation:

- Validation that transaction respect minimum fee
- Check time of transaction
- Check coinbase time

GUI:

- Version check at startup
- Bootstrap button in synchronization modal

Credits
--------

Thanks to everyone who contributed to this release:
- Bitcoin Core Team
- Calvario
- Douglas Pike (effectsToCause)
- Matthieu Derasse (mderasse)

And thanks to the community to keep following us and all their work